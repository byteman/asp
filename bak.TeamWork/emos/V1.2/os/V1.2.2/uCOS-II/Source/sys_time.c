/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                             TIME MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_TIME.C
* By      : Jean J. Labrosse
* Version : V2.86
*
* LICENSING TERMS:
* ---------------
*   uC/Sys-II is provided in source form for FREE evaluation, for educational use or for peaceful research.  
* If you plan on using  uC/Sys-II  in a commercial product you need to contact Micriµm to properly license 
* its use in your product. We provide ALL the source code for your convenience and to help you experience 
* uC/Sys-II.   The fact that the  source is provided does  NOT  mean that you can use it without  paying a 
* licensing fee.
*********************************************************************************************************
*/

#ifndef  SYS_MASTER_FILE
#include <C2.h>
#endif

/*
*********************************************************************************************************
*                                DELAY TASK 'n' TICKS   (n from 0 to 65535)
*
* Description: This function is called to delay execution of the currently running task until the
*              specified number of system ticks expires.  This, of course, directly equates to delaying
*              the current task for some time to expire.  No delay will result If the specified delay is
*              0.  If the specified delay is greater than 0 then, a context switch will result.
*
* Arguments  : ticks     is the time delay that the task will be suspended in number of clock 'ticks'.
*                        Note that by specifying 0, the task will not be delayed.
*
* Returns    : none
*********************************************************************************************************
*/

void  SysTimeDly (INT16U ticks)
{
    INT8U      y;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        return;
    }
    if (ticks > 0) {                             /* 0 means no delay!                                  */
        SYS_ENTER_CRITICAL();
        y            =  SysTCBCur->SysTCBY;        /* Delay current task                                 */
        SysRdyTbl[y] &= ~SysTCBCur->SysTCBBitX;
        if (SysRdyTbl[y] == 0) {
            SysRdyGrp &= ~SysTCBCur->SysTCBBitY;
        }
        SysTCBCur->SysTCBDly = ticks;              /* Load ticks in TCB                                  */
        SYS_EXIT_CRITICAL();
        SYS_Sched();                              /* Find next task to run!                             */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                     DELAY TASK FOR SPECIFIED TIME
*
* Description: This function is called to delay execution of the currently running task until some time
*              expires.  This call allows you to specify the delay time in HOURS, MINUTES, SECONDS and
*              MILLISECONDS instead of ticks.
*
* Arguments  : hours     specifies the number of hours that the task will be delayed (max. is 255)
*              minutes   specifies the number of minutes (max. 59)
*              seconds   specifies the number of seconds (max. 59)
*              milli     specifies the number of milliseconds (max. 999)
*
* Returns    : SYS_ERR_NONE
*              SYS_ERR_TIME_INVALID_MINUTES
*              SYS_ERR_TIME_INVALID_SECONDS
*              SYS_ERR_TIME_INVALID_MS
*              SYS_ERR_TIME_ZERO_DLY
*              SYS_ERR_TIME_DLY_ISR
*
* Note(s)    : The resolution on the milliseconds depends on the tick rate.  For example, you can't do
*              a 10 mS delay if the ticker interrupts every 100 mS.  In this case, the delay would be
*              set to 0.  The actual delay is rounded to the nearest tick.
*********************************************************************************************************
*/

#if SYS_TIME_DLY_HMSM_EN > 0
INT8U  SysTimeDlyHMSM (INT8U hours, INT8U minutes, INT8U seconds, INT16U ms)
{
    INT32U ticks;
    INT16U loops;


    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        return (SYS_ERR_TIME_DLY_ISR);
    }
#if SYS_ARG_CHK_EN > 0
    if (hours == 0) {
        if (minutes == 0) {
            if (seconds == 0) {
                if (ms == 0) {
                    return (SYS_ERR_TIME_ZERO_DLY);
                }
            }
        }
    }
    if (minutes > 59) {
        return (SYS_ERR_TIME_INVALID_MINUTES);    /* Validate arguments to be within range              */
    }
    if (seconds > 59) {
        return (SYS_ERR_TIME_INVALID_SECONDS);
    }
    if (ms > 999) {
        return (SYS_ERR_TIME_INVALID_MS);
    }
#endif
                                                 /* Compute the total number of clock ticks required.. */
                                                 /* .. (rounded to the nearest tick)                   */
    ticks = ((INT32U)hours * 3600L + (INT32U)minutes * 60L + (INT32U)seconds) * SYS_TICKS_PER_SEC
          + SYS_TICKS_PER_SEC * ((INT32U)ms + 500L / SYS_TICKS_PER_SEC) / 1000L;
    loops = (INT16U)(ticks >> 16);               /* Compute the integral number of 65536 tick delays   */
    ticks = ticks & 0xFFFFL;                     /* Obtain  the fractional number of ticks             */
    SysTimeDly((INT16U)ticks);
    while (loops > 0) {
        SysTimeDly((INT16U)32768u);
        SysTimeDly((INT16U)32768u);
        loops--;
    }
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                         RESUME A DELAYED TASK
*
* Description: This function is used resume a task that has been delayed through a call to either
*              SysTimeDly() or SysTimeDlyHMSM().  Note that you can call this function to resume a
*              task that is waiting for an event with timeout.  This would make the task look
*              like a timeout occurred.
*
*              Also, you cannot resume a task that has called SysTimeDlyHMSM() with a combined time that
*              exceeds 65535 clock ticks.  In other words, if the clock tick runs at 100 Hz then, you will
*              not be able to resume a delayed task that called SysTimeDlyHMSM(0, 10, 55, 350) or higher:
*
*                  (10 Minutes * 60 + 55 Seconds + 0.35) * 100 ticks/second.
*
* Arguments  : prio                      specifies the priority of the task to resume
*
* Returns    : SYS_ERR_NONE               Task has been resumed
*              SYS_ERR_PRIO_INVALID       if the priority you specify is higher that the maximum allowed
*                                        (i.e. >= SYS_LOWEST_PRIO)
*              SYS_ERR_TIME_NOT_DLY       Task is not waiting for time to expire
*              SYS_ERR_TASK_NOT_EXIST     The desired task has not been created or has been assigned to a Mutex.
*********************************************************************************************************
*/

#if SYS_TIME_DLY_RESUME_EN > 0
INT8U  SysTimeDlyResume (INT8U prio)
{
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                                    /* Storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (prio >= SYS_LOWEST_PRIO) {
        return (SYS_ERR_PRIO_INVALID);
    }
    SYS_ENTER_CRITICAL();
    ptcb = SysTCBPrioTbl[prio];                                 /* Make sure that task exist            */
    if (ptcb == (SYS_TCB *)0) {
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);                        /* The task does not exist              */
    }
    if (ptcb == SYS_TCB_RESERVED) {
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);                        /* The task does not exist              */
    }
    if (ptcb->SysTCBDly == 0) {                                 /* See if task is delayed               */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TIME_NOT_DLY);                          /* Indicate that task was not delayed   */
    }

    ptcb->SysTCBDly = 0;                                        /* Clear the time delay                 */
    if ((ptcb->SysTCBStat & SYS_STAT_PEND_ANY) != SYS_STAT_RDY) {
        ptcb->SysTCBStat     &= ~SYS_STAT_PEND_ANY;              /* Yes, Clear status flag               */
        ptcb->SysTCBStatPend  =  SYS_STAT_PEND_TO;               /* Indicate PEND timeout                */
    } else {
        ptcb->SysTCBStatPend  =  SYS_STAT_PEND_OK;
    }
    if ((ptcb->SysTCBStat & SYS_STAT_SUSPEND) == SYS_STAT_RDY) {  /* Is task suspended?                   */
        SysRdyGrp               |= ptcb->SysTCBBitY;             /* No,  Make ready                      */
        SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
        SYS_EXIT_CRITICAL();
        SYS_Sched();                                            /* See if this is new highest priority  */
    } else {
        SYS_EXIT_CRITICAL();                                    /* Task may be suspended                */
    }
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                         GET CURRENT SYSTEM TIME
*
* Description: This function is used by your application to obtain the current value of the 32-bit
*              counter which keeps track of the number of clock ticks.
*
* Arguments  : none
*
* Returns    : The current value of SysTime
*********************************************************************************************************
*/

#if SYS_TIME_GET_SET_EN > 0
INT32U  SysTimeGet (void)
{
    INT32U     ticks;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    SYS_ENTER_CRITICAL();
    ticks = SysTime;
    SYS_EXIT_CRITICAL();
    return (ticks);
}
#endif

/*
*********************************************************************************************************
*                                            SET SYSTEM CLOCK
*
* Description: This function sets the 32-bit counter which keeps track of the number of clock ticks.
*
* Arguments  : ticks      specifies the new value that SysTime needs to take.
*
* Returns    : none
*********************************************************************************************************
*/

#if SYS_TIME_GET_SET_EN > 0
void  SysTimeSet (INT32U ticks)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    SYS_ENTER_CRITICAL();
    SysTime = ticks;
    SYS_EXIT_CRITICAL();
}
#endif
