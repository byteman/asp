/*
*********************************************************************************************************
*                                               uC/Sys-II
*                                         The Real-Time Kernel
*
*
*                                (c) Copyright 2006, Micrium, Weston, FL
*                                          All Rights Reserved
*
*                                           ARM Cortex-M3 Port
*
* File      : SYS_CPU_C.C
* Version   : V2.86
* By        : Jean J. Labrosse
*
* For       : ARMv7M Cortex-M3
* Mode      : Thumb2
* Toolchain : RealView Development Suite
*             RealView Microcontroller Development Kit (MDK)
*             ARM Developer Suite (ADS)
*             Keil uVision
*********************************************************************************************************
*/

#define  SYS_CPU_GLOBALS
#include <C2.h>

/*
*********************************************************************************************************
*                                          LOCAL VARIABLES
*********************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  INT16U  SysTmrCtr;
#endif

/*
*********************************************************************************************************
*                                          SYS TICK DEFINES
*********************************************************************************************************
*/

#define  SYS_CPU_CM3_NVIC_ST_CTRL    (*((volatile INT32U *)0xE000E010))   /* SysTick Ctrl & Status Reg. */
#define  SYS_CPU_CM3_NVIC_ST_RELOAD  (*((volatile INT32U *)0xE000E014))   /* SysTick Reload  Value Reg. */
#define  SYS_CPU_CM3_NVIC_ST_CURRENT (*((volatile INT32U *)0xE000E018))   /* SysTick Current Value Reg. */
#define  SYS_CPU_CM3_NVIC_ST_CAL     (*((volatile INT32U *)0xE000E01C))   /* SysTick Cal     Value Reg. */

#define  SYS_CPU_CM3_NVIC_ST_CTRL_COUNT                    0x00010000     /* Count flag.                */
#define  SYS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC                  0x00000004     /* Clock Source.              */
#define  SYS_CPU_CM3_NVIC_ST_CTRL_INTEN                    0x00000002     /* Interrupt enable.          */
#define  SYS_CPU_CM3_NVIC_ST_CTRL_ENABLE                   0x00000001     /* Counter mode.              */

/*
*********************************************************************************************************
*                                       Sys INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by SysInit() at the beginning of SysInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0 && SYS_VERSION > 203
void  SysInitHookBegin (void)
{
#if SYS_TMR_EN > 0
    SysTmrCtr = 0;
#endif
}
#endif

/*
*********************************************************************************************************
*                                       Sys INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by SysInit() at the end of SysInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0 && SYS_VERSION > 203
void  SysInitHookEnd (void)
{
}
#endif

/*
*********************************************************************************************************
*                                          TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0
void  SysTaskCreateHook (SYS_TCB *ptcb)
{
#if SYS_APP_HOOKS_EN > 0
    App_TaskCreateHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0
void  SysTaskDelHook (SYS_TCB *ptcb)
{
#if SYS_APP_HOOKS_EN > 0
    App_TaskDelHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif

/*
*********************************************************************************************************
*                                             IDLE TASK HOOK
*
* Description: This function is called by the idle task.  This hook has been added to allow you to do
*              such things as STOP the CPU to conserve power.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are enabled during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0 && SYS_VERSION >= 251
void  SysTaskIdleHook (void)
{
#if SYS_APP_HOOKS_EN > 0
    App_TaskIdleHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                           STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/Sys-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/

#if SYS_CPU_HOOKS_EN > 0
void  SysTaskStatHook (void)
{
#if SYS_APP_HOOKS_EN > 0
    App_TaskStatHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                        INITIALIZE A TASK'S STACK
*
* Description: This function is called by either SysTaskCreate() or SysTaskCreateExt() to initialize the
*              stack frame of the task being created.  This function is highly processor specific.
*
* Arguments  : task          is a pointer to the task code
*
*              p_arg         is a pointer to a user supplied data area that will be passed to the task
*                            when the task first executes.
*
*              ptos          is a pointer to the top of stack.  It is assumed that 'ptos' points to
*                            a 'free' entry on the task stack.  If SYS_STK_GROWTH is set to 1 then
*                            'ptos' will contain the HIGHEST valid address of the stack.  Similarly, if
*                            SYS_STK_GROWTH is set to 0, the 'ptos' will contains the LOWEST valid address
*                            of the stack.
*
*              opt           specifies options that can be used to alter the behavior of SysTaskStkInit().
*                            (see uCSYS_II.H for SYS_TASK_OPT_xxx).
*
* Returns    : Always returns the location of the new top-of-stack once the processor registers have
*              been placed on the stack in the proper order.
*
* Note(s)    : 1) Interrupts are enabled when your task starts executing.
*              2) All tasks run in Thread mode, using process stack.
*********************************************************************************************************
*/

SYS_STK *SysTaskStkInit (void (*task)(void *p_arg), void *p_arg, SYS_STK *ptos, INT16U opt)
{
    SYS_STK *stk;


    (void)opt;                                   /* 'opt' is not used, prevent warning                 */
    stk       = ptos;                            /* Load stack pointer                                 */


		//add by kady
    *(stk)  = (INT32U)0x00000000L;             	 /* Dumy                                              */
    *(--stk)  = (INT32U)0x00000000L;             /* FPSR                                              */
    *(--stk)  = (INT32U)0x41700000L;             /* S15                                               */
    *(--stk)  = (INT32U)0x41600000L;             /* S14                                               */
    *(--stk)  = (INT32U)0x41500000L;             /* S13                                               */
    *(--stk)  = (INT32U)0x41400000L;             /* S12                                               */
    *(--stk)  = (INT32U)0x41300000L;             /* S11                                               */
    *(--stk)  = (INT32U)0x41200000L;             /* S10                                               */
    *(--stk)  = (INT32U)0x41100000L;             /* S9                                                */
    *(--stk)  = (INT32U)0x41000000L;             /* S8                                                */
    *(--stk)  = (INT32U)0x40E00000L;             /* S7                                                */
    *(--stk)  = (INT32U)0x40C00000L;             /* S6                                                */
    *(--stk)  = (INT32U)0x40A00000L;             /* S5                                                */
    *(--stk)  = (INT32U)0x40800000L;             /* S4                                                */
    *(--stk)  = (INT32U)0x40400000L;             /* S3                                                */
    *(--stk)  = (INT32U)0x40000000L;             /* S2                                                */
    *(--stk)  = (INT32U)0x3F800000L;             /* S1                                                */
    *(--stk)  = (INT32U)0x00000000L;             /* S0                                                */
		//add by kady end

                                                 /* Registers stacked as if auto-saved on exception    */
    *(--stk)    = (INT32U)0x01000000L;             /* xPSR                                               */
    *(--stk)  = (INT32U)task;                    /* Entry Point                                        */
    *(--stk)  = (INT32U)0xFFFFFFFEL;             /* R14 (LR) (init value will cause fault if ever used)*/
    *(--stk)  = (INT32U)0x12121212L;             /* R12                                                */
    *(--stk)  = (INT32U)0x03030303L;             /* R3                                                 */
    *(--stk)  = (INT32U)0x02020202L;             /* R2                                                 */
    *(--stk)  = (INT32U)0x01010101L;             /* R1                                                 */
    *(--stk)  = (INT32U)p_arg;                   /* R0 : argument                                      */

                                                 /* Remaining registers saved on process stack         */
    *(--stk)  = (INT32U)0x11111111L;             /* R11                                                */
    *(--stk)  = (INT32U)0x10101010L;             /* R10                                                */
    *(--stk)  = (INT32U)0x09090909L;             /* R9                                                 */
    *(--stk)  = (INT32U)0x08080808L;             /* R8                                                 */
    *(--stk)  = (INT32U)0x07070707L;             /* R7                                                 */
    *(--stk)  = (INT32U)0x06060606L;             /* R6                                                 */
    *(--stk)  = (INT32U)0x05050505L;             /* R5                                                 */
    *(--stk)  = (INT32U)0x04040404L;             /* R4                                                 */

	  //add by kady
    *(--stk)  = (INT32U)0x41F80000L;             /* S31                                                */
    *(--stk)  = (INT32U)0x41F00000L;             /* S30                                                */
    *(--stk)  = (INT32U)0x41E80000L;             /* S29                                                */
    *(--stk)  = (INT32U)0x41E00000L;             /* S28                                                */
    *(--stk)  = (INT32U)0x41D80000L;             /* S27                                                */
    *(--stk)  = (INT32U)0x41D00000L;             /* S26                                                */
    *(--stk)  = (INT32U)0x41C80000L;             /* S25                                                */
    *(--stk)  = (INT32U)0x41C00000L;             /* S24                                                */
    *(--stk)  = (INT32U)0x41B80000L;             /* S23                                                */
    *(--stk)  = (INT32U)0x41B00000L;             /* S22                                                */
    *(--stk)  = (INT32U)0x41A80000L;             /* S21                                                */
    *(--stk)  = (INT32U)0x41A00000L;             /* S20                                                */
    *(--stk)  = (INT32U)0x41980000L;             /* S19                                                */
    *(--stk)  = (INT32U)0x41900000L;             /* S18                                                */
    *(--stk)  = (INT32U)0x41880000L;             /* S17   											   */
    *(--stk)  = (INT32U)0x41800000L;             /* S16   											   */
	  //add by kady end

    return (stk);
}

/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'SysTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'SysTCBCur' points to the
*                 task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
#if (SYS_CPU_HOOKS_EN > 0) && (SYS_TASK_SW_HOOK_EN > 0)
void  SysTaskSwHook (void)
{
#if SYS_APP_HOOKS_EN > 0
    App_TaskSwHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                           SYS_TCBInit() HOOK
*
* Description: This function is called by SYS_TCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if SYS_CPU_HOOKS_EN > 0 && SYS_VERSION > 203
void  SysTCBInitHook (SYS_TCB *ptcb)
{
#if SYS_APP_HOOKS_EN > 0
    App_TCBInitHook(ptcb);
#else
    (void)ptcb;                                  /* Prevent compiler warning                           */
#endif
}
#endif


/*
*********************************************************************************************************
*                                               TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if (SYS_CPU_HOOKS_EN > 0) && (SYS_TIME_TICK_HOOK_EN > 0)
void  SysTimeTickHook (void)
{
#if SYS_APP_HOOKS_EN > 0
    App_TimeTickHook();
#endif

#if SYS_TMR_EN > 0
    SysTmrCtr++;
    if (SysTmrCtr >= (SYS_TICKS_PER_SEC / SYS_TMR_CFG_TICKS_PER_SEC)) {
        SysTmrCtr = 0;
        SysTmrSignal();
    }
#endif
}
#endif



/*
*********************************************************************************************************
*                                         SYS_CPU_SysTickHandler()
*
* Description: Handle the system tick (SysTick) interrupt, which is used to generate the uC/Sys-II tick
*              interrupt.
*
* Arguments  : none.
*
* Note(s)    : 1) This function MUST be placed on entry 15 of the Cortex-M3 vector table.
*********************************************************************************************************
*/

void  SYS_CPU_SysTickHandler (void)
{
    SYS_CPU_SR  cpu_sr;


    SYS_ENTER_CRITICAL();                         /* Tell uC/Sys-II that we are starting an ISR          */
    SysIntNesting++;
    SYS_EXIT_CRITICAL();

    SysTimeTick();                                /* Call uC/Sys-II's SysTimeTick()                       */

    SysIntExit();                                 /* Tell uC/Sys-II that we are leaving the ISR          */
}


/*
*********************************************************************************************************
*                                          SYS_CPU_SysTickInit()
*
* Description: Initialize the SysTick.
*
* Arguments  : none.
*
* Note(s)    : 1) This function MUST be called after SysStart() & after processor initialization.
*********************************************************************************************************
*/

void  SYS_CPU_SysTickInit (void)
{
    INT32U  cnts;


    cnts = SYS_CPU_SysTickClkFreq() / SYS_TICKS_PER_SEC;

    SYS_CPU_CM3_NVIC_ST_RELOAD = (cnts - 1);
                                                 /* Enable timer.                                      */
    SYS_CPU_CM3_NVIC_ST_CTRL  |= SYS_CPU_CM3_NVIC_ST_CTRL_CLK_SRC | SYS_CPU_CM3_NVIC_ST_CTRL_ENABLE;
                                                 /* Enable timer interrupt.                            */
    SYS_CPU_CM3_NVIC_ST_CTRL  |= SYS_CPU_CM3_NVIC_ST_CTRL_INTEN;
}
