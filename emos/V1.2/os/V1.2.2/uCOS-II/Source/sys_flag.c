/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                         EVENT FLAG  MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_FLAG.C
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

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
/*
*********************************************************************************************************
*                                            LOCAL PROTOTYPES
*********************************************************************************************************
*/

static  void     SYS_FlagBlock(SYS_FLAG_GRP *pgrp, SYS_FLAG_NODE *pnode, SYS_FLAGS flags, INT8U wait_type, INT16U timeout);
static  BOOLEAN  SYS_FlagTaskRdy(SYS_FLAG_NODE *pnode, SYS_FLAGS flags_rdy);

/*$PAGE*/
/*
*********************************************************************************************************
*                              CHECK THE STATUS OF FLAGS IN AN EVENT FLAG GROUP
*
* Description: This function is called to check the status of a combination of bits to be set or cleared
*              in an event flag group.  Your application can check for ANY bit to be set/cleared or ALL
*              bits to be set/cleared.
*
*              This call does not block if the desired flags are not present.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         Is a bit pattern indicating which bit(s) (i.e. flags) you wish to check.
*                            The bits you want are specified by setting the corresponding bits in
*                            'flags'.  e.g. if your application wants to wait for bits 0 and 1 then
*                            'flags' would contain 0x03.
*
*              wait_type     specifies whether you want ALL bits to be set/cleared or ANY of the bits
*                            to be set/cleared.
*                            You can specify the following argument:
*
*                            SYS_FLAG_WAIT_CLR_ALL   You will check ALL bits in 'flags' to be clear (0)
*                            SYS_FLAG_WAIT_CLR_ANY   You will check ANY bit  in 'flags' to be clear (0)
*                            SYS_FLAG_WAIT_SET_ALL   You will check ALL bits in 'flags' to be set   (1)
*                            SYS_FLAG_WAIT_SET_ANY   You will check ANY bit  in 'flags' to be set   (1)
*
*                            NOTE: Add SYS_FLAG_CONSUME if you want the event flag to be 'consumed' by
*                                  the call.  Example, to wait for any flag in a group AND then clear
*                                  the flags that are present, set 'wait_type' to:
*
*                                  SYS_FLAG_WAIT_SET_ANY + SYS_FLAG_CONSUME
*
*              perr          is a pointer to an error code and can be:
*                            SYS_ERR_NONE               No error
*                            SYS_ERR_EVENT_TYPE         You are not pointing to an event flag group
*                            SYS_ERR_FLAG_WAIT_TYPE     You didn't specify a proper 'wait_type' argument.
*                            SYS_ERR_FLAG_INVALID_PGRP  You passed a NULL pointer instead of the event flag
*                                                      group handle.
*                            SYS_ERR_FLAG_NOT_RDY       The desired flags you are waiting for are not
*                                                      available.
*
* Returns    : The flags in the event flag group that made the task ready or, 0 if a timeout or an error
*              occurred.
*
* Called from: Task or ISR
*
* Note(s)    : 1) IMPORTANT, the behavior of this function has changed from PREVIOUS versions.  The
*                 function NOW returns the flags that were ready INSTEAD of the current state of the
*                 event flags.
*********************************************************************************************************
*/

#if SYS_FLAG_ACCEPT_EN > 0
SYS_FLAGS  SysFlagAccept (SYS_FLAG_GRP *pgrp, SYS_FLAGS flags, INT8U wait_type, INT8U *perr)
{
    SYS_FLAGS      flags_rdy;
    INT8U         result;
    BOOLEAN       consume;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR     cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return ((SYS_FLAGS)0);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return ((SYS_FLAGS)0);
    }
#endif
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {          /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return ((SYS_FLAGS)0);
    }
    result = (INT8U)(wait_type & SYS_FLAG_CONSUME);
    if (result != (INT8U)0) {                              /* See if we need to consume the flags      */
        wait_type &= ~SYS_FLAG_CONSUME;
        consume    = SYS_TRUE;
    } else {
        consume    = SYS_FALSE;
    }
/*$PAGE*/
    *perr = SYS_ERR_NONE;                                   /* Assume NO error until proven otherwise.  */
    SYS_ENTER_CRITICAL();
    switch (wait_type) {
        case SYS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
             flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & flags);     /* Extract only the bits we want   */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we wanted      */
                 }
             } else {
                 *perr = SYS_ERR_FLAG_NOT_RDY;
             }
             SYS_EXIT_CRITICAL();
             break;

        case SYS_FLAG_WAIT_SET_ANY:
             flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & flags);     /* Extract only the bits we want   */
             if (flags_rdy != (SYS_FLAGS)0) {               /* See if any flag set                      */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we got         */
                 }
             } else {
                 *perr = SYS_ERR_FLAG_NOT_RDY;
             }
             SYS_EXIT_CRITICAL();
             break;

#if SYS_FLAG_WAIT_CLR_EN > 0
        case SYS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
             flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & flags);  /* Extract only the bits we want     */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags |= flags_rdy;       /* Set ONLY the flags that we wanted        */
                 }
             } else {
                 *perr = SYS_ERR_FLAG_NOT_RDY;
             }
             SYS_EXIT_CRITICAL();
             break;

        case SYS_FLAG_WAIT_CLR_ANY:
             flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & flags); /* Extract only the bits we want      */
             if (flags_rdy != (SYS_FLAGS)0) {               /* See if any flag cleared                  */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags |= flags_rdy;       /* Set ONLY the flags that we got           */
                 }
             } else {
                 *perr = SYS_ERR_FLAG_NOT_RDY;
             }
             SYS_EXIT_CRITICAL();
             break;
#endif

        default:
             SYS_EXIT_CRITICAL();
             flags_rdy = (SYS_FLAGS)0;
             *perr     = SYS_ERR_FLAG_WAIT_TYPE;
             break;
    }
    return (flags_rdy);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                           CREATE AN EVENT FLAG
*
* Description: This function is called to create an event flag group.
*
* Arguments  : flags         Contains the initial value to store in the event flag group.
*
*              perr          is a pointer to an error code which will be returned to your application:
*                               SYS_ERR_NONE               if the call was successful.
*                               SYS_ERR_CREATE_ISR         if you attempted to create an Event Flag from an
*                                                         ISR.
*                               SYS_ERR_FLAG_GRP_DEPLETED  if there are no more event flag groups
*
* Returns    : A pointer to an event flag group or a NULL pointer if no more groups are available.
*
* Called from: Task ONLY
*********************************************************************************************************
*/

SYS_FLAG_GRP  *SysFlagCreate (SYS_FLAGS flags, INT8U *perr)
{
    SYS_FLAG_GRP *pgrp;
#if SYS_CRITICAL_METHOD == 3                         /* Allocate storage for CPU status register        */
    SYS_CPU_SR    cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                       /* Validate 'perr'                                 */
        return ((SYS_FLAG_GRP *)0);
    }
#endif
    if (SysIntNesting > 0) {                         /* See if called from ISR ...                      */
        *perr = SYS_ERR_CREATE_ISR;                  /* ... can't CREATE from an ISR                    */
        return ((SYS_FLAG_GRP *)0);
    }
    SYS_ENTER_CRITICAL();
    pgrp = SysFlagFreeList;                          /* Get next free event flag                        */
    if (pgrp != (SYS_FLAG_GRP *)0) {                 /* See if we have event flag groups available      */
                                                    /* Adjust free list                                */
        SysFlagFreeList       = (SYS_FLAG_GRP *)SysFlagFreeList->SysFlagWaitList;
        pgrp->SysFlagType     = SYS_EVENT_TYPE_FLAG;  /* Set to event flag group type                    */
        pgrp->SysFlagFlags    = flags;               /* Set to desired initial value                    */
        pgrp->SysFlagWaitList = (void *)0;           /* Clear list of tasks waiting on flags            */
#if SYS_FLAG_NAME_SIZE > 1
        pgrp->SysFlagName[0]  = '?';
        pgrp->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
        SYS_EXIT_CRITICAL();
        *perr                = SYS_ERR_NONE;
    } else {
        SYS_EXIT_CRITICAL();
        *perr                = SYS_ERR_FLAG_GRP_DEPLETED;
    }
    return (pgrp);                                  /* Return pointer to event flag group              */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                     DELETE AN EVENT FLAG GROUP
*
* Description: This function deletes an event flag group and readies all tasks pending on the event flag
*              group.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              opt           determines delete options as follows:
*                            opt == SYS_DEL_NO_PEND   Deletes the event flag group ONLY if no task pending
*                            opt == SYS_DEL_ALWAYS    Deletes the event flag group even if tasks are
*                                                    waiting.  In this case, all the tasks pending will be
*                                                    readied.
*
*              perr          is a pointer to an error code that can contain one of the following values:
*                            SYS_ERR_NONE               The call was successful and the event flag group was
*                                                      deleted
*                            SYS_ERR_DEL_ISR            If you attempted to delete the event flag group from
*                                                      an ISR
*                            SYS_ERR_FLAG_INVALID_PGRP  If 'pgrp' is a NULL pointer.
*                            SYS_ERR_EVENT_TYPE         If you didn't pass a pointer to an event flag group
*                            SYS_ERR_INVALID_OPT        An invalid option was specified
*                            SYS_ERR_TASK_WAITING       One or more tasks were waiting on the event flag
*                                                      group.
*
* Returns    : pgrp          upon error
*              (SYS_EVENT *)0 if the event flag group was successfully deleted.
*
* Note(s)    : 1) This function must be used with care.  Tasks that would normally expect the presence of
*                 the event flag group MUST check the return code of SysFlagAccept() and SysFlagPend().
*              2) This call can potentially disable interrupts for a long time.  The interrupt disable
*                 time is directly proportional to the number of tasks waiting on the event flag group.
*********************************************************************************************************
*/

#if SYS_FLAG_DEL_EN > 0
SYS_FLAG_GRP  *SysFlagDel (SYS_FLAG_GRP *pgrp, INT8U opt, INT8U *perr)
{
    BOOLEAN       tasks_waiting;
    SYS_FLAG_NODE *pnode;
    SYS_FLAG_GRP  *pgrp_return;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR     cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return (pgrp);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return (pgrp);
    }
#endif
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_DEL_ISR;                            /* ... can't DELETE from an ISR             */
        return (pgrp);
    }
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {          /* Validate event group type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return (pgrp);
    }
    SYS_ENTER_CRITICAL();
    if (pgrp->SysFlagWaitList != (void *)0) {               /* See if any tasks waiting on event flags  */
        tasks_waiting = SYS_TRUE;                           /* Yes                                      */
    } else {
        tasks_waiting = SYS_FALSE;                          /* No                                       */
    }
    switch (opt) {
        case SYS_DEL_NO_PEND:                               /* Delete group if no task waiting          */
             if (tasks_waiting == SYS_FALSE) {
#if SYS_FLAG_NAME_SIZE > 1
                 pgrp->SysFlagName[0]  = '?';               /* Unknown name                             */
                 pgrp->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
                 pgrp->SysFlagType     = SYS_EVENT_TYPE_UNUSED;
                 pgrp->SysFlagWaitList = (void *)SysFlagFreeList; /* Return group to free list           */
                 pgrp->SysFlagFlags    = (SYS_FLAGS)0;
                 SysFlagFreeList       = pgrp;
                 SYS_EXIT_CRITICAL();
                 *perr                = SYS_ERR_NONE;
                 pgrp_return          = (SYS_FLAG_GRP *)0;  /* Event Flag Group has been deleted        */
             } else {
                 SYS_EXIT_CRITICAL();
                 *perr                = SYS_ERR_TASK_WAITING;
                 pgrp_return          = pgrp;
             }
             break;

        case SYS_DEL_ALWAYS:                                /* Always delete the event flag group       */
             pnode = (SYS_FLAG_NODE *)pgrp->SysFlagWaitList;
             while (pnode != (SYS_FLAG_NODE *)0) {          /* Ready ALL tasks waiting for flags        */
                 (void)SYS_FlagTaskRdy(pnode, (SYS_FLAGS)0);
                 pnode = (SYS_FLAG_NODE *)pnode->SysFlagNodeNext;
             }
#if SYS_FLAG_NAME_SIZE > 1
             pgrp->SysFlagName[0]  = '?';                   /* Unknown name                             */
             pgrp->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
             pgrp->SysFlagType     = SYS_EVENT_TYPE_UNUSED;
             pgrp->SysFlagWaitList = (void *)SysFlagFreeList;/* Return group to free list                */
             pgrp->SysFlagFlags    = (SYS_FLAGS)0;
             SysFlagFreeList       = pgrp;
             SYS_EXIT_CRITICAL();
             if (tasks_waiting == SYS_TRUE) {               /* Reschedule only if task(s) were waiting  */
                 SYS_Sched();                               /* Find highest priority task ready to run  */
             }
             *perr = SYS_ERR_NONE;
             pgrp_return          = (SYS_FLAG_GRP *)0;      /* Event Flag Group has been deleted        */
             break;

        default:
             SYS_EXIT_CRITICAL();
             *perr                = SYS_ERR_INVALID_OPT;
             pgrp_return          = pgrp;
             break;
    }
    return (pgrp_return);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                 GET THE NAME OF AN EVENT FLAG GROUP
*
* Description: This function is used to obtain the name assigned to an event flag group
*
* Arguments  : pgrp      is a pointer to the event flag group.
*
*              pname     is a pointer to an ASCII string that will receive the name of the event flag
*                        group.  The string must be able to hold at least SYS_FLAG_NAME_SIZE characters.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the requested task is resumed
*                        SYS_ERR_EVENT_TYPE          if 'pevent' is not pointing to an event flag group
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_FLAG_INVALID_PGRP   if you passed a NULL pointer for 'pgrp'
*                        SYS_ERR_NAME_GET_ISR        if you called this function from an ISR
*
* Returns    : The length of the string or 0 if the 'pgrp' is a NULL pointer.
*********************************************************************************************************
*/

#if SYS_FLAG_NAME_SIZE > 1
INT8U  SysFlagNameGet (SYS_FLAG_GRP *pgrp, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return (0);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {              /* Is 'pgrp' a NULL pointer?                          */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return (0);
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = SYS_ERR_PNAME_NULL;
        return (0);
    }
#endif
    if (SysIntNesting > 0) {                      /* See if trying to call from an ISR                  */
        *perr = SYS_ERR_NAME_GET_ISR;
        return (0);
    }
    SYS_ENTER_CRITICAL();
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_EVENT_TYPE;
        return (0);
    }
    len   = SYS_StrCopy(pname, pgrp->SysFlagName); /* Copy name from SYS_FLAG_GRP                         */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (len);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                 ASSIGN A NAME TO AN EVENT FLAG GROUP
*
* Description: This function assigns a name to an event flag group.
*
* Arguments  : pgrp      is a pointer to the event flag group.
*
*              pname     is a pointer to an ASCII string that will be used as the name of the event flag
*                        group.  The string must be able to hold at least SYS_FLAG_NAME_SIZE characters.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the requested task is resumed
*                        SYS_ERR_EVENT_TYPE          if 'pevent' is not pointing to an event flag group
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_FLAG_INVALID_PGRP   if you passed a NULL pointer for 'pgrp'
*                        SYS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/

#if SYS_FLAG_NAME_SIZE > 1
void  SysFlagNameSet (SYS_FLAG_GRP *pgrp, INT8U *pname, INT8U *perr)
{
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                    /* Validate 'perr'                                    */
        return;
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {              /* Is 'pgrp' a NULL pointer?                          */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
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
    SYS_ENTER_CRITICAL();
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_EVENT_TYPE;
        return;
    }
    len = SYS_StrLen(pname);                      /* Can we fit the string in the storage area?         */
    if (len > (SYS_FLAG_NAME_SIZE - 1)) {         /* No                                                 */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_FLAG_NAME_TOO_LONG;
        return;
    }
    (void)SYS_StrCopy(pgrp->SysFlagName, pname);   /* Yes, copy name from SYS_FLAG_GRP                    */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        WAIT ON AN EVENT FLAG GROUP
*
* Description: This function is called to wait for a combination of bits to be set in an event flag
*              group.  Your application can wait for ANY bit to be set or ALL bits to be set.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         Is a bit pattern indicating which bit(s) (i.e. flags) you wish to wait for.
*                            The bits you want are specified by setting the corresponding bits in
*                            'flags'.  e.g. if your application wants to wait for bits 0 and 1 then
*                            'flags' would contain 0x03.
*
*              wait_type     specifies whether you want ALL bits to be set or ANY of the bits to be set.
*                            You can specify the following argument:
*
*                            SYS_FLAG_WAIT_CLR_ALL   You will wait for ALL bits in 'mask' to be clear (0)
*                            SYS_FLAG_WAIT_SET_ALL   You will wait for ALL bits in 'mask' to be set   (1)
*                            SYS_FLAG_WAIT_CLR_ANY   You will wait for ANY bit  in 'mask' to be clear (0)
*                            SYS_FLAG_WAIT_SET_ANY   You will wait for ANY bit  in 'mask' to be set   (1)
*
*                            NOTE: Add SYS_FLAG_CONSUME if you want the event flag to be 'consumed' by
*                                  the call.  Example, to wait for any flag in a group AND then clear
*                                  the flags that are present, set 'wait_type' to:
*
*                                  SYS_FLAG_WAIT_SET_ANY + SYS_FLAG_CONSUME
*
*              timeout       is an optional timeout (in clock ticks) that your task will wait for the
*                            desired bit combination.  If you specify 0, however, your task will wait
*                            forever at the specified event flag group or, until a message arrives.
*
*              perr          is a pointer to an error code and can be:
*                            SYS_ERR_NONE               The desired bits have been set within the specified
*                                                      'timeout'.
*                            SYS_ERR_PEND_ISR           If you tried to PEND from an ISR
*                            SYS_ERR_FLAG_INVALID_PGRP  If 'pgrp' is a NULL pointer.
*                            SYS_ERR_EVENT_TYPE         You are not pointing to an event flag group
*                            SYS_ERR_TIMEOUT            The bit(s) have not been set in the specified
*                                                      'timeout'.
*                            SYS_ERR_PEND_ABORT         The wait on the flag was aborted.
*                            SYS_ERR_FLAG_WAIT_TYPE     You didn't specify a proper 'wait_type' argument.
*
* Returns    : The flags in the event flag group that made the task ready or, 0 if a timeout or an error
*              occurred.
*
* Called from: Task ONLY
*
* Note(s)    : 1) IMPORTANT, the behavior of this function has changed from PREVIOUS versions.  The
*                 function NOW returns the flags that were ready INSTEAD of the current state of the
*                 event flags.
*********************************************************************************************************
*/

SYS_FLAGS  SysFlagPend (SYS_FLAG_GRP *pgrp, SYS_FLAGS flags, INT8U wait_type, INT16U timeout, INT8U *perr)
{
    SYS_FLAG_NODE  node;
    SYS_FLAGS      flags_rdy;
    INT8U         result;
    INT8U         pend_stat;
    BOOLEAN       consume;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR     cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                              /* Validate 'perr'                          */
        return ((SYS_FLAGS)0);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {                        /* Validate 'pgrp'                          */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return ((SYS_FLAGS)0);
    }
#endif
    if (SysIntNesting > 0) {                                /* See if called from ISR ...               */
        *perr = SYS_ERR_PEND_ISR;                           /* ... can't PEND from an ISR               */
        return ((SYS_FLAGS)0);
    }
    if (SysLockNesting > 0) {                               /* See if called with scheduler locked ...  */
        *perr = SYS_ERR_PEND_LOCKED;                        /* ... can't PEND when locked               */
        return ((SYS_FLAGS)0);
    }
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {          /* Validate event block type                */
        *perr = SYS_ERR_EVENT_TYPE;
        return ((SYS_FLAGS)0);
    }
    result = (INT8U)(wait_type & SYS_FLAG_CONSUME);
    if (result != (INT8U)0) {                             /* See if we need to consume the flags      */
        wait_type &= ~(INT8U)SYS_FLAG_CONSUME;
        consume    = SYS_TRUE;
    } else {
        consume    = SYS_FALSE;
    }
/*$PAGE*/
    SYS_ENTER_CRITICAL();
    switch (wait_type) {
        case SYS_FLAG_WAIT_SET_ALL:                         /* See if all required flags are set        */
             flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & flags);   /* Extract only the bits we want     */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we wanted      */
                 }
                 SysTCBCur->SysTCBFlagsRdy = flags_rdy;      /* Save flags that were ready               */
                 SYS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *perr                   = SYS_ERR_NONE;
                 return (flags_rdy);
             } else {                                      /* Block task until events occur or timeout */
                 SYS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 SYS_EXIT_CRITICAL();
             }
             break;

        case SYS_FLAG_WAIT_SET_ANY:
             flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & flags);    /* Extract only the bits we want    */
             if (flags_rdy != (SYS_FLAGS)0) {               /* See if any flag set                      */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags &= ~flags_rdy;      /* Clear ONLY the flags that we got         */
                 }
                 SysTCBCur->SysTCBFlagsRdy = flags_rdy;      /* Save flags that were ready               */
                 SYS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *perr                   = SYS_ERR_NONE;
                 return (flags_rdy);
             } else {                                      /* Block task until events occur or timeout */
                 SYS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 SYS_EXIT_CRITICAL();
             }
             break;

#if SYS_FLAG_WAIT_CLR_EN > 0
        case SYS_FLAG_WAIT_CLR_ALL:                         /* See if all required flags are cleared    */
             flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & flags);  /* Extract only the bits we want     */
             if (flags_rdy == flags) {                     /* Must match ALL the bits that we want     */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags |= flags_rdy;       /* Set ONLY the flags that we wanted        */
                 }
                 SysTCBCur->SysTCBFlagsRdy = flags_rdy;      /* Save flags that were ready               */
                 SYS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *perr                   = SYS_ERR_NONE;
                 return (flags_rdy);
             } else {                                      /* Block task until events occur or timeout */
                 SYS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 SYS_EXIT_CRITICAL();
             }
             break;

        case SYS_FLAG_WAIT_CLR_ANY:
             flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & flags); /* Extract only the bits we want      */
             if (flags_rdy != (SYS_FLAGS)0) {               /* See if any flag cleared                  */
                 if (consume == SYS_TRUE) {                 /* See if we need to consume the flags      */
                     pgrp->SysFlagFlags |= flags_rdy;       /* Set ONLY the flags that we got           */
                 }
                 SysTCBCur->SysTCBFlagsRdy = flags_rdy;      /* Save flags that were ready               */
                 SYS_EXIT_CRITICAL();                       /* Yes, condition met, return to caller     */
                 *perr                   = SYS_ERR_NONE;
                 return (flags_rdy);
             } else {                                      /* Block task until events occur or timeout */
                 SYS_FlagBlock(pgrp, &node, flags, wait_type, timeout);
                 SYS_EXIT_CRITICAL();
             }
             break;
#endif

        default:
             SYS_EXIT_CRITICAL();
             flags_rdy = (SYS_FLAGS)0;
             *perr      = SYS_ERR_FLAG_WAIT_TYPE;
             return (flags_rdy);
    }
/*$PAGE*/
    SYS_Sched();                                            /* Find next HPT ready to run               */
    SYS_ENTER_CRITICAL();
    if (SysTCBCur->SysTCBStatPend != SYS_STAT_PEND_OK) {      /* Have we timed-out or aborted?            */
        pend_stat                = SysTCBCur->SysTCBStatPend;
        SysTCBCur->SysTCBStatPend  = SYS_STAT_PEND_OK;
        SYS_FlagUnlink(&node);
        SysTCBCur->SysTCBStat      = SYS_STAT_RDY;            /* Yes, make task ready-to-run              */
        SYS_EXIT_CRITICAL();
        flags_rdy                = (SYS_FLAGS)0;
        switch (pend_stat) {
            case SYS_STAT_PEND_ABORT:
                 *perr = SYS_ERR_PEND_ABORT;                 /* Indicate that we aborted   waiting       */
                 break;

            case SYS_STAT_PEND_TO:
            default:
                 *perr = SYS_ERR_TIMEOUT;                    /* Indicate that we timed-out waiting       */
                 break;
        }
        return (flags_rdy);
    }
    flags_rdy = SysTCBCur->SysTCBFlagsRdy;
    if (consume == SYS_TRUE) {                              /* See if we need to consume the flags      */
        switch (wait_type) {
            case SYS_FLAG_WAIT_SET_ALL:
            case SYS_FLAG_WAIT_SET_ANY:                     /* Clear ONLY the flags we got              */
                 pgrp->SysFlagFlags &= ~flags_rdy;
                 break;

#if SYS_FLAG_WAIT_CLR_EN > 0
            case SYS_FLAG_WAIT_CLR_ALL:
            case SYS_FLAG_WAIT_CLR_ANY:                     /* Set   ONLY the flags we got              */
                 pgrp->SysFlagFlags |=  flags_rdy;
                 break;
#endif
            default:
                 SYS_EXIT_CRITICAL();
                 *perr = SYS_ERR_FLAG_WAIT_TYPE;
                 return ((SYS_FLAGS)0);
        }
    }
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;                                   /* Event(s) must have occurred              */
    return (flags_rdy);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                               GET FLAGS WHO CAUSED TASK TO BECOME READY
*
* Description: This function is called to obtain the flags that caused the task to become ready to run.
*              In other words, this function allows you to tell "Who done it!".
*
* Arguments  : None
*
* Returns    : The flags that caused the task to be ready.
*
* Called from: Task ONLY
*********************************************************************************************************
*/

SYS_FLAGS  SysFlagPendGetFlagsRdy (void)
{
    SYS_FLAGS      flags;
#if SYS_CRITICAL_METHOD == 3                                /* Allocate storage for CPU status register */
    SYS_CPU_SR     cpu_sr = 0;
#endif



    SYS_ENTER_CRITICAL();
    flags = SysTCBCur->SysTCBFlagsRdy;
    SYS_EXIT_CRITICAL();
    return (flags);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                         PSysT EVENT FLAG BIT(S)
*
* Description: This function is called to set or clear some bits in an event flag group.  The bits to
*              set or clear are specified by a 'bit mask'.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              flags         If 'opt' (see below) is SYS_FLAG_SET, each bit that is set in 'flags' will
*                            set the corresponding bit in the event flag group.  e.g. to set bits 0, 4
*                            and 5 you would set 'flags' to:
*
*                                0x31     (note, bit 0 is least significant bit)
*
*                            If 'opt' (see below) is SYS_FLAG_CLR, each bit that is set in 'flags' will
*                            CLEAR the corresponding bit in the event flag group.  e.g. to clear bits 0,
*                            4 and 5 you would specify 'flags' as:
*
*                                0x31     (note, bit 0 is least significant bit)
*
*              opt           indicates whether the flags will be:
*                                set     (SYS_FLAG_SET) or
*                                cleared (SYS_FLAG_CLR)
*
*              perr          is a pointer to an error code and can be:
*                            SYS_ERR_NONE                The call was successfull
*                            SYS_ERR_FLAG_INVALID_PGRP   You passed a NULL pointer
*                            SYS_ERR_EVENT_TYPE          You are not pointing to an event flag group
*                            SYS_ERR_FLAG_INVALID_OPT    You specified an invalid option
*
* Returns    : the new value of the event flags bits that are still set.
*
* Called From: Task or ISR
*
* WARNING(s) : 1) The execution time of this function depends on the number of tasks waiting on the event
*                 flag group.
*              2) The amount of time interrupts are DISABLED depends on the number of tasks waiting on
*                 the event flag group.
*********************************************************************************************************
*/
SYS_FLAGS  SysFlagPost (SYS_FLAG_GRP *pgrp, SYS_FLAGS flags, INT8U opt, INT8U *perr)
{
    SYS_FLAG_NODE *pnode;
    BOOLEAN       sched;
    SYS_FLAGS      flags_cur;
    SYS_FLAGS      flags_rdy;
    BOOLEAN       rdy;
#if SYS_CRITICAL_METHOD == 3                          /* Allocate storage for CPU status register       */
    SYS_CPU_SR     cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                        /* Validate 'perr'                                */
        return ((SYS_FLAGS)0);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {                  /* Validate 'pgrp'                                */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return ((SYS_FLAGS)0);
    }
#endif
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) {    /* Make sure we are pointing to an event flag grp */
        *perr = SYS_ERR_EVENT_TYPE;
        return ((SYS_FLAGS)0);
    }
/*$PAGE*/
    SYS_ENTER_CRITICAL();
    switch (opt) {
        case SYS_FLAG_CLR:
             pgrp->SysFlagFlags &= ~flags;            /* Clear the flags specified in the group         */
             break;

        case SYS_FLAG_SET:
             pgrp->SysFlagFlags |=  flags;            /* Set   the flags specified in the group         */
             break;

        default:
             SYS_EXIT_CRITICAL();                     /* INVALID option                                 */
             *perr = SYS_ERR_FLAG_INVALID_OPT;
             return ((SYS_FLAGS)0);
    }
    sched = SYS_FALSE;                                /* Indicate that we don't need rescheduling       */
    pnode = (SYS_FLAG_NODE *)pgrp->SysFlagWaitList;
    while (pnode != (SYS_FLAG_NODE *)0) {             /* Go through all tasks waiting on event flag(s)  */
        switch (pnode->SysFlagNodeWaitType) {
            case SYS_FLAG_WAIT_SET_ALL:               /* See if all req. flags are set for current node */
                 flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & pnode->SysFlagNodeFlags);
                 if (flags_rdy == pnode->SysFlagNodeFlags) {
                     rdy = SYS_FlagTaskRdy(pnode, flags_rdy);  /* Make task RTR, event(s) Rx'd          */
                     if (rdy == SYS_TRUE) {
                         sched = SYS_TRUE;                     /* When done we will reschedule          */
                     }
                 }
                 break;

            case SYS_FLAG_WAIT_SET_ANY:               /* See if any flag set                            */
                 flags_rdy = (SYS_FLAGS)(pgrp->SysFlagFlags & pnode->SysFlagNodeFlags);
                 if (flags_rdy != (SYS_FLAGS)0) {
                     rdy = SYS_FlagTaskRdy(pnode, flags_rdy);  /* Make task RTR, event(s) Rx'd          */
                     if (rdy == SYS_TRUE) {
                         sched = SYS_TRUE;                     /* When done we will reschedule          */
                     }
                 }
                 break;

#if SYS_FLAG_WAIT_CLR_EN > 0
            case SYS_FLAG_WAIT_CLR_ALL:               /* See if all req. flags are set for current node */
                 flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & pnode->SysFlagNodeFlags);
                 if (flags_rdy == pnode->SysFlagNodeFlags) {
                     rdy = SYS_FlagTaskRdy(pnode, flags_rdy);  /* Make task RTR, event(s) Rx'd          */
                     if (rdy == SYS_TRUE) {
                         sched = SYS_TRUE;                     /* When done we will reschedule          */
                     }
                 }
                 break;

            case SYS_FLAG_WAIT_CLR_ANY:               /* See if any flag set                            */
                 flags_rdy = (SYS_FLAGS)(~pgrp->SysFlagFlags & pnode->SysFlagNodeFlags);
                 if (flags_rdy != (SYS_FLAGS)0) {
                     rdy = SYS_FlagTaskRdy(pnode, flags_rdy);  /* Make task RTR, event(s) Rx'd          */
                     if (rdy == SYS_TRUE) {
                         sched = SYS_TRUE;                     /* When done we will reschedule          */
                     }
                 }
                 break;
#endif
            default:
                 SYS_EXIT_CRITICAL();
                 *perr = SYS_ERR_FLAG_WAIT_TYPE;
                 return ((SYS_FLAGS)0);
        }
        pnode = (SYS_FLAG_NODE *)pnode->SysFlagNodeNext; /* Point to next task waiting for event flag(s) */
    }
    SYS_EXIT_CRITICAL();
    if (sched == SYS_TRUE) {
        SYS_Sched();
    }
    SYS_ENTER_CRITICAL();
    flags_cur = pgrp->SysFlagFlags;
    SYS_EXIT_CRITICAL();
    *perr     = SYS_ERR_NONE;
    return (flags_cur);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           QUERY EVENT FLAG
*
* Description: This function is used to check the value of the event flag group.
*
* Arguments  : pgrp         is a pointer to the desired event flag group.
*
*              perr          is a pointer to an error code returned to the called:
*                            SYS_ERR_NONE                The call was successfull
*                            SYS_ERR_FLAG_INVALID_PGRP   You passed a NULL pointer
*                            SYS_ERR_EVENT_TYPE          You are not pointing to an event flag group
*
* Returns    : The current value of the event flag group.
*
* Called From: Task or ISR
*********************************************************************************************************
*/

#if SYS_FLAG_QUERY_EN > 0
SYS_FLAGS  SysFlagQuery (SYS_FLAG_GRP *pgrp, INT8U *perr)
{
    SYS_FLAGS   flags;
#if SYS_CRITICAL_METHOD == 3                       /* Allocate storage for CPU status register          */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                     /* Validate 'perr'                                   */
        return ((SYS_FLAGS)0);
    }
    if (pgrp == (SYS_FLAG_GRP *)0) {               /* Validate 'pgrp'                                   */
        *perr = SYS_ERR_FLAG_INVALID_PGRP;
        return ((SYS_FLAGS)0);
    }
#endif
    if (pgrp->SysFlagType != SYS_EVENT_TYPE_FLAG) { /* Validate event block type                         */
        *perr = SYS_ERR_EVENT_TYPE;
        return ((SYS_FLAGS)0);
    }
    SYS_ENTER_CRITICAL();
    flags = pgrp->SysFlagFlags;
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (flags);                               /* Return the current value of the event flags       */
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                         SUSPEND TASK UNTIL EVENT FLAG(s) RECEIVED OR TIMEOUT OCCURS
*
* Description: This function is internal to uC/Sys-II and is used to put a task to sleep until the desired
*              event flag bit(s) are set.
*
* Arguments  : pgrp          is a pointer to the desired event flag group.
*
*              pnode         is a pointer to a structure which contains data about the task waiting for
*                            event flag bit(s) to be set.
*
*              flags         Is a bit pattern indicating which bit(s) (i.e. flags) you wish to check.
*                            The bits you want are specified by setting the corresponding bits in
*                            'flags'.  e.g. if your application wants to wait for bits 0 and 1 then
*                            'flags' would contain 0x03.
*
*              wait_type     specifies whether you want ALL bits to be set/cleared or ANY of the bits
*                            to be set/cleared.
*                            You can specify the following argument:
*
*                            SYS_FLAG_WAIT_CLR_ALL   You will check ALL bits in 'mask' to be clear (0)
*                            SYS_FLAG_WAIT_CLR_ANY   You will check ANY bit  in 'mask' to be clear (0)
*                            SYS_FLAG_WAIT_SET_ALL   You will check ALL bits in 'mask' to be set   (1)
*                            SYS_FLAG_WAIT_SET_ANY   You will check ANY bit  in 'mask' to be set   (1)
*
*              timeout       is the desired amount of time that the task will wait for the event flag
*                            bit(s) to be set.
*
* Returns    : none
*
* Called by  : SysFlagPend()  SYS_FLAG.C
*
* Note(s)    : This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

static  void  SYS_FlagBlock (SYS_FLAG_GRP *pgrp, SYS_FLAG_NODE *pnode, SYS_FLAGS flags, INT8U wait_type, INT16U timeout)
{
    SYS_FLAG_NODE  *pnode_next;
    INT8U          y;


    SysTCBCur->SysTCBStat      |= SYS_STAT_FLAG;
    SysTCBCur->SysTCBStatPend   = SYS_STAT_PEND_OK;
    SysTCBCur->SysTCBDly        = timeout;              /* Store timeout in task's TCB                   */
#if SYS_TASK_DEL_EN > 0
    SysTCBCur->SysTCBFlagNode   = pnode;                /* TCB to link to node                           */
#endif
    pnode->SysFlagNodeFlags    = flags;                /* Save the flags that we need to wait for       */
    pnode->SysFlagNodeWaitType = wait_type;            /* Save the type of wait we are doing            */
    pnode->SysFlagNodeTCB      = (void *)SysTCBCur;     /* Link to task's TCB                            */
    pnode->SysFlagNodeNext     = pgrp->SysFlagWaitList; /* Add node at beginning of event flag wait list */
    pnode->SysFlagNodePrev     = (void *)0;
    pnode->SysFlagNodeFlagGrp  = (void *)pgrp;         /* Link to Event Flag Group                      */
    pnode_next                = (SYS_FLAG_NODE *)pgrp->SysFlagWaitList;
    if (pnode_next != (void *)0) {                    /* Is this the first NODE to insert?             */
        pnode_next->SysFlagNodePrev = pnode;           /* No, link in doubly linked list                */
    }
    pgrp->SysFlagWaitList = (void *)pnode;

    y            =  SysTCBCur->SysTCBY;                 /* Suspend current task until flag(s) received   */
    SysRdyTbl[y] &= ~SysTCBCur->SysTCBBitX;
    if (SysRdyTbl[y] == 0x00) {
        SysRdyGrp &= ~SysTCBCur->SysTCBBitY;
    }
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                    INITIALIZE THE EVENT FLAG MODULE
*
* Description: This function is called by uC/Sys-II to initialize the event flag module.  Your application
*              MUST NOT call this function.  In other words, this function is internal to uC/Sys-II.
*
* Arguments  : none
*
* Returns    : none
*
* WARNING    : You MUST NOT call this function from your code.  This is an INTERNAL function to uC/Sys-II.
*********************************************************************************************************
*/

void  SYS_FlagInit (void)
{
#if SYS_MAX_FLAGS == 1
    SysFlagFreeList                 = (SYS_FLAG_GRP *)&SysFlagTbl[0];  /* Only ONE event flag group!      */
    SysFlagFreeList->SysFlagType     = SYS_EVENT_TYPE_UNUSED;
    SysFlagFreeList->SysFlagWaitList = (void *)0;
    SysFlagFreeList->SysFlagFlags    = (SYS_FLAGS)0;
#if SYS_FLAG_NAME_SIZE > 1
    SysFlagFreeList->SysFlagName[0]  = '?';
    SysFlagFreeList->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
#endif

#if SYS_MAX_FLAGS >= 2
    INT16U       i;
    SYS_FLAG_GRP *pgrp1;
    SYS_FLAG_GRP *pgrp2;


    SYS_MemClr((INT8U *)&SysFlagTbl[0], sizeof(SysFlagTbl));           /* Clear the flag group table      */
    pgrp1 = &SysFlagTbl[0];
    pgrp2 = &SysFlagTbl[1];
    for (i = 0; i < (SYS_MAX_FLAGS - 1); i++) {                      /* Init. list of free EVENT FLAGS  */
        pgrp1->SysFlagType     = SYS_EVENT_TYPE_UNUSED;
        pgrp1->SysFlagWaitList = (void *)pgrp2;
#if SYS_FLAG_NAME_SIZE > 1
        pgrp1->SysFlagName[0]  = '?';                                /* Unknown name                    */
        pgrp1->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
        pgrp1++;
        pgrp2++;
    }
    pgrp1->SysFlagType     = SYS_EVENT_TYPE_UNUSED;
    pgrp1->SysFlagWaitList = (void *)0;
#if SYS_FLAG_NAME_SIZE > 1
    pgrp1->SysFlagName[0]  = '?';                                    /* Unknown name                    */
    pgrp1->SysFlagName[1]  = SYS_ASCII_NUL;
#endif
    SysFlagFreeList        = &SysFlagTbl[0];
#endif
}

/*$PAGE*/
/*
*********************************************************************************************************
*                              MAKE TASK READY-TO-RUN, EVENT(s) OCCURRED
*
* Description: This function is internal to uC/Sys-II and is used to make a task ready-to-run because the
*              desired event flag bits have been set.
*
* Arguments  : pnode         is a pointer to a structure which contains data about the task waiting for
*                            event flag bit(s) to be set.
*
*              flags_rdy     contains the bit pattern of the event flags that cause the task to become
*                            ready-to-run.
*
* Returns    : SYS_TRUE       If the task has been placed in the ready list and thus needs scheduling
*              SYS_FALSE      The task is still not ready to run and thus scheduling is not necessary
*
* Called by  : SysFlagsPost() SYS_FLAG.C
*
* Note(s)    : 1) This function assumes that interrupts are disabled.
*              2) This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

static  BOOLEAN  SYS_FlagTaskRdy (SYS_FLAG_NODE *pnode, SYS_FLAGS flags_rdy)
{
    SYS_TCB   *ptcb;
    BOOLEAN   sched;


    ptcb                 = (SYS_TCB *)pnode->SysFlagNodeTCB; /* Point to TCB of waiting task             */
    ptcb->SysTCBDly       = 0;
    ptcb->SysTCBFlagsRdy  = flags_rdy;
    ptcb->SysTCBStat     &= ~(INT8U)SYS_STAT_FLAG;
    ptcb->SysTCBStatPend  = SYS_STAT_PEND_OK;
    if (ptcb->SysTCBStat == SYS_STAT_RDY) {                  /* Task now ready?                          */
        SysRdyGrp               |= ptcb->SysTCBBitY;         /* Put task into ready list                 */
        SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
        sched                   = SYS_TRUE;
    } else {
        sched                   = SYS_FALSE;
    }
    SYS_FlagUnlink(pnode);
    return (sched);
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                  UNLINK EVENT FLAG NODE FROM WAITING LIST
*
* Description: This function is internal to uC/Sys-II and is used to unlink an event flag node from a
*              list of tasks waiting for the event flag.
*
* Arguments  : pnode         is a pointer to a structure which contains data about the task waiting for
*                            event flag bit(s) to be set.
*
* Returns    : none
*
* Called by  : SYS_FlagTaskRdy() SYS_FLAG.C
*              SysFlagPend()     SYS_FLAG.C
*              SysTaskDel()      SYS_TASK.C
*
* Note(s)    : 1) This function assumes that interrupts are disabled.
*              2) This function is INTERNAL to uC/Sys-II and your application should not call it.
*********************************************************************************************************
*/

void  SYS_FlagUnlink (SYS_FLAG_NODE *pnode)
{
#if SYS_TASK_DEL_EN > 0
    SYS_TCB       *ptcb;
#endif
    SYS_FLAG_GRP  *pgrp;
    SYS_FLAG_NODE *pnode_prev;
    SYS_FLAG_NODE *pnode_next;


    pnode_prev = (SYS_FLAG_NODE *)pnode->SysFlagNodePrev;
    pnode_next = (SYS_FLAG_NODE *)pnode->SysFlagNodeNext;
    if (pnode_prev == (SYS_FLAG_NODE *)0) {                      /* Is it first node in wait list?      */
        pgrp                 = (SYS_FLAG_GRP *)pnode->SysFlagNodeFlagGrp;
        pgrp->SysFlagWaitList = (void *)pnode_next;              /*      Update list for new 1st node   */
        if (pnode_next != (SYS_FLAG_NODE *)0) {
            pnode_next->SysFlagNodePrev = (SYS_FLAG_NODE *)0;     /*      Link new 1st node PREV to NULL */
        }
    } else {                                                    /* No,  A node somewhere in the list   */
        pnode_prev->SysFlagNodeNext = pnode_next;                /*      Link around the node to unlink */
        if (pnode_next != (SYS_FLAG_NODE *)0) {                  /*      Was this the LAST node?        */
            pnode_next->SysFlagNodePrev = pnode_prev;            /*      No, Link around current node   */
        }
    }
#if SYS_TASK_DEL_EN > 0
    ptcb                = (SYS_TCB *)pnode->SysFlagNodeTCB;
    ptcb->SysTCBFlagNode = (SYS_FLAG_NODE *)0;
#endif
}
#endif
