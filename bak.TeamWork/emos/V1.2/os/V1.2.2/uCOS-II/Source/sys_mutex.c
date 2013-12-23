/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                  MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_MUTEX.C
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


#if SYS_MUTEX_EN > 0
/*
*********************************************************************************************************
*                                            LOCAL CONSTANTS
*********************************************************************************************************
*/

#define  SYS_MUTEX_KEEP_LOWER_8   ((INT16U)0x00FFu)
#define  SYS_MUTEX_KEEP_UPPER_8   ((INT16U)0xFF00u)

#define  SYS_MUTEX_AVAILABLE      ((INT16U)0x00FFu)

/*
*********************************************************************************************************
*                                            LOCAL CONSTANTS
*********************************************************************************************************
*/

static  void  SysMutex_RdyAtPrio(SYS_TCB *ptcb, INT8U prio);

/*$PAGE*/
/*
*********************************************************************************************************
*                                   ACCEPT MUTUAL EXCLUSION SEMAPHORE
*
* Description: This  function checks the mutual exclusion semaphore to see if a resource is available.
*              Unlike SysMutexPend(), SysMutexAccept() does not suspend the calling task if the resource is
*              not available or the event did not occur.
*
* Arguments  : pevent     is a pointer to the event control block
*
*              perr       is a pointer to an error code which will be returned to your application:
*                            SYS_ERR_NONE         if the call was successful.
*                            SYS_ERR_EVENT_TYPE   if 'pevent' is not a pointer to a mutex
*                            SYS_ERR_PEVENT_NULL  'pevent' is a NULL pointer
*                            SYS_ERR_PEND_ISR     if you called this function from an ISR
*                            SYS_ERR_PIP_LOWER    If the priority of the task that owns the Mutex is
*                                                HIGHER (i.e. a lower number) than the PIP.  This error
*                                                indicates that you did not set the PIP higher (lower
*                                                number) than ALL the tasks that compete for the Mutex.
*                                                Unfortunately, this is something that could not be
*                                                detected when the Mutex is created because we don't know
*                                                what tasks will be using the Mutex.
*
* Returns    : == SYS_TRUE    if the resource is available, the mutual exclusion semaphore is acquired
*              == SYS_FALSE   a) if the resource is not available
*                            b) you didn't pass a pointer to a mutual exclusion semaphore
*                            c) you called this function from an ISR
*
* Warning(s) : This function CANNOT be called from an ISR because mutual exclusion semaphores are
*              intended to be used by tasks only.
*********************************************************************************************************
*/

#if SYS_MUTEX_ACCEPT_EN > 0
BOOLEAN  SysMutexAccept (SYS_EVENT *pevent, INT8U *perr)
{
    INT8U      pip;                                    /* Priority Inheritance Priority (PIP)          */
#if SYS_CRITICAL_METHOD == 3                            /* Allocate storage for CPU status register     */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                          /* Validate 'perr'                              */
        return (SYS_FALSE);
    }
    if (pevent == (SYS_EVENT *)0) {                     /* Validate 'pevent'                            */
        *perr = SYS_ERR_PEVENT_NULL;
        return (SYS_FALSE);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MUTEX) {  /* Validate event block type                    */
        *perr = SYS_ERR_EVENT_TYPE;
        return (SYS_FALSE);
    }
    if (SysIntNesting > 0) {                            /* Make sure it's not called from an ISR        */
        *perr = SYS_ERR_PEND_ISR;
        return (SYS_FALSE);
    }
    SYS_ENTER_CRITICAL();                               /* Get value (0 or 1) of Mutex                  */
    pip = (INT8U)(pevent->SysEventCnt >> 8);            /* Get PIP from mutex                           */
    if ((pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8) == SYS_MUTEX_AVAILABLE) {
        pevent->SysEventCnt &= SYS_MUTEX_KEEP_UPPER_8;   /*      Mask off LSByte (Acquire Mutex)         */
        pevent->SysEventCnt |= SysTCBCur->SysTCBPrio;     /*      Save current task priority in LSByte    */
        pevent->SysEventPtr  = (void *)SysTCBCur;        /*      Link TCB of task owning Mutex           */
        if (SysTCBCur->SysTCBPrio <= pip) {              /*      PIP 'must' have a SMALLER prio ...      */
            SYS_EXIT_CRITICAL();                        /*      ... than current task!                  */
            *perr = SYS_ERR_PIP_LOWER;
        } else {
            SYS_EXIT_CRITICAL();
            *perr = SYS_ERR_NONE;
        }
        return (SYS_TRUE);
    }
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (SYS_FALSE);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  CREATE A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function creates a mutual exclusion semaphore.
*
* Arguments  : prio          is the priority to use when accessing the mutual exclusion semaphore.  In
*                            other words, when the semaphore is acquired and a higher priority task
*                            attempts to obtain the semaphore then the priority of the task owning the
*                            semaphore is raised to this priority.  It is assumed that you will specify
*                            a priority that is LOWER in value than ANY of the tasks competing for the
*                            mutex.
*
*              perr          is a pointer to an error code which will be returned to your application:
*                               SYS_ERR_NONE         if the call was successful.
*                               SYS_ERR_CREATE_ISR   if you attempted to create a MUTEX from an ISR
*                               SYS_ERR_PRIO_EXIST   if a task at the priority inheritance priority
*                                                   already exist.
*                               SYS_ERR_PEVENT_NULL  No more event control blocks available.
*                               SYS_ERR_PRIO_INVALID if the priority you specify is higher that the
*                                                   maximum allowed (i.e. > SYS_LOWEST_PRIO)
*
* Returns    : != (void *)0  is a pointer to the event control clock (SYS_EVENT) associated with the
*                            created mutex.
*              == (void *)0  if an error is detected.
*
* Note(s)    : 1) The LEAST significant 8 bits of '.SysEventCnt' are used to hold the priority number
*                 of the task owning the mutex or 0xFF if no task owns the mutex.
*
*              2) The MSysT  significant 8 bits of '.SysEventCnt' are used to hold the priority number
*                 to use to reduce priority inversion.
*********************************************************************************************************
*/

SYS_EVENT  *SysMutexCreate (INT8U prio, INT8U *perr)
{
    SYS_EVENT  *pevent;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return ((SYS_EVENT *)0);
    }
    if (prio >= SYS_LOWEST_PRIO) {                          /* Validate PIP                             */
        *perr = SYS_ERR_PRIO_INVALID;
        return ((SYS_EVENT *)0);
    }
#endif
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_CREATE_ISR;                         /* ... can't CREATE mutex from an ISR       */
        return ((SYS_EVENT *)0);
    }
    SYS_ENTER_CRITICAL();
    if (SysTCBPrioTbl[prio] != (SYS_TCB *)0) {               /* Mutex priority must not already exist    */
        SYS_EXIT_CRITICAL();                                /* Task already exist at priority ...       */
        *perr = SYS_ERR_PRIO_EXIST;                         /* ... inheritance priority                 */
        return ((SYS_EVENT *)0);
    }
    SysTCBPrioTbl[prio] = SYS_TCB_RESERVED;                  /* Reserve the table entry                  */
    pevent             = SysEventFreeList;                  /* Get next free event control block        */
    if (pevent == (SYS_EVENT *)0) {                         /* See if an ECB was available              */
        SysTCBPrioTbl[prio] = (SYS_TCB *)0;                  /* No, Release the table entry              */
        SYS_EXIT_CRITICAL();
        *perr              = SYS_ERR_PEVENT_NULL;           /* No more event control blocks             */
        return (pevent);
    }
    SysEventFreeList        = (SYS_EVENT *)SysEventFreeList->SysEventPtr;   /* Adjust the free list        */
    SYS_EXIT_CRITICAL();
    pevent->SysEventType    = SYS_EVENT_TYPE_MUTEX;
    pevent->SysEventCnt     = (INT16U)((INT16U)prio << 8) | SYS_MUTEX_AVAILABLE; /* Resource is avail.   */
    pevent->SysEventPtr     = (void *)0;                                 /* No task owning the mutex    */
#if SYS_EVENT_NAME_SIZE > 1
    pevent->SysEventName[0] = '?';
    pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
    SYS_EventWaitListInit(pevent);
    *perr                  = SYS_ERR_NONE;
    return (pevent);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                          DELETE A MUTEX
*
* Description: This function deletes a mutual exclusion semaphore and readies all tasks pending on the it.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mutex.
*
*              opt           determines delete options as follows:
*                            opt == SYS_DEL_NO_PEND   Delete mutex ONLY if no task pending
*                            opt == SYS_DEL_ALWAYS    Deletes the mutex even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            SYS_ERR_NONE             The call was successful and the mutex was deleted
*                            SYS_ERR_DEL_ISR          If you attempted to delete the MUTEX from an ISR
*                            SYS_ERR_INVALID_OPT      An invalid option was specified
*                            SYS_ERR_TASK_WAITING     One or more tasks were waiting on the mutex
*                            SYS_ERR_EVENT_TYPE       If you didn't pass a pointer to a mutex
*                            SYS_ERR_PEVENT_NULL      If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (SYS_EVENT *)0 if the mutex was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the mutex MUST check the return code of SysMutexPend().
*
*              2) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the mutex.
*
*              3) Because ALL tasks pending on the mutex will be readied, you MUST be careful because the
*                 resource(s) will no longer be guarded by the mutex.
*
*              4) IMPORTANT: In the 'SYS_DEL_ALWAYS' case, we assume that the owner of the Mutex (if there
*                            is one) is ready-to-run and is thus NOT pending on another kernel object or
*                            has delayed itself.  In other words, if a task owns the mutex being deleted,
*                            that task will be made ready-to-run at its original priority.
*********************************************************************************************************
*/

#if SYS_MUTEX_DEL_EN
SYS_EVENT  *SysMutexDel (SYS_EVENT *pevent, INT8U opt, INT8U *perr)
{
    BOOLEAN    tasks_waiting;
    SYS_EVENT  *pevent_return;
    INT8U      pip;                                        /* Priority inheritance priority            */
    INT8U      prio;
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return (pevent);
    }
    if (pevent == (SYS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = SYS_ERR_PEVENT_NULL;
        return (pevent);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MUTEX) {      /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return (pevent);
    }
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_DEL_ISR;                             /* ... can't DELETE from an ISR             */
        return (pevent);
    }
    SYS_ENTER_CRITICAL();
    if (pevent->SysEventGrp != 0) {                         /* See if any tasks waiting on mutex        */
        tasks_waiting = SYS_TRUE;                           /* Yes                                      */
    } else {
        tasks_waiting = SYS_FALSE;                          /* No                                       */
    }
    switch (opt) {
        case SYS_DEL_NO_PEND:                               /* DELETE MUTEX ONLY IF NO TASK WAITING --- */
             if (tasks_waiting == SYS_FALSE) {
#if SYS_EVENT_NAME_SIZE > 1
                 pevent->SysEventName[0] = '?';             /* Unknown name                             */
                 pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
                 pip                 = (INT8U)(pevent->SysEventCnt >> 8);
                 SysTCBPrioTbl[pip]   = (SYS_TCB *)0;        /* Free up the PIP                          */
                 pevent->SysEventType = SYS_EVENT_TYPE_UNUSED;
                 pevent->SysEventPtr  = SysEventFreeList;    /* Return Event Control Block to free list  */
                 pevent->SysEventCnt  = 0;
                 SysEventFreeList     = pevent;
                 SYS_EXIT_CRITICAL();
                 *perr               = SYS_ERR_NONE;
                 pevent_return       = (SYS_EVENT *)0;      /* Mutex has been deleted                   */
             } else {
                 SYS_EXIT_CRITICAL();
                 *perr               = SYS_ERR_TASK_WAITING;
                 pevent_return       = pevent;
             }
             break;

        case SYS_DEL_ALWAYS:                                /* ALWAYS DELETE THE MUTEX ---------------- */
             pip  = (INT8U)(pevent->SysEventCnt >> 8);                     /* Get PIP of mutex          */
             prio = (INT8U)(pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8);  /* Get owner's original prio */
             ptcb = (SYS_TCB *)pevent->SysEventPtr;
             if (ptcb != (SYS_TCB *)0) {                    /* See if any task owns the mutex           */
                 if (ptcb->SysTCBPrio == pip) {             /* See if original prio was changed         */
                     SysMutex_RdyAtPrio(ptcb, prio);        /* Yes, Restore the task's original prio    */
                 }
             }
             while (pevent->SysEventGrp != 0) {             /* Ready ALL tasks waiting for mutex        */
                 (void)SYS_EventTaskRdy(pevent, (void *)0, SYS_STAT_MUTEX, SYS_STAT_PEND_OK);
             }
#if SYS_EVENT_NAME_SIZE > 1
             pevent->SysEventName[0] = '?';                 /* Unknown name                             */
             pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
             pip                 = (INT8U)(pevent->SysEventCnt >> 8);
             SysTCBPrioTbl[pip]   = (SYS_TCB *)0;            /* Free up the PIP                          */
             pevent->SysEventType = SYS_EVENT_TYPE_UNUSED;
             pevent->SysEventPtr  = SysEventFreeList;        /* Return Event Control Block to free list  */
             pevent->SysEventCnt  = 0;
             SysEventFreeList     = pevent;                 /* Get next free event control block        */
             SYS_EXIT_CRITICAL();
             if (tasks_waiting == SYS_TRUE) {               /* Reschedule only if task(s) were waiting  */
                 SYS_Sched();                               /* Find highest priority task ready to run  */
             }
             *perr         = SYS_ERR_NONE;
             pevent_return = (SYS_EVENT *)0;                /* Mutex has been deleted                   */
             break;

        default:
             SYS_EXIT_CRITICAL();
             *perr         = SYS_ERR_INVALID_OPT;
             pevent_return = pevent;
             break;
    }
    return (pevent_return);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                  PEND ON MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function waits for a mutual exclusion semaphore.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            mutex.
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for the resource up to the amount of time specified by this argument.
*                            If you specify 0, however, your task will wait forever at the specified
*                            mutex or, until the resource becomes available.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*                               SYS_ERR_NONE        The call was successful and your task owns the mutex
*                               SYS_ERR_TIMEOUT     The mutex was not available within the specified 'timeout'.
*                               SYS_ERR_PEND_ABORT  The wait on the mutex was aborted.
*                               SYS_ERR_EVENT_TYPE  If you didn't pass a pointer to a mutex
*                               SYS_ERR_PEVENT_NULL 'pevent' is a NULL pointer
*                               SYS_ERR_PEND_ISR    If you called this function from an ISR and the result
*                                                  would lead to a suspension.
*                               SYS_ERR_PIP_LOWER   If the priority of the task that owns the Mutex is
*                                                  HIGHER (i.e. a lower number) than the PIP.  This error
*                                                  indicates that you did not set the PIP higher (lower
*                                                  number) than ALL the tasks that compete for the Mutex.
*                                                  Unfortunately, this is something that could not be
*                                                  detected when the Mutex is created because we don't know
*                                                  what tasks will be using the Mutex.
*                               SYS_ERR_PEND_LOCKED If you called this function when the scheduler is locked
*
* Returns    : none
*
* Note(s)    : 1) The task that owns the Mutex MUST NOT pend on any other event while it owns the mutex.
*
*              2) You MUST NOT change the priority of the task that owns the mutex
*********************************************************************************************************
*/

void  SysMutexPend (SYS_EVENT *pevent, INT16U timeout, INT8U *perr)
{
    INT8U      pip;                                        /* Priority Inheritance Priority (PIP)      */
    INT8U      mprio;                                      /* Mutex owner priority                     */
    BOOLEAN    rdy;                                        /* Flag indicating task was ready           */
    SYS_TCB    *ptcb;
    SYS_EVENT  *pevent2;
    INT8U      y;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return;
    }
    if (pevent == (SYS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = SYS_ERR_PEVENT_NULL;
        return;
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MUTEX) {      /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return;
    }
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_PEND_ISR;                           /* ... can't PEND from an ISR               */
        return;
    }
    if (SysLockNesting > 0) {                               /* See if called with scheduler locked ...  */
        *perr = SYS_ERR_PEND_LOCKED;                        /* ... can't PEND when locked               */
        return;
    }
/*$PAGE*/
    SYS_ENTER_CRITICAL();
    pip = (INT8U)(pevent->SysEventCnt >> 8);                /* Get PIP from mutex                       */
                                                           /* Is Mutex available?                      */
    if ((INT8U)(pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8) == SYS_MUTEX_AVAILABLE) {
        pevent->SysEventCnt &= SYS_MUTEX_KEEP_UPPER_8;       /* Yes, Acquire the resource                */
        pevent->SysEventCnt |= SysTCBCur->SysTCBPrio;         /*      Save priority of owning task        */
        pevent->SysEventPtr  = (void *)SysTCBCur;            /*      Point to owning task's SYS_TCB       */
        if (SysTCBCur->SysTCBPrio <= pip) {                  /*      PIP 'must' have a SMALLER prio ...  */
            SYS_EXIT_CRITICAL();                            /*      ... than current task!              */
            *perr = SYS_ERR_PIP_LOWER;
        } else {
            SYS_EXIT_CRITICAL();
            *perr = SYS_ERR_NONE;
        }
        return;
    }
    mprio = (INT8U)(pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8);  /* No, Get priority of mutex owner   */
    ptcb  = (SYS_TCB *)(pevent->SysEventPtr);                       /*     Point to TCB of mutex owner   */
    if (ptcb->SysTCBPrio > pip) {                                  /*     Need to promote prio of owner?*/
        if (mprio > SysTCBCur->SysTCBPrio) {
            y = ptcb->SysTCBY;
            if ((SysRdyTbl[y] & ptcb->SysTCBBitX) != 0) {           /*     See if mutex owner is ready   */
                SysRdyTbl[y] &= ~ptcb->SysTCBBitX;                  /*     Yes, Remove owner from Rdy ...*/
                if (SysRdyTbl[y] == 0) {                           /*          ... list at current prio */
                    SysRdyGrp &= ~ptcb->SysTCBBitY;
                }
                rdy = SYS_TRUE;
            } else {
                pevent2 = ptcb->SysTCBEventPtr;
                if (pevent2 != (SYS_EVENT *)0) {                   /* Remove from event wait list       */
                    if ((pevent2->SysEventTbl[ptcb->SysTCBY] &= ~ptcb->SysTCBBitX) == 0) {
                        pevent2->SysEventGrp &= ~ptcb->SysTCBBitY;
                    }
                }
                rdy = SYS_FALSE;                            /* No                                       */
            }
            ptcb->SysTCBPrio = pip;                         /* Change owner task prio to PIP            */
#if SYS_LOWEST_PRIO <= 63
            ptcb->SysTCBY    = (INT8U)( ptcb->SysTCBPrio >> 3);
            ptcb->SysTCBX    = (INT8U)( ptcb->SysTCBPrio & 0x07);
            ptcb->SysTCBBitY = (INT8U)(1 << ptcb->SysTCBY);
            ptcb->SysTCBBitX = (INT8U)(1 << ptcb->SysTCBX);
#else
            ptcb->SysTCBY    = (INT8U)((ptcb->SysTCBPrio >> 4) & 0xFF);
            ptcb->SysTCBX    = (INT8U)( ptcb->SysTCBPrio & 0x0F);
            ptcb->SysTCBBitY = (INT16U)(1 << ptcb->SysTCBY);
            ptcb->SysTCBBitX = (INT16U)(1 << ptcb->SysTCBX);
#endif
            if (rdy == SYS_TRUE) {                          /* If task was ready at owner's priority ...*/
                SysRdyGrp               |= ptcb->SysTCBBitY; /* ... make it ready at new priority.       */
                SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
            } else {
                pevent2 = ptcb->SysTCBEventPtr;
                if (pevent2 != (SYS_EVENT *)0) {            /* Add to event wait list                   */
                    pevent2->SysEventGrp               |= ptcb->SysTCBBitY;
                    pevent2->SysEventTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
                }
            }
            SysTCBPrioTbl[pip] = ptcb;
        }
    }
    SysTCBCur->SysTCBStat     |= SYS_STAT_MUTEX;         /* Mutex not available, pend current task        */
    SysTCBCur->SysTCBStatPend  = SYS_STAT_PEND_OK;
    SysTCBCur->SysTCBDly       = timeout;               /* Store timeout in current task's TCB           */
    SYS_EventTaskWait(pevent);                         /* Suspend task until event or timeout occurs    */
    SYS_EXIT_CRITICAL();
    SYS_Sched();                                       /* Find next highest priority task ready         */
    SYS_ENTER_CRITICAL();
    switch (SysTCBCur->SysTCBStatPend) {                /* See if we timed-out or aborted                */
        case SYS_STAT_PEND_OK:
             *perr = SYS_ERR_NONE;
             break;

        case SYS_STAT_PEND_ABORT:
             *perr = SYS_ERR_PEND_ABORT;               /* Indicate that we aborted getting mutex        */
             break;
             
        case SYS_STAT_PEND_TO:
        default:
             SYS_EventTaskRemove(SysTCBCur, pevent);
             *perr = SYS_ERR_TIMEOUT;                  /* Indicate that we didn't get mutex within TO   */
             break;
    }
    SysTCBCur->SysTCBStat          =  SYS_STAT_RDY;      /* Set   task  status to ready                   */
    SysTCBCur->SysTCBStatPend      =  SYS_STAT_PEND_OK;  /* Clear pend  status                            */
    SysTCBCur->SysTCBEventPtr      = (SYS_EVENT  *)0;    /* Clear event pointers                          */
#if (SYS_EVENT_MULTI_EN > 0)
    SysTCBCur->SysTCBEventMultiPtr = (SYS_EVENT **)0;
#endif
    SYS_EXIT_CRITICAL();
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                  PSysT TO A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function signals a mutual exclusion semaphore
*
* Arguments  : pevent              is a pointer to the event control block associated with the desired
*                                  mutex.
*
* Returns    : SYS_ERR_NONE             The call was successful and the mutex was signaled.
*              SYS_ERR_EVENT_TYPE       If you didn't pass a pointer to a mutex
*              SYS_ERR_PEVENT_NULL      'pevent' is a NULL pointer
*              SYS_ERR_PSysT_ISR         Attempted to post from an ISR (not valid for MUTEXes)
*              SYS_ERR_NOT_MUTEX_OWNER  The task that did the post is NOT the owner of the MUTEX.
*              SYS_ERR_PIP_LOWER        If the priority of the new task that owns the Mutex is
*                                      HIGHER (i.e. a lower number) than the PIP.  This error
*                                      indicates that you did not set the PIP higher (lower
*                                      number) than ALL the tasks that compete for the Mutex.
*                                      Unfortunately, this is something that could not be
*                                      detected when the Mutex is created because we don't know
*                                      what tasks will be using the Mutex.
*********************************************************************************************************
*/

INT8U  SysMutexPost (SYS_EVENT *pevent)
{
    INT8U      pip;                                   /* Priority inheritance priority                 */
    INT8U      prio;
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysIntNesting > 0) {                           /* See if called from ISR ...                    */
        return (SYS_ERR_PSysT_ISR);                     /* ... can't PSysT mutex from an ISR              */
    }
#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (SYS_ERR_PEVENT_NULL);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MUTEX) { /* Validate event block type                     */
        return (SYS_ERR_EVENT_TYPE);
    }
    SYS_ENTER_CRITICAL();
    pip  = (INT8U)(pevent->SysEventCnt >> 8);          /* Get priority inheritance priority of mutex    */
    prio = (INT8U)(pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8);  /* Get owner's original priority      */
    if (SysTCBCur != (SYS_TCB *)pevent->SysEventPtr) {   /* See if posting task owns the MUTEX            */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_NOT_MUTEX_OWNER);
    }
    if (SysTCBCur->SysTCBPrio == pip) {                 /* Did we have to raise current task's priority? */
        SysMutex_RdyAtPrio(SysTCBCur, prio);            /* Restore the task's original priority          */
    }
    SysTCBPrioTbl[pip] = SYS_TCB_RESERVED;              /* Reserve table entry                           */
    if (pevent->SysEventGrp != 0) {                    /* Any task waiting for the mutex?               */
                                                      /* Yes, Make HPT waiting for mutex ready         */
        prio                = SYS_EventTaskRdy(pevent, (void *)0, SYS_STAT_MUTEX, SYS_STAT_PEND_OK);
        pevent->SysEventCnt &= SYS_MUTEX_KEEP_UPPER_8;  /*      Save priority of mutex's new owner       */
        pevent->SysEventCnt |= prio;
        pevent->SysEventPtr  = SysTCBPrioTbl[prio];     /*      Link to new mutex owner's SYS_TCB         */
        if (prio <= pip) {                            /*      PIP 'must' have a SMALLER prio ...       */
            SYS_EXIT_CRITICAL();                       /*      ... than current task!                   */
            SYS_Sched();                               /*      Find highest priority task ready to run  */
            return (SYS_ERR_PIP_LOWER);
        } else {
            SYS_EXIT_CRITICAL();
            SYS_Sched();                               /*      Find highest priority task ready to run  */
            return (SYS_ERR_NONE);
        }
    }
    pevent->SysEventCnt |= SYS_MUTEX_AVAILABLE;         /* No,  Mutex is now available                   */
    pevent->SysEventPtr  = (void *)0;
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                     QUERY A MUTUAL EXCLUSION SEMAPHORE
*
* Description: This function obtains information about a mutex
*
* Arguments  : pevent          is a pointer to the event control block associated with the desired mutex
*
*              p_mutex_data    is a pointer to a structure that will contain information about the mutex
*
* Returns    : SYS_ERR_NONE          The call was successful and the message was sent
*              SYS_ERR_QUERY_ISR     If you called this function from an ISR
*              SYS_ERR_PEVENT_NULL   If 'pevent'       is a NULL pointer
*              SYS_ERR_PDATA_NULL    If 'p_mutex_data' is a NULL pointer
*              SYS_ERR_EVENT_TYPE    If you are attempting to obtain data from a non mutex.
*********************************************************************************************************
*/

#if SYS_MUTEX_QUERY_EN > 0
INT8U  SysMutexQuery (SYS_EVENT *pevent, SYS_MUTEX_DATA *p_mutex_data)
{
    INT8U      i;
#if SYS_LOWEST_PRIO <= 63
    INT8U     *psrc;
    INT8U     *pdest;
#else
    INT16U    *psrc;
    INT16U    *pdest;
#endif
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        return (SYS_ERR_QUERY_ISR);                         /* ... can't QUERY mutex from an ISR        */
    }
#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (SYS_ERR_PEVENT_NULL);
    }
    if (p_mutex_data == (SYS_MUTEX_DATA *)0) {              /* Validate 'p_mutex_data'                  */
        return (SYS_ERR_PDATA_NULL);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MUTEX) {      /* Validate event block type                */
        return (SYS_ERR_EVENT_TYPE);
    }
    SYS_ENTER_CRITICAL();
    p_mutex_data->SysMutexPIP  = (INT8U)(pevent->SysEventCnt >> 8);
    p_mutex_data->SysOwnerPrio = (INT8U)(pevent->SysEventCnt & SYS_MUTEX_KEEP_LOWER_8);
    if (p_mutex_data->SysOwnerPrio == 0xFF) {
        p_mutex_data->SysValue = SYS_TRUE;
    } else {
        p_mutex_data->SysValue = SYS_FALSE;
    }
    p_mutex_data->SysEventGrp  = pevent->SysEventGrp;        /* Copy wait list                           */
    psrc                      = &pevent->SysEventTbl[0];
    pdest                     = &p_mutex_data->SysEventTbl[0];
    for (i = 0; i < SYS_EVENT_TBL_SIZE; i++) {
        *pdest++ = *psrc++;
    }
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif                                                     /* SYS_MUTEX_QUERY_EN                        */

/*$PAGE*/
/*
*********************************************************************************************************
*                                RESTORE A TASK BACK TO ITS ORIGINAL PRIORITY
*
* Description: This function makes a task ready at the specified priority
*
* Arguments  : ptcb            is a pointer to SYS_TCB of the task to make ready
*
*              prio            is the desired priority
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SysMutex_RdyAtPrio (SYS_TCB *ptcb, INT8U prio)
{
    INT8U   y;


    y            =  ptcb->SysTCBY;                          /* Remove owner from ready list at 'pip'    */
    SysRdyTbl[y] &= ~ptcb->SysTCBBitX;
    if (SysRdyTbl[y] == 0) {
        SysRdyGrp &= ~ptcb->SysTCBBitY;
    }
    ptcb->SysTCBPrio         = prio;
#if SYS_LOWEST_PRIO <= 63
    ptcb->SysTCBY            = (INT8U)((prio >> (INT8U)3) & (INT8U)0x07);
    ptcb->SysTCBX            = (INT8U) (prio & (INT8U)0x07);
    ptcb->SysTCBBitY         = (INT8U)(1 << ptcb->SysTCBY);
    ptcb->SysTCBBitX         = (INT8U)(1 << ptcb->SysTCBX);
#else
    ptcb->SysTCBY            = (INT8U)((prio >> (INT8U)4) & (INT8U)0x0F);
    ptcb->SysTCBX            = (INT8U) (prio & (INT8U)0x0F);
    ptcb->SysTCBBitY         = (INT16U)(1 << ptcb->SysTCBY);
    ptcb->SysTCBBitX         = (INT16U)(1 << ptcb->SysTCBX);
#endif
    SysRdyGrp               |= ptcb->SysTCBBitY;             /* Make task ready at original priority     */
    SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
    SysTCBPrioTbl[prio]      = ptcb;
}


#endif                                                     /* SYS_MUTEX_EN                              */
