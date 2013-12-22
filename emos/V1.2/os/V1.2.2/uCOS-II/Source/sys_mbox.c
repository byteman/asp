/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                       MESSAGE MAILBOX MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_MBOX.C
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

#if SYS_MBOX_EN > 0
/*
*********************************************************************************************************
*                                     ACCEPT MESSAGE FROM MAILBOX
*
* Description: This function checks the mailbox to see if a message is available.  Unlike SysMboxPend(),
*              SysMboxAccept() does not suspend the calling task if a message is not available.
*
* Arguments  : pevent        is a pointer to the event control block
*
* Returns    : != (void *)0  is the message in the mailbox if one is available.  The mailbox is cleared
*                            so the next time SysMboxAccept() is called, the mailbox will be empty.
*              == (void *)0  if the mailbox is empty or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass the proper event pointer.
*********************************************************************************************************
*/

#if SYS_MBOX_ACCEPT_EN > 0
void  *SysMboxAccept (SYS_EVENT *pevent)
{
    void      *pmsg;
#if SYS_CRITICAL_METHOD == 3                               /* Allocate storage for CPU status register  */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                        /* Validate 'pevent'                         */
        return ((void *)0);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {      /* Validate event block type                 */
        return ((void *)0);
    }
    SYS_ENTER_CRITICAL();
    pmsg               = pevent->SysEventPtr;
    pevent->SysEventPtr = (void *)0;                       /* Clear the mailbox                         */
    SYS_EXIT_CRITICAL();
    return (pmsg);                                        /* Return the message received (or NULL)     */
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                        CREATE A MESSAGE MAILBOX
*
* Description: This function creates a message mailbox if free event control blocks are available.
*
* Arguments  : pmsg          is a pointer to a message that you wish to deposit in the mailbox.  If
*                            you set this value to the NULL pointer (i.e. (void *)0) then the mailbox
*                            will be considered empty.
*
* Returns    : != (SYS_EVENT *)0  is a pointer to the event control clock (SYS_EVENT) associated with the
*                                created mailbox
*              == (SYS_EVENT *)0  if no event control blocks were available
*********************************************************************************************************
*/

SYS_EVENT  *SysMboxCreate (void *pmsg)
{
    SYS_EVENT  *pevent;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysIntNesting > 0) {                      /* See if called from ISR ...                         */
        return ((SYS_EVENT *)0);                  /* ... can't CREATE from an ISR                       */
    }
    SYS_ENTER_CRITICAL();
    pevent = SysEventFreeList;                    /* Get next free event control block                  */
    if (SysEventFreeList != (SYS_EVENT *)0) {      /* See if pool of free ECB pool was empty             */
        SysEventFreeList = (SYS_EVENT *)SysEventFreeList->SysEventPtr;
    }
    SYS_EXIT_CRITICAL();
    if (pevent != (SYS_EVENT *)0) {
        pevent->SysEventType    = SYS_EVENT_TYPE_MBOX;
        pevent->SysEventCnt     = 0;
        pevent->SysEventPtr     = pmsg;           /* Deposit message in event control block             */
#if SYS_EVENT_NAME_SIZE > 1
        pevent->SysEventName[0] = '?';
        pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
        SYS_EventWaitListInit(pevent);
    }
    return (pevent);                             /* Return pointer to event control block              */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                         DELETE A MAIBOX
*
* Description: This function deletes a mailbox and readies all tasks pending on the mailbox.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired
*                            mailbox.
*
*              opt           determines delete options as follows:
*                            opt == SYS_DEL_NO_PEND   Delete the mailbox ONLY if no task pending
*                            opt == SYS_DEL_ALWAYS    Deletes the mailbox even if tasks are waiting.
*                                                    In this case, all the tasks pending will be readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            SYS_ERR_NONE             The call was successful and the mailbox was deleted
*                            SYS_ERR_DEL_ISR          If you attempted to delete the mailbox from an ISR
*                            SYS_ERR_INVALID_OPT      An invalid option was specified
*                            SYS_ERR_TASK_WAITING     One or more tasks were waiting on the mailbox
*                            SYS_ERR_EVENT_TYPE       If you didn't pass a pointer to a mailbox
*                            SYS_ERR_PEVENT_NULL      If 'pevent' is a NULL pointer.
*
* Returns    : pevent        upon error
*              (SYS_EVENT *)0 if the mailbox was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the mailbox MUST check the return code of SysMboxPend().
*              2) SysMboxAccept() callers will not know that the intended mailbox has been deleted!
*              3) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the mailbox.
*              4) Because ALL tasks pending on the mailbox will be readied, you MUST be careful in
*                 applications where the mailbox is used for mutual exclusion because the resource(s)
*                 will no longer be guarded by the mailbox.
*********************************************************************************************************
*/

#if SYS_MBOX_DEL_EN > 0
SYS_EVENT  *SysMboxDel (SYS_EVENT *pevent, INT8U opt, INT8U *perr)
{
    BOOLEAN    tasks_waiting;
    SYS_EVENT  *pevent_return;
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
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {       /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return (pevent);
    }
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pevent);
    }
    SYS_ENTER_CRITICAL();
    if (pevent->SysEventGrp != 0) {                         /* See if any tasks waiting on mailbox      */
        tasks_waiting = SYS_TRUE;                           /* Yes                                      */
    } else {
        tasks_waiting = SYS_FALSE;                          /* No                                       */
    }
    switch (opt) {
        case SYS_DEL_NO_PEND:                               /* Delete mailbox only if no task waiting   */
             if (tasks_waiting == SYS_FALSE) {
#if SYS_EVENT_NAME_SIZE > 1
                 pevent->SysEventName[0] = '?';             /* Unknown name                             */
                 pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
                 pevent->SysEventType = SYS_EVENT_TYPE_UNUSED;
                 pevent->SysEventPtr  = SysEventFreeList;    /* Return Event Control Block to free list  */
                 pevent->SysEventCnt  = 0;
                 SysEventFreeList     = pevent;             /* Get next free event control block        */
                 SYS_EXIT_CRITICAL();
                 *perr               = SYS_ERR_NONE;
                 pevent_return       = (SYS_EVENT *)0;      /* Mailbox has been deleted                 */
             } else {
                 SYS_EXIT_CRITICAL();
                 *perr               = SYS_ERR_TASK_WAITING;
                 pevent_return       = pevent;
             }
             break;

        case SYS_DEL_ALWAYS:                                /* Always delete the mailbox                */
             while (pevent->SysEventGrp != 0) {             /* Ready ALL tasks waiting for mailbox      */
                 (void)SYS_EventTaskRdy(pevent, (void *)0, SYS_STAT_MBOX, SYS_STAT_PEND_OK);
             }
#if SYS_EVENT_NAME_SIZE > 1
             pevent->SysEventName[0] = '?';                 /* Unknown name                             */
             pevent->SysEventName[1] = SYS_ASCII_NUL;
#endif
             pevent->SysEventType    = SYS_EVENT_TYPE_UNUSED;
             pevent->SysEventPtr     = SysEventFreeList;     /* Return Event Control Block to free list  */
             pevent->SysEventCnt     = 0;
             SysEventFreeList        = pevent;              /* Get next free event control block        */
             SYS_EXIT_CRITICAL();
             if (tasks_waiting == SYS_TRUE) {               /* Reschedule only if task(s) were waiting  */
                 SYS_Sched();                               /* Find highest priority task ready to run  */
             }
             *perr         = SYS_ERR_NONE;
             pevent_return = (SYS_EVENT *)0;                /* Mailbox has been deleted                 */
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
*                                      PEND ON MAILBOX FOR A MESSAGE
*
* Description: This function waits for a message to be sent to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for a message to arrive at the mailbox up to the amount of time
*                            specified by this argument.  If you specify 0, however, your task will wait
*                            forever at the specified mailbox or, until a message arrives.
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            SYS_ERR_NONE         The call was successful and your task received a
*                                                message.
*                            SYS_ERR_TIMEOUT      A message was not received within the specified 'timeout'.
*                            SYS_ERR_PEND_ABORT   The wait on the mailbox was aborted.
*                            SYS_ERR_EVENT_TYPE   Invalid event type
*                            SYS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            SYS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer
*                            SYS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked
*
* Returns    : != (void *)0  is a pointer to the message received
*              == (void *)0  if no message was received or,
*                            if 'pevent' is a NULL pointer or,
*                            if you didn't pass the proper pointer to the event control block.
*********************************************************************************************************
*/
/*$PAGE*/
void  *SysMboxPend (SYS_EVENT *pevent, INT16U timeout, INT8U *perr)
{
    void      *pmsg;
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                         /* Validate 'perr'                               */
        return ((void *)0);
    }
    if (pevent == (SYS_EVENT *)0) {                    /* Validate 'pevent'                             */
        *perr = SYS_ERR_PEVENT_NULL;
        return ((void *)0);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {  /* Validate event block type                     */
        *perr = SYS_ERR_EVENT_TYPE;
        return ((void *)0);
    }
    if (SysIntNesting > 0) {                           /* See if called from ISR ...                    */
        *perr = SYS_ERR_PEND_ISR;                      /* ... can't PEND from an ISR                    */
        return ((void *)0);
    }
    if (SysLockNesting > 0) {                          /* See if called with scheduler locked ...       */
        *perr = SYS_ERR_PEND_LOCKED;                   /* ... can't PEND when locked                    */
        return ((void *)0);
    }
    SYS_ENTER_CRITICAL();
    pmsg = pevent->SysEventPtr;
    if (pmsg != (void *)0) {                          /* See if there is already a message             */
        pevent->SysEventPtr = (void *)0;               /* Clear the mailbox                             */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_NONE;
        return (pmsg);                                /* Return the message received (or NULL)         */
    }
    SysTCBCur->SysTCBStat     |= SYS_STAT_MBOX;          /* Message not available, task will pend         */
    SysTCBCur->SysTCBStatPend  = SYS_STAT_PEND_OK;
    SysTCBCur->SysTCBDly       = timeout;               /* Load timeout in TCB                           */
    SYS_EventTaskWait(pevent);                         /* Suspend task until event or timeout occurs    */
    SYS_EXIT_CRITICAL();
    SYS_Sched();                                       /* Find next highest priority task ready to run  */
    SYS_ENTER_CRITICAL();
    switch (SysTCBCur->SysTCBStatPend) {                /* See if we timed-out or aborted                */
        case SYS_STAT_PEND_OK:
             pmsg =  SysTCBCur->SysTCBMsg;
            *perr =  SYS_ERR_NONE;
             break;

        case SYS_STAT_PEND_ABORT:
             pmsg = (void *)0;
            *perr =  SYS_ERR_PEND_ABORT;               /* Indicate that we aborted                      */
             break;

        case SYS_STAT_PEND_TO:
        default:
             SYS_EventTaskRemove(SysTCBCur, pevent);
             pmsg = (void *)0;
            *perr =  SYS_ERR_TIMEOUT;                  /* Indicate that we didn't get event within TO   */
             break;
    }
    SysTCBCur->SysTCBStat          =  SYS_STAT_RDY;      /* Set   task  status to ready                   */
    SysTCBCur->SysTCBStatPend      =  SYS_STAT_PEND_OK;  /* Clear pend  status                            */
    SysTCBCur->SysTCBEventPtr      = (SYS_EVENT  *)0;    /* Clear event pointers                          */
#if (SYS_EVENT_MULTI_EN > 0)
    SysTCBCur->SysTCBEventMultiPtr = (SYS_EVENT **)0;
#endif
    SysTCBCur->SysTCBMsg           = (void      *)0;    /* Clear  received message                       */
    SYS_EXIT_CRITICAL();
    return (pmsg);                                    /* Return received message                       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                      ABORT WAITING ON A MESSAGE MAILBOX
*
* Description: This function aborts & readies any tasks currently waiting on a mailbox.  This function 
*              should be used to fault-abort the wait on the mailbox, rather than to normally signal
*              the mailbox via SysMboxPost() or SysMboxPostOpt().
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox.
*
*              opt           determines the type of ABORT performed:
*                            SYS_PEND_OPT_NONE         ABORT wait for a single task (HPT) waiting on the
*                                                     mailbox
*                            SYS_PEND_OPT_BROADCAST    ABORT wait for ALL tasks that are  waiting on the
*                                                     mailbox
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            SYS_ERR_NONE         No tasks were     waiting on the mailbox.
*                            SYS_ERR_PEND_ABORT   At least one task waiting on the mailbox was readied
*                                                and informed of the aborted wait; check return value 
*                                                for the number of tasks whose wait on the mailbox 
*                                                was aborted.
*                            SYS_ERR_EVENT_TYPE   If you didn't pass a pointer to a mailbox.
*                            SYS_ERR_PEVENT_NULL  If 'pevent' is a NULL pointer.
*
* Returns    : == 0          if no tasks were waiting on the mailbox, or upon error.
*              >  0          if one or more tasks waiting on the mailbox are now readied and informed.
*********************************************************************************************************
*/

#if SYS_MBOX_PEND_ABORT_EN > 0
INT8U  SysMboxPendAbort (SYS_EVENT *pevent, INT8U opt, INT8U *perr)
{
    INT8U      nbr_tasks;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return (0);
    }
    if (pevent == (SYS_EVENT *)0) {                         /* Validate 'pevent'                        */
        *perr = SYS_ERR_PEVENT_NULL;
        return (0);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {       /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return (0);
    }
    SYS_ENTER_CRITICAL();
    if (pevent->SysEventGrp != 0) {                         /* See if any task waiting on mailbox?      */
        nbr_tasks = 0;
        switch (opt) {
            case SYS_PEND_OPT_BROADCAST:                    /* Do we need to abort ALL waiting tasks?   */
                 while (pevent->SysEventGrp != 0) {         /* Yes, ready ALL tasks waiting on mailbox  */
                     (void)SYS_EventTaskRdy(pevent, (void *)0, SYS_STAT_MBOX, SYS_STAT_PEND_ABORT);
                     nbr_tasks++;
                 }
                 break;
             
            case SYS_PEND_OPT_NONE:
            default:                                       /* No,  ready HPT       waiting on mailbox  */
                 (void)SYS_EventTaskRdy(pevent, (void *)0, SYS_STAT_MBOX, SYS_STAT_PEND_ABORT);
                 nbr_tasks++;
                 break;
        }
        SYS_EXIT_CRITICAL();
        SYS_Sched();                                        /* Find HPT ready to run                    */
        *perr = SYS_ERR_PEND_ABORT;
        return (nbr_tasks);
    }
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (0);                                            /* No tasks waiting on mailbox              */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       PSysT MESSAGE TO A MAILBOX
*
* Description: This function sends a message to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              pmsg          is a pointer to the message to send.  You MUST NOT send a NULL pointer.
*
* Returns    : SYS_ERR_NONE          The call was successful and the message was sent
*              SYS_ERR_MBOX_FULL     If the mailbox already contains a message.  You can can only send one
*                                   message at a time and thus, the message MUST be consumed before you
*                                   are allowed to send another one.
*              SYS_ERR_EVENT_TYPE    If you are attempting to post to a non mailbox.
*              SYS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer
*              SYS_ERR_PSysT_NULL_PTR If you are attempting to post a NULL pointer
*
* Note(s)    : 1) HPT means Highest Priority Task
*********************************************************************************************************
*/

#if SYS_MBOX_PSysT_EN > 0
INT8U  SysMboxPost (SYS_EVENT *pevent, void *pmsg)
{
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (SYS_ERR_PEVENT_NULL);
    }
    if (pmsg == (void *)0) {                          /* Make sure we are not posting a NULL pointer   */
        return (SYS_ERR_PSysT_NULL_PTR);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {  /* Validate event block type                     */
        return (SYS_ERR_EVENT_TYPE);
    }
    SYS_ENTER_CRITICAL();
    if (pevent->SysEventGrp != 0) {                    /* See if any task pending on mailbox            */
                                                      /* Ready HPT waiting on event                    */
        (void)SYS_EventTaskRdy(pevent, pmsg, SYS_STAT_MBOX, SYS_STAT_PEND_OK);
        SYS_EXIT_CRITICAL();
        SYS_Sched();                                   /* Find highest priority task ready to run       */
        return (SYS_ERR_NONE);
    }
    if (pevent->SysEventPtr != (void *)0) {            /* Make sure mailbox doesn't already have a msg  */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_MBOX_FULL);
    }
    pevent->SysEventPtr = pmsg;                        /* Place message in mailbox                      */
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       PSysT MESSAGE TO A MAILBOX
*
* Description: This function sends a message to a mailbox
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              pmsg          is a pointer to the message to send.  You MUST NOT send a NULL pointer.
*
*              opt           determines the type of PSysT performed:
*                            SYS_PSysT_OPT_NONE         PSysT to a single waiting task
*                                                     (Identical to SysMboxPost())
*                            SYS_PSysT_OPT_BROADCAST    PSysT to ALL tasks that are waiting on the mailbox
*
*                            SYS_PSysT_OPT_NO_SCHED     Indicates that the scheduler will NOT be invoked
*
* Returns    : SYS_ERR_NONE          The call was successful and the message was sent
*              SYS_ERR_MBOX_FULL     If the mailbox already contains a message.  You can can only send one
*                                   message at a time and thus, the message MUST be consumed before you
*                                   are allowed to send another one.
*              SYS_ERR_EVENT_TYPE    If you are attempting to post to a non mailbox.
*              SYS_ERR_PEVENT_NULL   If 'pevent' is a NULL pointer
*              SYS_ERR_PSysT_NULL_PTR If you are attempting to post a NULL pointer
*
* Note(s)    : 1) HPT means Highest Priority Task
*
* Warning    : Interrupts can be disabled for a long time if you do a 'broadcast'.  In fact, the
*              interrupt disable time is proportional to the number of tasks waiting on the mailbox.
*********************************************************************************************************
*/

#if SYS_MBOX_PSysT_OPT_EN > 0
INT8U  SysMboxPostOpt (SYS_EVENT *pevent, void *pmsg, INT8U opt)
{
#if SYS_CRITICAL_METHOD == 3                           /* Allocate storage for CPU status register      */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                    /* Validate 'pevent'                             */
        return (SYS_ERR_PEVENT_NULL);
    }
    if (pmsg == (void *)0) {                          /* Make sure we are not posting a NULL pointer   */
        return (SYS_ERR_PSysT_NULL_PTR);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {  /* Validate event block type                     */
        return (SYS_ERR_EVENT_TYPE);
    }
    SYS_ENTER_CRITICAL();
    if (pevent->SysEventGrp != 0) {                    /* See if any task pending on mailbox            */
        if ((opt & SYS_PSysT_OPT_BROADCAST) != 0x00) {  /* Do we need to post msg to ALL waiting tasks ? */
            while (pevent->SysEventGrp != 0) {         /* Yes, Post to ALL tasks waiting on mailbox     */
                (void)SYS_EventTaskRdy(pevent, pmsg, SYS_STAT_MBOX, SYS_STAT_PEND_OK);
            }
        } else {                                      /* No,  Post to HPT waiting on mbox              */
            (void)SYS_EventTaskRdy(pevent, pmsg, SYS_STAT_MBOX, SYS_STAT_PEND_OK);
        }
        SYS_EXIT_CRITICAL();
        if ((opt & SYS_PSysT_OPT_NO_SCHED) == 0) {	  /* See if scheduler needs to be invoked          */
            SYS_Sched();                               /* Find HPT ready to run                         */
        }
        return (SYS_ERR_NONE);
    }
    if (pevent->SysEventPtr != (void *)0) {            /* Make sure mailbox doesn't already have a msg  */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_MBOX_FULL);
    }
    pevent->SysEventPtr = pmsg;                        /* Place message in mailbox                      */
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        QUERY A MESSAGE MAILBOX
*
* Description: This function obtains information about a message mailbox.
*
* Arguments  : pevent        is a pointer to the event control block associated with the desired mailbox
*
*              p_mbox_data   is a pointer to a structure that will contain information about the message
*                            mailbox.
*
* Returns    : SYS_ERR_NONE         The call was successful and the message was sent
*              SYS_ERR_EVENT_TYPE   If you are attempting to obtain data from a non mailbox.
*              SYS_ERR_PEVENT_NULL  If 'pevent'      is a NULL pointer
*              SYS_ERR_PDATA_NULL   If 'p_mbox_data' is a NULL pointer
*********************************************************************************************************
*/

#if SYS_MBOX_QUERY_EN > 0
INT8U  SysMboxQuery (SYS_EVENT *pevent, SYS_MBOX_DATA *p_mbox_data)
{
    INT8U      i;
#if SYS_LOWEST_PRIO <= 63
    INT8U     *psrc;
    INT8U     *pdest;
#else
    INT16U    *psrc;
    INT16U    *pdest;
#endif
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (pevent == (SYS_EVENT *)0) {                         /* Validate 'pevent'                        */
        return (SYS_ERR_PEVENT_NULL);
    }
    if (p_mbox_data == (SYS_MBOX_DATA *)0) {                /* Validate 'p_mbox_data'                   */
        return (SYS_ERR_PDATA_NULL);
    }
#endif
    if (pevent->SysEventType != SYS_EVENT_TYPE_MBOX) {       /* Validate event block type                */
        return (SYS_ERR_EVENT_TYPE);
    }
    SYS_ENTER_CRITICAL();
    p_mbox_data->SysEventGrp = pevent->SysEventGrp;          /* Copy message mailbox wait list           */
    psrc                    = &pevent->SysEventTbl[0];
    pdest                   = &p_mbox_data->SysEventTbl[0];
    for (i = 0; i < SYS_EVENT_TBL_SIZE; i++) {
        *pdest++ = *psrc++;
    }
    p_mbox_data->SysMsg = pevent->SysEventPtr;               /* Get message from mailbox                 */
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif                                                     /* SYS_MBOX_QUERY_EN                         */
#endif                                                     /* SYS_MBOX_EN                               */
