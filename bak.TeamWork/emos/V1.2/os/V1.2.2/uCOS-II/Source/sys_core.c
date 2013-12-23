/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                             CORE FUNCTIONS
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_CORE.C
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
#define  SYS_GLOBALS
#include <C2.h>
#endif

/*
*********************************************************************************************************
*                                       PRIORITY RESOLUTION TABLE
*
* Note: Index into table is bit pattern to resolve highest priority
*       Indexed value corresponds to highest priority bit position (i.e. 0..7)
*********************************************************************************************************
*/

INT8U  const  SysUnMapTbl[256] = {
    0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x00 to 0x0F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x10 to 0x1F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x20 to 0x2F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x30 to 0x3F                             */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x40 to 0x4F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x50 to 0x5F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x60 to 0x6F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x70 to 0x7F                             */
    7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x80 to 0x8F                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0x90 to 0x9F                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xA0 to 0xAF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xB0 to 0xBF                             */
    6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xC0 to 0xCF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xD0 to 0xDF                             */
    5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,       /* 0xE0 to 0xEF                             */
    4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0        /* 0xF0 to 0xFF                             */
};

/*$PAGE*/
/*
*********************************************************************************************************
*                                       FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  SYS_InitEventList(void);

static  void  SYS_InitMisc(void);

static  void  SYS_InitRdyList(void);

static  void  SYS_InitTaskIdle(void);

#if SYS_TASK_STAT_EN > 0
static  void  SYS_InitTaskStat(void);
#endif

static  void  SYS_InitTCBList(void);

static  void  SYS_SchedNew(void);

/*$PAGE*/
/*
*********************************************************************************************************
*                         GET THE NAME OF A SEMAPHORE, MUTEX, MAILBOX or QUEUE
*
* Description: This function is used to obtain the name assigned to a semaphore, mutex, mailbox or queue.
*
* Arguments  : pevent    is a pointer to the event group.  'pevent' can point either to a semaphore,
*                        a mutex, a mailbox or a queue.  Where this function is concerned, the actual
*                        type is irrelevant.
*
*              pname     is a pointer to an ASCII string that will receive the name of the semaphore,
*                        mutex, mailbox or queue.  The string must be able to hold at least
*                        SYS_EVENT_NAME_SIZE characters.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the name was copied to 'pname'
*                        SYS_ERR_EVENT_TYPE          if 'pevent' is not pointing to the proper event
*                                                   control block type.
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_PEVENT_NULL         if you passed a NULL pointer for 'pevent'
*                        SYS_ERR_NAME_GET_ISR        if you are trying to call this function from an ISR
*
* Returns    : The length of the string or 0 if the 'pevent' is a NULL pointer.
*********************************************************************************************************
*/

#if (SYS_EVENT_EN) && (SYS_EVENT_NAME_SIZE > 1)
INT8U  SysEventNameGet (SYS_EVENT *pevent, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return (0);
    }
    if (pevent == (SYS_EVENT *)0) {               /* Is 'pevent' a NULL pointer?                        */
        *perr = SYS_ERR_PEVENT_NULL;
        return (0);
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = SYS_ERR_PNAME_NULL;
        return (0);
    }
#endif
    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        *perr  = SYS_ERR_NAME_GET_ISR;
        return (0);
    }
    switch (pevent->SysEventType) {
        case SYS_EVENT_TYPE_SEM:
        case SYS_EVENT_TYPE_MUTEX:
        case SYS_EVENT_TYPE_MBOX:
        case SYS_EVENT_TYPE_Q:
             break;

        default:
             *perr = SYS_ERR_EVENT_TYPE;
             return (0);
    }
    SYS_ENTER_CRITICAL();
    len   = SYS_StrCopy(pname, pevent->SysEventName);   /* Copy name from SYS_EVENT                       */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (len);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                         ASSIGN A NAME TO A SEMAPHORE, MUTEX, MAILBOX or QUEUE
*
* Description: This function assigns a name to a semaphore, mutex, mailbox or queue.
*
* Arguments  : pevent    is a pointer to the event group.  'pevent' can point either to a semaphore,
*                        a mutex, a mailbox or a queue.  Where this function is concerned, it doesn't
*                        matter the actual type.
*
*              pname     is a pointer to an ASCII string that will be used as the name of the semaphore,
*                        mutex, mailbox or queue.  The string must be able to hold at least
*                        SYS_EVENT_NAME_SIZE characters.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the requested task is resumed
*                        SYS_ERR_EVENT_TYPE          if 'pevent' is not pointing to the proper event
*                                                   control block type.
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_PEVENT_NULL         if you passed a NULL pointer for 'pevent'
*                        SYS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/

#if (SYS_EVENT_EN) && (SYS_EVENT_NAME_SIZE > 1)
void  SysEventNameSet (SYS_EVENT *pevent, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return;
    }
    if (pevent == (SYS_EVENT *)0) {               /* Is 'pevent' a NULL pointer?                        */
        *perr = SYS_ERR_PEVENT_NULL;
        return;
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = SYS_ERR_PNAME_NULL;
        return;
    }
#endif
    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        *perr = SYS_ERR_NAME_SET_ISR;
        return;
    }
    switch (pevent->SysEventType) {
        case SYS_EVENT_TYPE_SEM:
        case SYS_EVENT_TYPE_MUTEX:
        case SYS_EVENT_TYPE_MBOX:
        case SYS_EVENT_TYPE_Q:
             break;

        default:
             *perr = SYS_ERR_EVENT_TYPE;
             return;
    }
    SYS_ENTER_CRITICAL();
    len = SYS_StrLen(pname);                           /* Can we fit the string in the storage area?    */
    if (len > (SYS_EVENT_NAME_SIZE - 1)) {             /* No                                            */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_EVENT_NAME_TOO_LONG;
        return;
    }
    (void)SYS_StrCopy(pevent->SysEventName, pname);     /* Yes, copy name to the event control block     */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                      PEND ON MULTIPLE EVENTS
*
* Description: This function waits for multiple events.  If multiple events are ready at the start of the
*              pend call, then all available events are returned as ready.  If the task must pend on the
*              multiple events, then only the first posted or aborted event is returned as ready.
*
* Arguments  : pevents_pend  is a pointer to a NULL-terminated array of event control blocks to wait for.
*
*              pevents_rdy   is a pointer to an array to return which event control blocks are available
*                            or ready.  The size of the array MUST be greater than or equal to the size
*                            of the 'pevents_pend' array, including terminating NULL.
*
*              pmsgs_rdy     is a pointer to an array to return messages from any available message-type
*                            events.  The size of the array MUST be greater than or equal to the size of
*                            the 'pevents_pend' array, excluding the terminating NULL.  Since NULL
*                            messages are valid messages, this array cannot be NULL-terminated.  Instead,
*                            every available message-type event returns its messages in the 'pmsgs_rdy'
*                            array at the same index as the event is returned in the 'pevents_rdy' array.
*                            All other 'pmsgs_rdy' array indices are filled with NULL messages.
*
*              timeout       is an optional timeout period (in clock ticks).  If non-zero, your task will
*                            wait for the resources up to the amount of time specified by this argument.
*                            If you specify 0, however, your task will wait forever for the specified
*                            events or, until the resources becomes available (or the events occur).
*
*              perr          is a pointer to where an error message will be deposited.  Possible error
*                            messages are:
*
*                            SYS_ERR_NONE         The call was successful and your task owns the resources
*                                                or, the events you are waiting for occurred; check the
*                                                'pevents_rdy' array for which events are available.
*                            SYS_ERR_PEND_ABORT   The wait on the events was aborted; check the
*                                                'pevents_rdy' array for which events were aborted.
*                            SYS_ERR_TIMEOUT      The events were not received within the specified
*                                                'timeout'.
*                            SYS_ERR_PEVENT_NULL  If 'pevents_pend', 'pevents_rdy', or 'pmsgs_rdy' is a 
*                                                NULL pointer.
*                            SYS_ERR_EVENT_TYPE   If you didn't pass a pointer to an array of semaphores, 
*                                                mailboxes, and/or queues.
*                            SYS_ERR_PEND_ISR     If you called this function from an ISR and the result
*                                                would lead to a suspension.
*                            SYS_ERR_PEND_LOCKED  If you called this function when the scheduler is locked.
*
* Returns    : >  0          the number of events returned as ready or aborted.
*              == 0          if no events are returned as ready because of timeout or upon error.
*
* Notes      : 1) a. Validate 'pevents_pend' array as valid SYS_EVENTs : 
*
*                        semaphores, mailboxes, queues
*
*                 b. Return ALL available events and messages, if any
*
*                 c. Add    current task priority as pending to   each events's wait list
*                      Performed in SYS_EventTaskWaitMulti()
*
*                 d. Wait on any of multiple events
*
*                 e. Remove current task priority as pending from each events's wait list
*                      Performed in SYS_EventTaskRdy(), if events posted or aborted
*
*                 f. Return any event posted or aborted, if any
*                      else
*                    Return timeout
*
*              2) 'pevents_rdy' initialized to NULL PRIOR to all other validation or function handling in 
*                 case of any error(s).
*********************************************************************************************************
*/
/*$PAGE*/
#if ((SYS_EVENT_EN) && (SYS_EVENT_MULTI_EN > 0))
INT16U  SysEventPendMulti (SYS_EVENT **pevents_pend, SYS_EVENT **pevents_rdy, void **pmsgs_rdy, INT16U timeout, INT8U *perr)
{
    SYS_EVENT  **pevents;
    SYS_EVENT   *pevent;
#if ((SYS_Q_EN > 0) && (SYS_MAX_QS > 0))
    SYS_Q       *pq;
#endif
    BOOLEAN     events_rdy;
    INT16U      events_rdy_nbr;
    INT8U       events_stat;
#if (SYS_CRITICAL_METHOD == 3)                           /* Allocate storage for CPU status register    */
    SYS_CPU_SR   cpu_sr = 0;
#endif



#if (SYS_ARG_CHK_EN > 0)
    if (perr == (INT8U *)0) {                           /* Validate 'perr'                             */
        return (0);
    }
    if (pevents_pend == (SYS_EVENT **)0) {               /* Validate 'pevents_pend'                     */
       *perr =  SYS_ERR_PEVENT_NULL;
        return (0);
    }
    if (pevents_rdy  == (SYS_EVENT **)0) {               /* Validate 'pevents_rdy'                      */
       *perr =  SYS_ERR_PEVENT_NULL;
        return (0);
    }
    if (pmsgs_rdy == (void **)0) {                      /* Validate 'pmsgs_rdy'                        */
       *perr =  SYS_ERR_PEVENT_NULL;
        return (0);
    }
#endif

   *pevents_rdy = (SYS_EVENT *)0;                        /* Init array to NULL in case of errors        */

    pevents     =  pevents_pend;
    pevent      = *pevents;
    while  (pevent != (SYS_EVENT *)0) {
        switch (pevent->SysEventType) {                  /* Validate event block types                  */
#if (SYS_SEM_EN  > 0)
            case SYS_EVENT_TYPE_SEM:
                 break;
#endif
#if (SYS_MBOX_EN > 0)
            case SYS_EVENT_TYPE_MBOX:
                 break;
#endif
#if ((SYS_Q_EN   > 0) && (SYS_MAX_QS > 0))
            case SYS_EVENT_TYPE_Q:
                 break;
#endif

            case SYS_EVENT_TYPE_MUTEX:                                            
            case SYS_EVENT_TYPE_FLAG:
            default:           
                *perr = SYS_ERR_EVENT_TYPE;
                 return (0);
        }
        pevents++;
        pevent = *pevents;
    }

    if (SysIntNesting  > 0) {                            /* See if called from ISR ...                  */
       *perr =  SYS_ERR_PEND_ISR;                        /* ... can't PEND from an ISR                  */
        return (0);
    }
    if (SysLockNesting > 0) {                            /* See if called with scheduler locked ...     */
       *perr =  SYS_ERR_PEND_LOCKED;                     /* ... can't PEND when locked                  */
        return (0);
    }

/*$PAGE*/
    SYS_ENTER_CRITICAL();
    events_rdy     =  SYS_FALSE;
    events_rdy_nbr =  0;
    events_stat    =  SYS_STAT_RDY;
    pevents        =  pevents_pend;
    pevent         = *pevents;
    while (pevent != (SYS_EVENT *)0) {                   /* See if any events already available         */
        switch (pevent->SysEventType) {
#if (SYS_SEM_EN > 0)
            case SYS_EVENT_TYPE_SEM:
                 if (pevent->SysEventCnt > 0) {          /* If semaphore count > 0, resource available; */
                     pevent->SysEventCnt--;              /* ... decrement semaphore,                ... */
                    *pevents_rdy++ =  pevent;           /* ... and return available semaphore event    */
                      events_rdy   =  SYS_TRUE;
                    *pmsgs_rdy++   = (void *)0;         /* NO message returned  for semaphores         */
                      events_rdy_nbr++;

                 } else {
                      events_stat |=  SYS_STAT_SEM;      /* Configure multi-pend for semaphore events   */
                 }
                 break;
#endif

#if (SYS_MBOX_EN > 0)
            case SYS_EVENT_TYPE_MBOX:
                 if (pevent->SysEventPtr != (void *)0) { /* If mailbox NOT empty;                   ... */
                                                        /* ... return available message,           ... */
                    *pmsgs_rdy++         = (void *)pevent->SysEventPtr;
                     pevent->SysEventPtr  = (void *)0;
                    *pevents_rdy++       =  pevent;     /* ... and return available mailbox event      */
                      events_rdy         =  SYS_TRUE;
                      events_rdy_nbr++;

                 } else {
                      events_stat |= SYS_STAT_MBOX;      /* Configure multi-pend for mailbox events     */
                 }
                 break;
#endif

#if ((SYS_Q_EN > 0) && (SYS_MAX_QS > 0))
            case SYS_EVENT_TYPE_Q:
                 pq = (SYS_Q *)pevent->SysEventPtr;
                 if (pq->SysQEntries > 0) {              /* If queue NOT empty;                     ... */
                                                        /* ... return available message,           ... */
                    *pmsgs_rdy++ = (void *)*pq->SysQOut++;
                     if (pq->SysQOut == pq->SysQEnd) {    /* If OUT ptr at queue end, ...                */
                         pq->SysQOut  = pq->SysQStart;    /* ... wrap   to queue start                   */
                     }
                     pq->SysQEntries--;                  /* Update number of queue entries              */
                    *pevents_rdy++ = pevent;            /* ... and return available queue event        */
                      events_rdy   = SYS_TRUE;
                      events_rdy_nbr++;

                 } else {
                      events_stat |= SYS_STAT_Q;         /* Configure multi-pend for queue events       */
                 }
                 break;
#endif

            case SYS_EVENT_TYPE_MUTEX:                                            
            case SYS_EVENT_TYPE_FLAG:
            default:           
                 SYS_EXIT_CRITICAL();
                *pevents_rdy = (SYS_EVENT *)0;           /* NULL terminate return event array           */
                *perr        =  SYS_ERR_EVENT_TYPE;
                 return (events_rdy_nbr);
        }
        pevents++;
        pevent = *pevents;
    }

    if ( events_rdy == SYS_TRUE) {                       /* Return any events already available         */
       *pevents_rdy = (SYS_EVENT *)0;                    /* NULL terminate return event array           */
        SYS_EXIT_CRITICAL();
       *perr        =  SYS_ERR_NONE;
        return (events_rdy_nbr);
    }
/*$PAGE*/
                                                        /* Otherwise, must wait until any event occurs */
    SysTCBCur->SysTCBStat     |= events_stat  |           /* Resource not available, ...                 */
                               SYS_STAT_MULTI;           /* ... pend on multiple events                 */
    SysTCBCur->SysTCBStatPend  = SYS_STAT_PEND_OK;
    SysTCBCur->SysTCBDly       = timeout;                 /* Store pend timeout in TCB                   */
    SYS_EventTaskWaitMulti(pevents_pend);                /* Suspend task until events or timeout occurs */

    SYS_EXIT_CRITICAL();
    SYS_Sched();                                         /* Find next highest priority task ready       */
    SYS_ENTER_CRITICAL();

    switch (SysTCBCur->SysTCBStatPend) {                  /* Handle event posted, aborted, or timed-out  */
        case SYS_STAT_PEND_OK:
        case SYS_STAT_PEND_ABORT:
             pevent = SysTCBCur->SysTCBEventPtr;
             if (pevent != (SYS_EVENT *)0) {             /* If task event ptr != NULL, ...              */
                *pevents_rdy++ =  pevent;               /* ... return available event ...              */
                *pevents_rdy   = (SYS_EVENT *)0;         /* ... & NULL terminate return event array     */
                  events_rdy_nbr++;

             } else {                                   /* Else NO event available, handle as timeout  */
                 SysTCBCur->SysTCBStatPend = SYS_STAT_PEND_TO;
                 SYS_EventTaskRemoveMulti(SysTCBCur, pevents_pend);
             }
			 break;

        case SYS_STAT_PEND_TO:                           /* If events timed out, ...                    */
        default:                                        /* ... remove task from events' wait lists     */
             SYS_EventTaskRemoveMulti(SysTCBCur, pevents_pend);
             break;
    }

    switch (SysTCBCur->SysTCBStatPend) {
        case SYS_STAT_PEND_OK:
             switch (pevent->SysEventType) {             /* Return event's message                      */
#if (SYS_SEM_EN > 0)
                 case SYS_EVENT_TYPE_SEM:
                     *pmsgs_rdy++ = (void *)0;          /* NO message returned for semaphores          */
                      break;
#endif

#if ((SYS_MBOX_EN > 0) ||                 \
    ((SYS_Q_EN    > 0) && (SYS_MAX_QS > 0)))
                 case SYS_EVENT_TYPE_MBOX:
                 case SYS_EVENT_TYPE_Q:
                     *pmsgs_rdy++ = (void *)SysTCBCur->SysTCBMsg;     /* Return received message         */
                      break;
#endif

                 case SYS_EVENT_TYPE_MUTEX:                                       
                 case SYS_EVENT_TYPE_FLAG:
                 default:           
                      SYS_EXIT_CRITICAL();
                     *pevents_rdy = (SYS_EVENT *)0;      /* NULL terminate return event array           */
                     *perr        =  SYS_ERR_EVENT_TYPE;
                      return (events_rdy_nbr);
             }
            *perr = SYS_ERR_NONE;
             break;

        case SYS_STAT_PEND_ABORT:
            *pmsgs_rdy++ = (void *)0;                   /* NO message returned for abort               */
            *perr        =  SYS_ERR_PEND_ABORT;          /* Indicate that event  aborted                */
             break;
                                                        
        case SYS_STAT_PEND_TO:                                                
        default:        
            *pmsgs_rdy++ = (void *)0;                   /* NO message returned for timeout             */
            *perr        =  SYS_ERR_TIMEOUT;             /* Indicate that events timed out              */
             break;
    }

    SysTCBCur->SysTCBStat          =  SYS_STAT_RDY;        /* Set   task  status to ready                 */
    SysTCBCur->SysTCBStatPend      =  SYS_STAT_PEND_OK;    /* Clear pend  status                          */
    SysTCBCur->SysTCBEventPtr      = (SYS_EVENT  *)0;      /* Clear event pointers                        */
    SysTCBCur->SysTCBEventMultiPtr = (SYS_EVENT **)0;
    SysTCBCur->SysTCBMsg           = (void      *)0;      /* Clear task  message                         */
    SYS_EXIT_CRITICAL();

    return (events_rdy_nbr);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*
* Description: This function is used to initialize the internals of uC/Sys-II and MUST be called prior to
*              creating any uC/Sys-II object and, prior to calling SysStart().
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  SysInit (void)
{
    SysInitHookBegin();                                           /* Call port specific initialization code   */

    SYS_InitMisc();                                               /* Initialize miscellaneous variables       */

    SYS_InitRdyList();                                            /* Initialize the Ready List                */

    SYS_InitTCBList();                                            /* Initialize the free list of SYS_TCBs      */

    SYS_InitEventList();                                          /* Initialize the free list of SYS_EVENTs    */

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
    SYS_FlagInit();                                               /* Initialize the event flag structures     */
#endif

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
    SYS_MemInit();                                                /* Initialize the memory manager            */
#endif

#if (SYS_Q_EN > 0) && (SYS_MAX_QS > 0)
    SYS_QInit();                                                  /* Initialize the message queue structures  */
#endif

    SYS_InitTaskIdle();                                           /* Create the Idle Task                     */
#if SYS_TASK_STAT_EN > 0
    SYS_InitTaskStat();                                           /* Create the Statistic Task                */
#endif

#if SYS_TMR_EN > 0
    SysTmr_Init();                                                /* Initialize the Timer Manager             */
#endif

    SysInitHookEnd();                                             /* Call port specific init. code            */

#if SYS_DEBUG_EN > 0
    SysDebugInit();
#endif
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                              ENTER ISR
*
* Description: This function is used to notify uC/Sys-II that you are about to service an interrupt
*              service routine (ISR).  This allows uC/Sys-II to keep track of interrupt nesting and thus
*              only perform rescheduling at the last nested ISR.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) This function should be called ith interrupts already disabled
*              2) Your ISR can directly increment SysIntNesting without calling this function because
*                 SysIntNesting has been declared 'global'.
*              3) You MUST still call SysIntExit() even though you increment SysIntNesting directly.
*              4) You MUST invoke SysIntEnter() and SysIntExit() in pair.  In other words, for every call
*                 to SysIntEnter() at the beginning of the ISR you MUST have a call to SysIntExit() at the
*                 end of the ISR.
*              5) You are allowed to nest interrupts up to 255 levels deep.
*              6) I removed the SYS_ENTER_CRITICAL() and SYS_EXIT_CRITICAL() around the increment because
*                 SysIntEnter() is always called with interrupts disabled.
*********************************************************************************************************
*/

void  SysIntEnter (void)
{
    if (SysRunning == SYS_TRUE) {
        if (SysIntNesting < 255u) {
            SysIntNesting++;                      /* Increment ISR nesting level                        */
        }
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               EXIT ISR
*
* Description: This function is used to notify uC/Sys-II that you have completed serviving an ISR.  When
*              the last nested ISR has completed, uC/Sys-II will call the scheduler to determine whether
*              a new, high-priority task, is ready to run.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke SysIntEnter() and SysIntExit() in pair.  In other words, for every call
*                 to SysIntEnter() at the beginning of the ISR you MUST have a call to SysIntExit() at the
*                 end of the ISR.
*              2) Rescheduling is prevented when the scheduler is locked (see SYS_SchedLock())
*********************************************************************************************************
*/

void  SysIntExit (void)
{
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysRunning == SYS_TRUE) {
        SYS_ENTER_CRITICAL();
        if (SysIntNesting > 0) {                            /* Prevent SysIntNesting from wrapping       */
            SysIntNesting--;
        }
        if (SysIntNesting == 0) {                           /* Reschedule only if all ISRs complete ... */
            if (SysLockNesting == 0) {                      /* ... and not locked.                      */
                SYS_SchedNew();
//add_by_StreakingMCU
				SysTCBHighRdy = SysTCBPrioTbl[SysPrioHighRdy];
//end_add_by_StreakingMCU				
                //if (SysPrioHighRdy != SysPrioCur) {          /* No Ctx Sw if current task is highest rdy */
				if (SysTCBHighRdy != SysTCBCur) {  
//delete_by_StreakingMCU
//                    SysTCBHighRdy  = SysTCBPrioTbl[SysPrioHighRdy];
//end_delete_by_StreakingMCU					
#if SYS_TASK_PROFILE_EN > 0
                    SysTCBHighRdy->SysTCBCtxSwCtr++;         /* Inc. # of context switches to this task  */
#endif
                    SysCtxSwCtr++;                          /* Keep track of the number of ctx switches */
                    SysIntCtxSw();                          /* Perform interrupt level ctx switch       */
                }
            }
        }
        SYS_EXIT_CRITICAL();
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                          PREVENT SCHEDULING
*
* Description: This function is used to prevent rescheduling to take place.  This allows your application
*              to prevent context switches until you are ready to permit context switching.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke SysSchedLock() and SysSchedUnlock() in pair.  In other words, for every
*                 call to SysSchedLock() you MUST have a call to SysSchedUnlock().
*********************************************************************************************************
*/

#if SYS_SCHED_LOCK_EN > 0
void  SysSchedLock (void)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysRunning == SYS_TRUE) {                  /* Make sure multitasking is running                  */
        SYS_ENTER_CRITICAL();
        if (SysIntNesting == 0) {                 /* Can't call from an ISR                             */
            if (SysLockNesting < 255u) {          /* Prevent SysLockNesting from wrapping back to 0      */
                SysLockNesting++;                 /* Increment lock nesting level                       */
            }
        }
        SYS_EXIT_CRITICAL();
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          ENABLE SCHEDULING
*
* Description: This function is used to re-allow rescheduling.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) You MUST invoke SysSchedLock() and SysSchedUnlock() in pair.  In other words, for every
*                 call to SysSchedLock() you MUST have a call to SysSchedUnlock().
*********************************************************************************************************
*/

#if SYS_SCHED_LOCK_EN > 0
void  SysSchedUnlock (void)
{
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (SysRunning == SYS_TRUE) {                            /* Make sure multitasking is running        */
        SYS_ENTER_CRITICAL();
        if (SysLockNesting > 0) {                           /* Do not decrement if already 0            */
            SysLockNesting--;                               /* Decrement lock nesting level             */
            if (SysLockNesting == 0) {                      /* See if scheduler is enabled and ...      */
                if (SysIntNesting == 0) {                   /* ... not in an ISR                        */
                    SYS_EXIT_CRITICAL();
                    SYS_Sched();                            /* See if a HPT is ready                    */
                } else {
                    SYS_EXIT_CRITICAL();
                }
            } else {
                SYS_EXIT_CRITICAL();
            }
        } else {
            SYS_EXIT_CRITICAL();
        }
    }
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          START MULTITASKING
*
* Description: This function is used to start the multitasking process which lets uC/Sys-II manages the
*              task that you have created.  Before you can call SysStart(), you MUST have called SysInit()
*              and you MUST have created at least one task.
*
* Arguments  : none
*
* Returns    : none
*
* Note       : SysStartHighRdy() MUST:
*                 a) Call SysTaskSwHook() then,
*                 b) Set SysRunning to SYS_TRUE.
*                 c) Load the context of the task pointed to by SysTCBHighRdy.
*                 d_ Execute the task.
*********************************************************************************************************
*/

void  SysStart (void)
{
    if (SysRunning == SYS_FALSE) {
        SYS_SchedNew();                               /* Find highest priority's task priority number   */
        SysPrioCur     = SysPrioHighRdy;
        SysTCBHighRdy  = SysTCBPrioTbl[SysPrioHighRdy]; /* Point to highest priority task ready to run    */
        SysTCBCur      = SysTCBHighRdy;
        SysStartHighRdy();                            /* Execute target specific code to start task     */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        STATISTICS INITIALIZATION
*
* Description: This function is called by your application to establish CPU usage by first determining
*              how high a 32-bit counter would count to in 1 second if no other tasks were to execute
*              during that time.  CPU usage is then determined by a low priority task which keeps track
*              of this 32-bit counter every second but this time, with other tasks running.  CPU usage is
*              determined by:
*
*                                             SysIdleCtr
*                 CPU Usage (%) = 100 * (1 - ------------)
*                                            SysIdleCtrMax
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if SYS_TASK_STAT_EN > 0
void  SysStatInit (void)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    SysTimeDly(2);                                /* Synchronize with clock tick                        */
    SYS_ENTER_CRITICAL();
    SysIdleCtr    = 0L;                           /* Clear idle counter                                 */
    SYS_EXIT_CRITICAL();
    SysTimeDly(SYS_TICKS_PER_SEC / 10);            /* Determine MAX. idle counter value for 1/10 second  */
    SYS_ENTER_CRITICAL();
    SysIdleCtrMax = SysIdleCtr;                    /* Store maximum idle counter count in 1/10 second    */
    SysStatRdy    = SYS_TRUE;
    SYS_EXIT_CRITICAL();
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                         PROCESS SYSTEM TICK
*
* Description: This function is used to signal to uC/Sys-II the occurrence of a 'system tick' (also known
*              as a 'clock tick').  This function should be called by the ticker ISR but, can also be
*              called by a high priority task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

void  SysTimeTick (void)
{
    SYS_TCB    *ptcb;
#if SYS_TICK_STEP_EN > 0
    BOOLEAN    step;
#endif
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register     */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_TIME_TICK_HOOK_EN > 0
    SysTimeTickHook();                                      /* Call user definable hook                     */
#endif
#if SYS_TIME_GET_SET_EN > 0
    SYS_ENTER_CRITICAL();                                   /* Update the 32-bit tick counter               */
    SysTime++;
    SYS_EXIT_CRITICAL();
#endif
    if (SysRunning == SYS_TRUE) {
#if SYS_TICK_STEP_EN > 0
        switch (SysTickStepState) {                         /* Determine whether we need to process a tick  */
            case SYS_TICK_STEP_DIS:                         /* Yes, stepping is disabled                    */
                 step = SYS_TRUE;
                 break;

            case SYS_TICK_STEP_WAIT:                        /* No,  waiting for uC/Sys-View to set ...       */
                 step = SYS_FALSE;                          /*      .. SysTickStepState to SYS_TICK_STEP_ONCE */
                 break;

            case SYS_TICK_STEP_ONCE:                        /* Yes, process tick once and wait for next ... */
                 step            = SYS_TRUE;                /*      ... step command from uC/Sys-View        */
                 SysTickStepState = SYS_TICK_STEP_WAIT;
                 break;

            default:                                       /* Invalid case, correct situation              */
                 step            = SYS_TRUE;
                 SysTickStepState = SYS_TICK_STEP_DIS;
                 break;
        }
        if (step == SYS_FALSE) {                            /* Return if waiting for step command           */
            return;
        }
#endif
        ptcb = SysTCBList;                                  /* Point at first TCB in TCB list               */
        while (ptcb->SysTCBPrio != SYS_TASK_IDLE_PRIO) {     /* Go through all TCBs in TCB list              */
            SYS_ENTER_CRITICAL();
            if (ptcb->SysTCBDly != 0) {                     /* No, Delayed or waiting for event with TO     */
                if (--ptcb->SysTCBDly == 0) {               /* Decrement nbr of ticks to end of delay       */
                                                           /* Check for timeout                            */
                    if ((ptcb->SysTCBStat & SYS_STAT_PEND_ANY) != SYS_STAT_RDY) {
                        ptcb->SysTCBStat  &= ~(INT8U)SYS_STAT_PEND_ANY;          /* Yes, Clear status flag   */
                        ptcb->SysTCBStatPend = SYS_STAT_PEND_TO;                 /* Indicate PEND timeout    */
                    } else {
                        ptcb->SysTCBStatPend = SYS_STAT_PEND_OK;
                    }

                    if ((ptcb->SysTCBStat & SYS_STAT_SUSPEND) == SYS_STAT_RDY) {  /* Is task suspended?       */
                        SysRdyGrp               |= ptcb->SysTCBBitY;             /* No,  Make ready          */
                        SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
                    }
                }
            }
            ptcb = ptcb->SysTCBNext;                        /* Point at next TCB in TCB list                */
            SYS_EXIT_CRITICAL();
        }
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             GET VERSION
*
* Description: This function is used to return the version number of uC/Sys-II.  The returned value
*              corresponds to uC/Sys-II's version number multiplied by 100.  In other words, version 2.00
*              would be returned as 200.
*
* Arguments  : none
*
* Returns    : the version number of uC/Sys-II multiplied by 100.
*********************************************************************************************************
*/

INT16U  SysVersion (void)
{
    return (SYS_VERSION);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                            DUMMY FUNCTION
*
* Description: This function doesn't do anything.  It is called by SysTaskDel().
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if SYS_TASK_DEL_EN > 0
void  SYS_Dummy (void)
{
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                             MAKE TASK READY TO RUN BASED ON EVENT OCCURING
*
* Description: This function is called by other uC/Sys-II services and is used to ready a task that was
*              waiting for an event to occur.
*
* Arguments  : pevent      is a pointer to the event control block corresponding to the event.
*
*              pmsg        is a pointer to a message.  This pointer is used by message oriented services
*                          such as MAILBOXEs and QUEUEs.  The pointer is not used when called by other
*                          service functions.
*
*              msk         is a mask that is used to clear the status byte of the TCB.  For example,
*                          SysSemPost() will pass SYS_STAT_SEM, SysMboxPost() will pass SYS_STAT_MBOX etc.
*
*              pend_stat   is used to indicate the readied task's pending status:
*
*                          SYS_STAT_PEND_OK      Task ready due to a post (or delete), not a timeout or
*                                               an abort.
*                          SYS_STAT_PEND_ABORT   Task ready due to an abort.
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if (SYS_EVENT_EN)
INT8U  SYS_EventTaskRdy (SYS_EVENT *pevent, void *pmsg, INT8U msk, INT8U pend_stat)
{
    SYS_TCB  *ptcb;
    INT8U    y;
    INT8U    x;
    INT8U    prio;
#if SYS_LOWEST_PRIO > 63
    INT16U  *ptbl;
#endif


#if SYS_LOWEST_PRIO <= 63
    y    = SysUnMapTbl[pevent->SysEventGrp];              /* Find HPT waiting for message                */
    x    = SysUnMapTbl[pevent->SysEventTbl[y]];
    prio = (INT8U)((y << 3) + x);                       /* Find priority of task getting the msg       */
#else
    if ((pevent->SysEventGrp & 0xFF) != 0) {             /* Find HPT waiting for message                */
        y = SysUnMapTbl[ pevent->SysEventGrp & 0xFF];
    } else {
        y = SysUnMapTbl[(pevent->SysEventGrp >> 8) & 0xFF] + 8;
    }
    ptbl = &pevent->SysEventTbl[y];
    if ((*ptbl & 0xFF) != 0) {
        x = SysUnMapTbl[*ptbl & 0xFF];
    } else {
        x = SysUnMapTbl[(*ptbl >> 8) & 0xFF] + 8;
    }
    prio = (INT8U)((y << 4) + x);                       /* Find priority of task getting the msg       */
#endif

    ptcb                  =  SysTCBPrioTbl[prio];        /* Point to this task's SYS_TCB                 */
    ptcb->SysTCBDly        =  0;                         /* Prevent SysTimeTick() from readying task     */
#if ((SYS_Q_EN > 0) && (SYS_MAX_QS > 0)) || (SYS_MBOX_EN > 0)
    ptcb->SysTCBMsg        =  pmsg;                      /* Send message directly to waiting task       */
#else
    pmsg                  =  pmsg;                      /* Prevent compiler warning if not used        */
#endif
    ptcb->SysTCBStat      &= ~msk;                       /* Clear bit associated with event type        */
    ptcb->SysTCBStatPend   =  pend_stat;                 /* Set pend status of post or abort            */
                                                        /* See if task is ready (could be susp'd)      */
    if ((ptcb->SysTCBStat &   SYS_STAT_SUSPEND) == SYS_STAT_RDY) {
        SysRdyGrp         |=  ptcb->SysTCBBitY;           /* Put task in the ready to run list           */
        SysRdyTbl[y]      |=  ptcb->SysTCBBitX;
    }

    SYS_EventTaskRemove(ptcb, pevent);                   /* Remove this task from event   wait list     */
#if (SYS_EVENT_MULTI_EN > 0)
    if (ptcb->SysTCBEventMultiPtr != (SYS_EVENT **)0) {   /* Remove this task from events' wait lists    */
        SYS_EventTaskRemoveMulti(ptcb, ptcb->SysTCBEventMultiPtr);
        ptcb->SysTCBEventPtr       = (SYS_EVENT  *)pevent;/* Return event as first multi-pend event ready*/
    }
#endif

    return (prio);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                   MAKE TASK WAIT FOR EVENT TO OCCUR
*
* Description: This function is called by other uC/Sys-II services to suspend a task because an event has
*              not occurred.
*
* Arguments  : pevent   is a pointer to the event control block for which the task will be waiting for.
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if (SYS_EVENT_EN)
void  SYS_EventTaskWait (SYS_EVENT *pevent)
{
    INT8U  y;


    SysTCBCur->SysTCBEventPtr               = pevent;                 /* Store ptr to ECB in TCB         */

    pevent->SysEventTbl[SysTCBCur->SysTCBY] |= SysTCBCur->SysTCBBitX;    /* Put task in waiting list        */
    pevent->SysEventGrp                   |= SysTCBCur->SysTCBBitY;

    y             =  SysTCBCur->SysTCBY;            /* Task no longer ready                              */
    SysRdyTbl[y]  &= ~SysTCBCur->SysTCBBitX;
    if (SysRdyTbl[y] == 0) {
        SysRdyGrp &= ~SysTCBCur->SysTCBBitY;         /* Clear event grp bit if this was only task pending */
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                          MAKE TASK WAIT FOR ANY OF MULTIPLE EVENTS TO OCCUR
*
* Description: This function is called by other uC/Sys-II services to suspend a task because any one of
*              multiple events has not occurred.
*
* Arguments  : pevents_wait     is a pointer to an array of event control blocks, NULL-terminated, for 
*                               which the task will be waiting for.
*
* Returns    : none.
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if ((SYS_EVENT_EN) && (SYS_EVENT_MULTI_EN > 0))
void  SYS_EventTaskWaitMulti (SYS_EVENT **pevents_wait)
{
    SYS_EVENT **pevents;
    SYS_EVENT  *pevent;
    INT8U      y;


    SysTCBCur->SysTCBEventPtr      = (SYS_EVENT  *)0;
    SysTCBCur->SysTCBEventMultiPtr = (SYS_EVENT **)pevents_wait;       /* Store ptr to ECBs in TCB        */

    pevents =  pevents_wait;
    pevent  = *pevents;
    while (pevent != (SYS_EVENT *)0) {                               /* Put task in waiting lists       */
        pevent->SysEventTbl[SysTCBCur->SysTCBY] |= SysTCBCur->SysTCBBitX;
        pevent->SysEventGrp                   |= SysTCBCur->SysTCBBitY;
        pevents++;
        pevent = *pevents;
    }

    y             =  SysTCBCur->SysTCBY;            /* Task no longer ready                              */
    SysRdyTbl[y]  &= ~SysTCBCur->SysTCBBitX;
    if (SysRdyTbl[y] == 0) {
        SysRdyGrp &= ~SysTCBCur->SysTCBBitY;         /* Clear event grp bit if this was only task pending */
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                   REMOVE TASK FROM EVENT WAIT LIST
*
* Description: Remove a task from an event's wait list.
*
* Arguments  : ptcb     is a pointer to the task to remove.
*
*              pevent   is a pointer to the event control block.
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if (SYS_EVENT_EN)
void  SYS_EventTaskRemove (SYS_TCB   *ptcb,
                          SYS_EVENT *pevent)
{
    INT8U  y;


    y                       =  ptcb->SysTCBY;
    pevent->SysEventTbl[y]  &= ~ptcb->SysTCBBitX;         /* Remove task from wait list                  */
    if (pevent->SysEventTbl[y] == 0) {
        pevent->SysEventGrp &= ~ptcb->SysTCBBitY;
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                             REMOVE TASK FROM MULTIPLE EVENTS WAIT LISTS
*
* Description: Remove a task from multiple events' wait lists.
*
* Arguments  : ptcb             is a pointer to the task to remove.
*
*              pevents_multi    is a pointer to the array of event control blocks, NULL-terminated.
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if ((SYS_EVENT_EN) && (SYS_EVENT_MULTI_EN > 0))
void  SYS_EventTaskRemoveMulti (SYS_TCB    *ptcb,
                               SYS_EVENT **pevents_multi)
{
    SYS_EVENT **pevents;
    SYS_EVENT  *pevent;
    INT8U      y;
#if (SYS_LOWEST_PRIO <= 63)
    INT8U      bity;
    INT8U      bitx;
#else
    INT16U     bity;
    INT16U     bitx;
#endif


    y       =  ptcb->SysTCBY;
    bity    =  ptcb->SysTCBBitY;
    bitx    =  ptcb->SysTCBBitX;
    pevents =  pevents_multi;
    pevent  = *pevents;
    while (pevent != (SYS_EVENT *)0) {                   /* Remove task from all events' wait lists     */
        pevent->SysEventTbl[y]  &= ~bitx;
        if (pevent->SysEventTbl[y] == 0) {
            pevent->SysEventGrp &= ~bity;
        }
        pevents++;
        pevent = *pevents;
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                 INITIALIZE EVENT CONTROL BLOCK'S WAIT LIST
*
* Description: This function is called by other uC/Sys-II services to initialize the event wait list.
*
* Arguments  : pevent    is a pointer to the event control block allocated to the event.
*
* Returns    : none
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/
#if (SYS_EVENT_EN)
void  SYS_EventWaitListInit (SYS_EVENT *pevent)
{
#if SYS_LOWEST_PRIO <= 63
    INT8U  *ptbl;
#else
    INT16U *ptbl;
#endif
    INT8U   i;


    pevent->SysEventGrp = 0;                      /* No task waiting on event                           */
    ptbl               = &pevent->SysEventTbl[0];

    for (i = 0; i < SYS_EVENT_TBL_SIZE; i++) {
        *ptbl++ = 0;
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                           INITIALIZE THE FREE LIST OF EVENT CONTROL BLOCKS
*
* Description: This function is called by SysInit() to initialize the free list of event control blocks.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SYS_InitEventList (void)
{
#if (SYS_EVENT_EN) && (SYS_MAX_EVENTS > 0)
#if (SYS_MAX_EVENTS > 1)
    INT16U     i;
    SYS_EVENT  *pevent1;
    SYS_EVENT  *pevent2;


    SYS_MemClr((INT8U *)&SysEventTbl[0], sizeof(SysEventTbl)); /* Clear the event table                   */
    pevent1 = &SysEventTbl[0];
    pevent2 = &SysEventTbl[1];
    for (i = 0; i < (SYS_MAX_EVENTS - 1); i++) {             /* Init. list of free EVENT control blocks */
        pevent1->SysEventType    = SYS_EVENT_TYPE_UNUSED;
        pevent1->SysEventPtr     = pevent2;
#if SYS_EVENT_NAME_SIZE > 1
        pevent1->SysEventName[0] = '?';                      /* Unknown name                            */
        pevent1->SysEventName[1] = SYS_ASCII_NUL;
#endif
        pevent1++;
        pevent2++;
    }
    pevent1->SysEventType            = SYS_EVENT_TYPE_UNUSED;
    pevent1->SysEventPtr             = (SYS_EVENT *)0;
#if SYS_EVENT_NAME_SIZE > 1
    pevent1->SysEventName[0]         = '?';
    pevent1->SysEventName[1]         = SYS_ASCII_NUL;
#endif
    SysEventFreeList                 = &SysEventTbl[0];
#else
    SysEventFreeList                 = &SysEventTbl[0];       /* Only have ONE event control block       */
    SysEventFreeList->SysEventType    = SYS_EVENT_TYPE_UNUSED;
    SysEventFreeList->SysEventPtr     = (SYS_EVENT *)0;
#if SYS_EVENT_NAME_SIZE > 1
    SysEventFreeList->SysEventName[0] = '?';                  /* Unknown name                            */
    SysEventFreeList->SysEventName[1] = SYS_ASCII_NUL;
#endif
#endif
#endif
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                    INITIALIZE MISCELLANEOUS VARIABLES
*
* Description: This function is called by SysInit() to initialize miscellaneous variables.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SYS_InitMisc (void)
{
#if SYS_TIME_GET_SET_EN > 0
    SysTime        = 0L;                                    /* Clear the 32-bit system clock            */
#endif

    SysIntNesting  = 0;                                     /* Clear the interrupt nesting counter      */
    SysLockNesting = 0;                                     /* Clear the scheduling lock counter        */

    SysTaskCtr     = 0;                                     /* Clear the number of tasks                */

    SysRunning     = SYS_FALSE;                              /* Indicate that multitasking not started   */

    SysCtxSwCtr    = 0;                                     /* Clear the context switch counter         */
    SysIdleCtr     = 0L;                                    /* Clear the 32-bit idle counter            */

#if SYS_TASK_STAT_EN > 0
    SysIdleCtrRun  = 0L;
    SysIdleCtrMax  = 0L;
    SysStatRdy     = SYS_FALSE;                              /* Statistic task is not ready              */
#endif
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                       INITIALIZE THE READY LIST
*
* Description: This function is called by SysInit() to initialize the Ready List.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SYS_InitRdyList (void)
{
    INT8U    i;
#if SYS_LOWEST_PRIO <= 63
    INT8U   *prdytbl;
#else
    INT16U  *prdytbl;
#endif


    SysRdyGrp      = 0;                                     /* Clear the ready list                     */
    prdytbl       = &SysRdyTbl[0];
    for (i = 0; i < SYS_RDY_TBL_SIZE; i++) {
        *prdytbl++ = 0;
    }

    SysPrioCur     = 0;
    SysPrioHighRdy = 0;

    SysTCBHighRdy  = (SYS_TCB *)0;
    SysTCBCur      = (SYS_TCB *)0;
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                         CREATING THE IDLE TASK
*
* Description: This function creates the Idle Task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SYS_InitTaskIdle (void)
{
#if SYS_TASK_NAME_SIZE > 7
    INT8U  err;
#endif


#if SYS_TASK_CREATE_EXT_EN > 0
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreateExt(SYS_TaskIdle,
                          (void *)0,                                 /* No arguments passed to SYS_TaskIdle() */
                          &SysTaskIdleStk[SYS_TASK_IDLE_STK_SIZE - 1], /* Set Top-Of-Stack                     */
                          SYS_TASK_IDLE_PRIO,                         /* Lowest priority level                */
                          SYS_TASK_IDLE_ID,
                          &SysTaskIdleStk[0],                         /* Set Bottom-Of-Stack                  */
                          SYS_TASK_IDLE_STK_SIZE,
                          (void *)0,                                 /* No TCB extension                     */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);/* Enable stack checking + clear stack  */
    #else
    (void)SysTaskCreateExt(SYS_TaskIdle,
                          (void *)0,                                 /* No arguments passed to SYS_TaskIdle() */
                          &SysTaskIdleStk[0],                         /* Set Top-Of-Stack                     */
                          SYS_TASK_IDLE_PRIO,                         /* Lowest priority level                */
                          SYS_TASK_IDLE_ID,
                          &SysTaskIdleStk[SYS_TASK_IDLE_STK_SIZE - 1], /* Set Bottom-Of-Stack                  */
                          SYS_TASK_IDLE_STK_SIZE,
                          (void *)0,                                 /* No TCB extension                     */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);/* Enable stack checking + clear stack  */
    #endif
#else
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreate(SYS_TaskIdle,
                       (void *)0,
                       &SysTaskIdleStk[SYS_TASK_IDLE_STK_SIZE - 1],
                       SYS_TASK_IDLE_PRIO);
    #else
    (void)SysTaskCreate(SYS_TaskIdle,
                       (void *)0,
                       &SysTaskIdleStk[0],
                       SYS_TASK_IDLE_PRIO);
    #endif
#endif

#if SYS_TASK_NAME_SIZE > 14
    SysTaskNameSet(SYS_TASK_IDLE_PRIO, (INT8U *)"uC/Sys-II Idle", &err);
#else
#if SYS_TASK_NAME_SIZE > 7
    SysTaskNameSet(SYS_TASK_IDLE_PRIO, (INT8U *)"Sys-Idle", &err);
#endif
#endif
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                                      CREATING THE STATISTIC TASK
*
* Description: This function creates the Statistic Task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if SYS_TASK_STAT_EN > 0
static  void  SYS_InitTaskStat (void)
{
#if SYS_TASK_NAME_SIZE > 7
    INT8U  err;
#endif


#if SYS_TASK_CREATE_EXT_EN > 0
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreateExt(SYS_TaskStat,
                          (void *)0,                                   /* No args passed to SYS_TaskStat()*/
                          &SysTaskStatStk[SYS_TASK_STAT_STK_SIZE - 1],   /* Set Top-Of-Stack               */
                          SYS_TASK_STAT_PRIO,                           /* One higher than the idle task  */
                          SYS_TASK_STAT_ID,
                          &SysTaskStatStk[0],                           /* Set Bottom-Of-Stack            */
                          SYS_TASK_STAT_STK_SIZE,
                          (void *)0,                                   /* No TCB extension               */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear  */
    #else
    (void)SysTaskCreateExt(SYS_TaskStat,
                          (void *)0,                                   /* No args passed to SYS_TaskStat()*/
                          &SysTaskStatStk[0],                           /* Set Top-Of-Stack               */
                          SYS_TASK_STAT_PRIO,                           /* One higher than the idle task  */
                          SYS_TASK_STAT_ID,
                          &SysTaskStatStk[SYS_TASK_STAT_STK_SIZE - 1],   /* Set Bottom-Of-Stack            */
                          SYS_TASK_STAT_STK_SIZE,
                          (void *)0,                                   /* No TCB extension               */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);  /* Enable stack checking + clear  */
    #endif
#else
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreate(SYS_TaskStat,
                       (void *)0,                                      /* No args passed to SYS_TaskStat()*/
                       &SysTaskStatStk[SYS_TASK_STAT_STK_SIZE - 1],      /* Set Top-Of-Stack               */
                       SYS_TASK_STAT_PRIO);                             /* One higher than the idle task  */
    #else
    (void)SysTaskCreate(SYS_TaskStat,
                       (void *)0,                                      /* No args passed to SYS_TaskStat()*/
                       &SysTaskStatStk[0],                              /* Set Top-Of-Stack               */
                       SYS_TASK_STAT_PRIO);                             /* One higher than the idle task  */
    #endif
#endif

#if SYS_TASK_NAME_SIZE > 14
    SysTaskNameSet(SYS_TASK_STAT_PRIO, (INT8U *)"uC/Sys-II Stat", &err);
#else
#if SYS_TASK_NAME_SIZE > 7
    SysTaskNameSet(SYS_TASK_STAT_PRIO, (INT8U *)"Sys-Stat", &err);
#endif
#endif
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                             INITIALIZATION
*                            INITIALIZE THE FREE LIST OF TASK CONTROL BLOCKS
*
* Description: This function is called by SysInit() to initialize the free list of SYS_TCBs.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

static  void  SYS_InitTCBList (void)
{
    INT8U    i;
    SYS_TCB  *ptcb1;
    SYS_TCB  *ptcb2;


    SYS_MemClr((INT8U *)&SysTCBTbl[0],     sizeof(SysTCBTbl));      /* Clear all the TCBs                 */
    SYS_MemClr((INT8U *)&SysTCBPrioTbl[0], sizeof(SysTCBPrioTbl));  /* Clear the priority table           */
    ptcb1 = &SysTCBTbl[0];
    ptcb2 = &SysTCBTbl[1];
    for (i = 0; i < (SYS_MAX_TASKS + SYS_N_SYS_TASKS - 1); i++) {  /* Init. list of free TCBs            */
        ptcb1->SysTCBNext = ptcb2;
#if SYS_TASK_NAME_SIZE > 1
        ptcb1->SysTCBTaskName[0] = '?';                           /* Unknown name                       */
        ptcb1->SysTCBTaskName[1] = SYS_ASCII_NUL;
#endif
        ptcb1++;
        ptcb2++;
    }
    ptcb1->SysTCBNext = (SYS_TCB *)0;                              /* Last SYS_TCB                        */
#if SYS_TASK_NAME_SIZE > 1
    ptcb1->SysTCBTaskName[0] = '?';                               /* Unknown name                       */
    ptcb1->SysTCBTaskName[1] = SYS_ASCII_NUL;
#endif
    SysTCBList               = (SYS_TCB *)0;                       /* TCB lists initializations          */
    SysTCBFreeList           = &SysTCBTbl[0];
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        CLEAR A SECTION OF MEMORY
*
* Description: This function is called by other uC/Sys-II services to clear a contiguous block of RAM.
*
* Arguments  : pdest    is the start of the RAM to clear (i.e. write 0x00 to)
*
*              size     is the number of bytes to clear.
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.
*              2) Note that we can only clear up to 64K bytes of RAM.  This is not an issue because none
*                 of the uses of this function gets close to this limit.
*              3) The clear is done one byte at a time since this will work on any processor irrespective
*                 of the alignment of the destination.
*********************************************************************************************************
*/

void  SYS_MemClr (INT8U *pdest, INT16U size)
{
    while (size > 0) {
        *pdest++ = (INT8U)0;
        size--;
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                        COPY A BLOCK OF MEMORY
*
* Description: This function is called by other uC/Sys-II services to copy a block of memory from one
*              location to another.
*
* Arguments  : pdest    is a pointer to the 'destination' memory block
*
*              psrc     is a pointer to the 'source'      memory block
*
*              size     is the number of bytes to copy.
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.  There is
*                 no provision to handle overlapping memory copy.  However, that's not a problem since this
*                 is not a situation that will happen.
*              2) Note that we can only copy up to 64K bytes of RAM
*              3) The copy is done one byte at a time since this will work on any processor irrespective
*                 of the alignment of the source and destination.
*********************************************************************************************************
*/

void  SYS_MemCopy (INT8U *pdest, INT8U *psrc, INT16U size)
{
    while (size > 0) {
        *pdest++ = *psrc++;
        size--;
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                              SCHEDULER
*
* Description: This function is called by other uC/Sys-II services to determine whether a new, high
*              priority task has been made ready to run.  This function is invoked by TASK level code
*              and is not used to reschedule tasks from ISRs (see SysIntExit() for ISR rescheduling).
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.
*              2) Rescheduling is prevented when the scheduler is locked (see SYS_SchedLock())
*********************************************************************************************************
*/

void  SYS_Sched (void)
{
#if SYS_CRITICAL_METHOD == 3                            /* Allocate storage for CPU status register     */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    SYS_ENTER_CRITICAL();
    if (SysIntNesting == 0) {                           /* Schedule only if all ISRs done and ...       */
        if (SysLockNesting == 0) {                      /* ... scheduler is not locked                  */
            SYS_SchedNew();
//add_by_StreakingMCU
				SysTCBHighRdy = SysTCBPrioTbl[SysPrioHighRdy];
//end_add_by_StreakingMCU				
                //if (SysPrioHighRdy != SysPrioCur) {          /* No Ctx Sw if current task is highest rdy */
				if (SysTCBHighRdy != SysTCBCur) {  
//delete_by_StreakingMCU
//                    SysTCBHighRdy  = SysTCBPrioTbl[SysPrioHighRdy];
//end_delete_by_StreakingMCU					
#if SYS_TASK_PROFILE_EN > 0
                SysTCBHighRdy->SysTCBCtxSwCtr++;         /* Inc. # of context switches to this task      */
#endif
                SysCtxSwCtr++;                          /* Increment context switch counter             */
                SYS_TASK_SW();                          /* Perform a context switch                     */
            }
        }
    }
    SYS_EXIT_CRITICAL();
}


/*
*********************************************************************************************************
*                              FIND HIGHEST PRIORITY TASK READY TO RUN
*
* Description: This function is called by other uC/Sys-II services to determine the highest priority task
*              that is ready to run.  The global variable 'SysPrioHighRdy' is changed accordingly.
*
* Arguments  : none
*
* Returns    : none
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.
*              2) Interrupts are assumed to be disabled when this function is called.
*********************************************************************************************************
*/

static  void  SYS_SchedNew (void)
{
#if SYS_LOWEST_PRIO <= 63                         /* See if we support up to 64 tasks                   */
    INT8U   y;


    y             = SysUnMapTbl[SysRdyGrp];
    SysPrioHighRdy = (INT8U)((y << 3) + SysUnMapTbl[SysRdyTbl[y]]);
#else                                            /* We support up to 256 tasks                         */
    INT8U   y;
    INT16U *ptbl;


    if ((SysRdyGrp & 0xFF) != 0) {
        y = SysUnMapTbl[SysRdyGrp & 0xFF];
    } else {
        y = SysUnMapTbl[(SysRdyGrp >> 8) & 0xFF] + 8;
    }
    ptbl = &SysRdyTbl[y];
    if ((*ptbl & 0xFF) != 0) {
        SysPrioHighRdy = (INT8U)((y << 4) + SysUnMapTbl[(*ptbl & 0xFF)]);
    } else {
        SysPrioHighRdy = (INT8U)((y << 4) + SysUnMapTbl[(*ptbl >> 8) & 0xFF] + 8);
    }
#endif
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                        COPY AN ASCII STRING
*
* Description: This function is called by other uC/Sys-II services to copy an ASCII string from a 'source'
*              string to a 'destination' string.
*
* Arguments  : pdest    is a pointer to the string that will be receiving the copy.  Note that there MUST
*                       be sufficient space in the destination storage area to receive this string.
*
*              psrc     is a pointer to the source string.  The source string MUST NOT be greater than
*                       254 characters.
*
* Returns    : The size of the string (excluding the NUL terminating character)
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

#if (SYS_EVENT_NAME_SIZE > 1) || (SYS_FLAG_NAME_SIZE > 1) || (SYS_MEM_NAME_SIZE > 1) || (SYS_TASK_NAME_SIZE > 1) || (SYS_TMR_CFG_NAME_SIZE > 1)
INT8U  SYS_StrCopy (INT8U *pdest, INT8U *psrc)
{
    INT8U  len;


    len = 0;
    while (*psrc != SYS_ASCII_NUL) {
        *pdest++ = *psrc++;
        len++;
    }
    *pdest = SYS_ASCII_NUL;
    return (len);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                DETERMINE THE LENGTH OF AN ASCII STRING
*
* Description: This function is called by other uC/Sys-II services to determine the size of an ASCII string
*              (excluding the NUL character).
*
* Arguments  : psrc     is a pointer to the string for which we need to know the size.
*
* Returns    : The size of the string (excluding the NUL terminating character)
*
* Notes      : 1) This function is INTERNAL to uC/Sys-II and your application should not call it.
*              2) The string to check must be less than 255 characters long.
*********************************************************************************************************
*/

#if (SYS_EVENT_NAME_SIZE > 1) || (SYS_FLAG_NAME_SIZE > 1) || (SYS_MEM_NAME_SIZE > 1) || (SYS_TASK_NAME_SIZE > 1) || (SYS_TMR_CFG_NAME_SIZE > 1)
INT8U  SYS_StrLen (INT8U *psrc)
{
    INT8U  len;


    len = 0;
    while (*psrc != SYS_ASCII_NUL) {
        psrc++;
        len++;
    }
    return (len);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                              IDLE TASK
*
* Description: This task is internal to uC/Sys-II and executes whenever no other higher priority tasks
*              executes because they are ALL waiting for event(s) to occur.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : 1) SysTaskIdleHook() is called after the critical section to ensure that interrupts will be
*                 enabled for at least a few instructions.  On some processors (ex. Philips XA), enabling
*                 and then disabling interrupts didn't allow the processor enough time to have interrupts
*                 enabled before they were disabled again.  uC/Sys-II would thus never recognize
*                 interrupts.
*              2) This hook has been added to allow you to do such things as STOP the CPU to conserve
*                 power.
*********************************************************************************************************
*/

void  SYS_TaskIdle (void *p_arg)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    (void)p_arg;                                 /* Prevent compiler warning for not using 'p_arg'     */
    for (;;) {
        SYS_ENTER_CRITICAL();
        SysIdleCtr++;
        SYS_EXIT_CRITICAL();
        SysTaskIdleHook();                        /* Call user definable HOOK                           */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                            STATISTICS TASK
*
* Description: This task is internal to uC/Sys-II and is used to compute some statistics about the
*              multitasking environment.  Specifically, SYS_TaskStat() computes the CPU usage.
*              CPU usage is determined by:
*
*                                          SysIdleCtr
*                 SysCPUUsage = 100 * (1 - ------------)     (units are in %)
*                                         SysIdleCtrMax
*
* Arguments  : parg     this pointer is not used at this time.
*
* Returns    : none
*
* Notes      : 1) This task runs at a priority level higher than the idle task.  In fact, it runs at the
*                 next higher priority, SYS_TASK_IDLE_PRIO-1.
*              2) You can disable this task by setting the configuration #define SYS_TASK_STAT_EN to 0.
*              3) You MUST have at least a delay of 2/10 seconds to allow for the system to establish the
*                 maximum value for the idle counter.
*********************************************************************************************************
*/

#if SYS_TASK_STAT_EN > 0
void  SYS_TaskStat (void *p_arg)
{
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    (void)p_arg;                                 /* Prevent compiler warning for not using 'p_arg'     */
    while (SysStatRdy == SYS_FALSE) {
        SysTimeDly(2 * SYS_TICKS_PER_SEC / 10);    /* Wait until statistic task is ready                 */
    }
    SysIdleCtrMax /= 100L;
    if (SysIdleCtrMax == 0L) {
        SysCPUUsage = 0;
        (void)SysTaskSuspend(SYS_PRIO_SELF);
    }
    for (;;) {
        SYS_ENTER_CRITICAL();
        SysIdleCtrRun = SysIdleCtr;                /* Obtain the of the idle counter for the past second */
        SysIdleCtr    = 0L;                       /* Reset the idle counter for the next second         */
        SYS_EXIT_CRITICAL();
        SysCPUUsage   = (INT8U)(100L - SysIdleCtrRun / SysIdleCtrMax);
        SysTaskStatHook();                        /* Invoke user definable hook                         */
#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
        SYS_TaskStatStkChk();                     /* Check the stacks for each task                     */
#endif
        SysTimeDly(SYS_TICKS_PER_SEC / 10);        /* Accumulate SysIdleCtr for the next 1/10 second      */
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                      CHECK ALL TASK STACKS
*
* Description: This function is called by SYS_TaskStat() to check the stacks of each active task.
*
* Arguments  : none
*
* Returns    : none
*********************************************************************************************************
*/

#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
void  SYS_TaskStatStkChk (void)
{
    SYS_TCB      *ptcb;
    SYS_STK_DATA  stk_data;
    INT8U        err;
    INT8U        prio;


    for (prio = 0; prio <= SYS_TASK_IDLE_PRIO; prio++) {
        err = SysTaskStkChk(prio, &stk_data);
        if (err == SYS_ERR_NONE) {
            ptcb = SysTCBPrioTbl[prio];
            if (ptcb != (SYS_TCB *)0) {                               /* Make sure task 'ptcb' is ...   */
                if (ptcb != SYS_TCB_RESERVED) {                       /* ... still valid.               */
#if SYS_TASK_PROFILE_EN > 0
                    #if SYS_STK_GROWTH == 1
                    ptcb->SysTCBStkBase = ptcb->SysTCBStkBottom + ptcb->SysTCBStkSize;
                    #else
                    ptcb->SysTCBStkBase = ptcb->SysTCBStkBottom - ptcb->SysTCBStkSize;
                    #endif
                    ptcb->SysTCBStkUsed = stk_data.SysUsed;            /* Store the number of bytes used */
#endif
                }
            }
        }
    }
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            INITIALIZE TCB
*
* Description: This function is internal to uC/Sys-II and is used to initialize a Task Control Block when
*              a task is created (see SysTaskCreate() and SysTaskCreateExt()).
*
* Arguments  : prio          is the priority of the task being created
*
*              ptos          is a pointer to the task's top-of-stack assuming that the CPU registers
*                            have been placed on the stack.  Note that the top-of-stack corresponds to a
*                            'high' memory location is SYS_STK_GROWTH is set to 1 and a 'low' memory
*                            location if SYS_STK_GROWTH is set to 0.  Note that stack growth is CPU
*                            specific.
*
*              pbos          is a pointer to the bottom of stack.  A NULL pointer is passed if called by
*                            'SysTaskCreate()'.
*
*              id            is the task's ID (0..65535)
*
*              stk_size      is the size of the stack (in 'stack units').  If the stack units are INT8Us
*                            then, 'stk_size' contains the number of bytes for the stack.  If the stack
*                            units are INT32Us then, the stack contains '4 * stk_size' bytes.  The stack
*                            units are established by the #define constant SYS_STK which is CPU
*                            specific.  'stk_size' is 0 if called by 'SysTaskCreate()'.
*
*              pext          is a pointer to a user supplied memory area that is used to extend the task
*                            control block.  This allows you to store the contents of floating-point
*                            registers, MMU registers or anything else you could find useful during a
*                            context switch.  You can even assign a name to each task and store this name
*                            in this TCB extension.  A NULL pointer is passed if called by SysTaskCreate().
*
*              opt           options as passed to 'SysTaskCreateExt()' or,
*                            0 if called from 'SysTaskCreate()'.
*
* Returns    : SYS_ERR_NONE         if the call was successful
*              SYS_ERR_TASK_NO_MORE_TCB  if there are no more free TCBs to be allocated and thus, the task cannot
*                                  be created.
*
* Note       : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

INT8U  SYS_TCBInit (INT8U prio, SYS_STK *ptos, SYS_STK *pbos, INT16U id, INT32U stk_size, void *pext, INT16U opt)
{
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    SYS_ENTER_CRITICAL();
    ptcb = SysTCBFreeList;                                  /* Get a free TCB from the free TCB list    */
    if (ptcb != (SYS_TCB *)0) {
        SysTCBFreeList            = ptcb->SysTCBNext;        /* Update pointer to free TCB list          */
        SYS_EXIT_CRITICAL();
        ptcb->SysTCBStkPtr        = ptos;                   /* Load Stack pointer in TCB                */
        ptcb->SysTCBPrio          = prio;                   /* Load task priority into TCB              */
        ptcb->SysTCBStat          = SYS_STAT_RDY;            /* Task is ready to run                     */
        ptcb->SysTCBStatPend      = SYS_STAT_PEND_OK;        /* Clear pend status                        */
        ptcb->SysTCBDly           = 0;                      /* Task is not delayed                      */

#if SYS_TASK_CREATE_EXT_EN > 0
        ptcb->SysTCBExtPtr        = pext;                   /* Store pointer to TCB extension           */
        ptcb->SysTCBStkSize       = stk_size;               /* Store stack size                         */
        ptcb->SysTCBStkBottom     = pbos;                   /* Store pointer to bottom of stack         */
        ptcb->SysTCBOpt           = opt;                    /* Store task options                       */
        ptcb->SysTCBId            = id;                     /* Store task ID                            */
#else
        pext                     = pext;                   /* Prevent compiler warning if not used     */
        stk_size                 = stk_size;
        pbos                     = pbos;
        opt                      = opt;
        id                       = id;
#endif

#if SYS_TASK_DEL_EN > 0
        ptcb->SysTCBDelReq        = SYS_ERR_NONE;
#endif

#if SYS_LOWEST_PRIO <= 63
        ptcb->SysTCBY             = (INT8U)(prio >> 3);          /* Pre-compute X, Y, BitX and BitY     */
        ptcb->SysTCBX             = (INT8U)(prio & 0x07);
        ptcb->SysTCBBitY          = (INT8U)(1 << ptcb->SysTCBY);
        ptcb->SysTCBBitX          = (INT8U)(1 << ptcb->SysTCBX);
#else
        ptcb->SysTCBY             = (INT8U)((prio >> 4) & 0xFF); /* Pre-compute X, Y, BitX and BitY     */
        ptcb->SysTCBX             = (INT8U) (prio & 0x0F);
        ptcb->SysTCBBitY          = (INT16U)(1 << ptcb->SysTCBY);
        ptcb->SysTCBBitX          = (INT16U)(1 << ptcb->SysTCBX);
#endif

#if (SYS_EVENT_EN)
        ptcb->SysTCBEventPtr      = (SYS_EVENT  *)0;         /* Task is not pending on an  event         */
#if (SYS_EVENT_MULTI_EN > 0)
        ptcb->SysTCBEventMultiPtr = (SYS_EVENT **)0;         /* Task is not pending on any events        */
#endif
#endif

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0) && (SYS_TASK_DEL_EN > 0)
        ptcb->SysTCBFlagNode  = (SYS_FLAG_NODE *)0;          /* Task is not pending on an event flag     */
#endif

#if (SYS_MBOX_EN > 0) || ((SYS_Q_EN > 0) && (SYS_MAX_QS > 0))
        ptcb->SysTCBMsg       = (void *)0;                  /* No message received                      */
#endif

#if SYS_TASK_PROFILE_EN > 0
        ptcb->SysTCBCtxSwCtr    = 0L;                       /* Initialize profiling variables           */
        ptcb->SysTCBCyclesStart = 0L;
        ptcb->SysTCBCyclesTot   = 0L;
        ptcb->SysTCBStkBase     = (SYS_STK *)0;
        ptcb->SysTCBStkUsed     = 0L;
#endif

#if SYS_TASK_NAME_SIZE > 1
        ptcb->SysTCBTaskName[0] = '?';                      /* Unknown name at task creation            */
        ptcb->SysTCBTaskName[1] = SYS_ASCII_NUL;
#endif

        SysTCBInitHook(ptcb);

        SysTaskCreateHook(ptcb);                            /* Call user defined hook                   */

        SYS_ENTER_CRITICAL();
        SysTCBPrioTbl[prio] = ptcb;
        ptcb->SysTCBNext    = SysTCBList;                    /* Link into TCB chain                      */
        ptcb->SysTCBPrev    = (SYS_TCB *)0;
        if (SysTCBList != (SYS_TCB *)0) {
            SysTCBList->SysTCBPrev = ptcb;
        }
        SysTCBList               = ptcb;
        SysRdyGrp               |= ptcb->SysTCBBitY;         /* Make task ready to run                   */
        SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
        SysTaskCtr++;                                       /* Increment the #tasks counter             */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_NONE);
    }
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_TASK_NO_MORE_TCB);
}
