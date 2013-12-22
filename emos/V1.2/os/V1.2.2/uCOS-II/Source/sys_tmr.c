/*
************************************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                            TIMER MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
*
* File    : SYS_TMR.C
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
************************************************************************************************************************
*/

#include <C2.h>

/*
************************************************************************************************************************
*                                                        NOTES
*
* 1) Your application MUST define the following #define constants:
*
*    SYS_TASK_TMR_PRIO          The priority of the Timer management task
*    SYS_TASK_TMR_STK_SIZE      The size     of the Timer management task's stack
*
* 2) You must call SysTmrSignal() to notify the Timer management task that it's time to update the timers.
************************************************************************************************************************
*/

/*
************************************************************************************************************************
*                                                     CONSTANTS
************************************************************************************************************************
*/

#define  SYS_TMR_LINK_DLY       0
#define  SYS_TMR_LINK_PERIODIC  1

/*
************************************************************************************************************************
*                                                  LOCAL PROTOTYPES
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  SYS_TMR  *SysTmr_Alloc         (void);
static  void     SysTmr_Free          (SYS_TMR *ptmr);
static  void     SysTmr_InitTask      (void);
static  void     SysTmr_Link          (SYS_TMR *ptmr, INT8U type);
static  void     SysTmr_Unlink        (SYS_TMR *ptmr);
static  void     SysTmr_Lock          (void);
static  void     SysTmr_Unlock        (void);
static  void     SysTmr_Task          (void   *p_arg);
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   CREATE A TIMER
*
* Description: This function is called by your application code to create a timer.
*
* Arguments  : dly           Initial delay.
*                            If the timer is configured for ONE-SHOT mode, this is the timeout used
*                            If the timer is configured for PERIODIC mode, this is the first timeout to wait for
*                               before the timer starts entering periodic mode
*
*              period        The 'period' being repeated for the timer.
*                               If you specified 'SYS_TMR_OPT_PERIODIC' as an option, when the timer expires, it will
*                               automatically restart with the same period.
*
*              opt           Specifies either:
*                               SYS_TMR_OPT_ONE_SHOT       The timer counts down only once
*                               SYS_TMR_OPT_PERIODIC       The timer counts down and then reloads itself
*
*              callback      Is a pointer to a callback function that will be called when the timer expires.  The
*                               callback function must be declared as follows:
*
*                               void MyCallback (SYS_TMR *ptmr, void *p_arg);
*
*              callback_arg  Is an argument (a pointer) that is passed to the callback function when it is called.
*
*              pname         Is a pointer to an ASCII string that is used to name the timer.  Names are useful for
*                               debugging.  The length of the ASCII string for the name can be as big as:
*
*                               SYS_TMR_CFG_NAME_SIZE and should be found in SYS_CFG.H
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID_DLY     you specified an invalid delay
*                               SYS_ERR_TMR_INVALID_PERIOD  you specified an invalid period
*                               SYS_ERR_TMR_INVALID_OPT     you specified an invalid option
*                               SYS_ERR_TMR_ISR             if the call was made from an ISR
*                               SYS_ERR_TMR_NON_AVAIL       if there are no free timers from the timer pool
*                               SYS_ERR_TMR_NAME_TOO_LONG   if the timer name is too long to fit
*
* Returns    : A pointer to an SYS_TMR data structure.  
*              This is the 'handle' that your application will use to reference the timer created.
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
SYS_TMR  *SysTmrCreate (INT32U           dly,
                      INT32U           period,
                      INT8U            opt,
                      SYS_TMR_CALLBACK  callback,
                      void            *callback_arg,
                      INT8U           *pname,
                      INT8U           *perr)
{
    SYS_TMR   *ptmr;
#if SYS_TMR_CFG_NAME_SIZE > 0
    INT8U     len;
#endif


#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                               /* Validate arguments                                     */
        return ((SYS_TMR *)0);
    }
    switch (opt) {
        case SYS_TMR_OPT_PERIODIC:
             if (period == 0) {
                 *perr = SYS_ERR_TMR_INVALID_PERIOD;
                 return ((SYS_TMR *)0);
             }
             break;

        case SYS_TMR_OPT_ONE_SHOT:
             if (dly == 0) {
                 *perr = SYS_ERR_TMR_INVALID_DLY;
                 return ((SYS_TMR *)0);
             }
             break;

        default:
             *perr = SYS_ERR_TMR_INVALID_OPT;
             return ((SYS_TMR *)0);
    }
#endif
    if (SysIntNesting > 0) {                                 /* See if trying to call from an ISR                      */
        *perr  = SYS_ERR_TMR_ISR;
        return ((SYS_TMR *)0);
    }
    SysTmr_Lock();
    ptmr = SysTmr_Alloc();                                   /* Obtain a timer from the free pool                      */
    if (ptmr == (SYS_TMR *)0) {
        SysTmr_Unlock();
        *perr = SYS_ERR_TMR_NON_AVAIL;
        return ((SYS_TMR *)0);
    }
    ptmr->SysTmrState       = SYS_TMR_STATE_STOPPED;          /* Indicate that timer is not running yet                 */
    ptmr->SysTmrDly         = dly;
    ptmr->SysTmrPeriod      = period;
    ptmr->SysTmrOpt         = opt;
    ptmr->SysTmrCallback    = callback;
    ptmr->SysTmrCallbackArg = callback_arg;
#if SYS_TMR_CFG_NAME_SIZE > 0
    if (pname !=(INT8U *)0) {
        len = SYS_StrLen(pname);                             /* Copy timer name                                        */
        if (len < SYS_TMR_CFG_NAME_SIZE) {
            (void)SYS_StrCopy(ptmr->SysTmrName, pname);
        } else {
#if SYS_TMR_CFG_NAME_SIZE > 1
            ptmr->SysTmrName[0] = '#';                       /* Invalid size specified                                 */
            ptmr->SysTmrName[1] = SYS_ASCII_NUL;
#endif
            *perr              = SYS_ERR_TMR_NAME_TOO_LONG;
            SysTmr_Unlock();
            return (ptmr);
        }
    }
#endif
    SysTmr_Unlock();
    *perr = SYS_ERR_NONE;
    return (ptmr);
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   DELETE A TIMER
*
* Description: This function is called by your application code to delete a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop and delete.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID        'ptmr'  is a NULL pointer
*                               SYS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_TMR_ISR            if the function was called from an ISR
*                               SYS_ERR_TMR_INACTIVE       if the timer was not created
*                               SYS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : SYS_TRUE       If the call was successful
*              SYS_FALSE      If not
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
BOOLEAN  SysTmrDel (SYS_TMR  *ptmr,
                   INT8U   *perr)
{
#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                               /* Validate arguments                                     */
        return (SYS_FALSE);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (SYS_FALSE);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (SYS_FALSE);
    }
    if (SysIntNesting > 0) {                                 /* See if trying to call from an ISR                      */
        *perr  = SYS_ERR_TMR_ISR;
        return (SYS_FALSE);
    }
    SysTmr_Lock();
    switch (ptmr->SysTmrState) {
        case SYS_TMR_STATE_RUNNING:
             SysTmr_Unlink(ptmr);                            /* Remove from current wheel spoke                        */
             SysTmr_Free(ptmr);                              /* Return timer to free list of timers                    */
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (SYS_TRUE);

        case SYS_TMR_STATE_STOPPED:                          /* Timer has not started or ...                           */
        case SYS_TMR_STATE_COMPLETED:                        /* ... timer has completed the ONE-SHOT time              */
             SysTmr_Free(ptmr);                              /* Return timer to free list of timers                    */
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (SYS_TRUE);

        case SYS_TMR_STATE_UNUSED:                           /* Already deleted                                        */
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INACTIVE;
             return (SYS_FALSE);

        default:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INVALID_STATE;
             return (SYS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                             GET THE NAME OF A TIMER
*
* Description: This function is called to obtain the name of a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to obtain the name for
*
*              pdest         Is a pointer to where the name of the timer will be placed.  It is the caller's responsibility
*                            to ensure that he has sufficient storage in the destination, i.e. at least SYS_TMR_CFG_NAME_SIZE
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE               The call was successful
*                               SYS_ERR_TMR_INVALID_DEST   'pdest' is a NULL pointer
*                               SYS_ERR_TMR_INVALID        'ptmr'  is a NULL pointer
*                               SYS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_NAME_GET_ISR       if the call was made from an ISR
*                               SYS_ERR_TMR_INACTIVE       'ptmr'  points to a timer that is not active
*                               SYS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The length of the string or 0 if the timer does not exist.
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0 && SYS_TMR_CFG_NAME_SIZE > 0
INT8U  SysTmrNameGet (SYS_TMR  *ptmr,
                     INT8U   *pdest,
                     INT8U   *perr)
{
    INT8U  len;


#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {
        return (0);
    }
    if (pdest == (INT8U *)0) {
        *perr = SYS_ERR_TMR_INVALID_DEST;
        return (0);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (0);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (0);
    }
    if (SysIntNesting > 0) {                            /* See if trying to call from an ISR                           */
        *perr = SYS_ERR_NAME_GET_ISR;
        return (0);
    }
    SysTmr_Lock();
    switch (ptmr->SysTmrState) {
        case SYS_TMR_STATE_RUNNING:
        case SYS_TMR_STATE_STOPPED:
        case SYS_TMR_STATE_COMPLETED:
             len   = SYS_StrCopy(pdest, ptmr->SysTmrName);
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (len);

        case SYS_TMR_STATE_UNUSED:                      /* Timer is not allocated                                      */
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INACTIVE;
             return (0);

        default:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INVALID_STATE;
             return (0);
    }
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                    GET HOW MUCH TIME IS LEFT BEFORE A TIMER EXPIRES
*
* Description: This function is called to get the number of ticks before a timer times out.
*
* Arguments  : ptmr          Is a pointer to the timer to obtain the remaining time from.
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               SYS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_TMR_ISR            if the call was made from an ISR
*                               SYS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               SYS_ERR_TMR_INVALID_STATE  the timer is in an invalid state
*
* Returns    : The time remaining for the timer to expire.  The time represents 'timer' increments.  In other words, if
*              SysTmr_Task() is signaled every 1/10 of a second then the returned value represents the number of 1/10 of
*              a second remaining before the timer expires.
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
INT32U  SysTmrRemainGet (SYS_TMR  *ptmr,
                        INT8U   *perr)
{
    INT32U  remain;


#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {
        return (0);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (0);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (0);
    }
    if (SysIntNesting > 0) {                            /* See if trying to call from an ISR                           */
        *perr = SYS_ERR_TMR_ISR;
        return (0);
    }
    SysTmr_Lock();
    switch (ptmr->SysTmrState) {
        case SYS_TMR_STATE_RUNNING:
             remain = ptmr->SysTmrMatch - SysTmrTime;    /* Determine how much time is left to timeout                  */
             SysTmr_Unlock();
             *perr  = SYS_ERR_NONE;
             return (remain);

        case SYS_TMR_STATE_STOPPED:                     /* It's assumed that the timer has not started yet             */
             switch (ptmr->SysTmrOpt) {
                 case SYS_TMR_OPT_PERIODIC:
                      if (ptmr->SysTmrDly == 0) {
                          remain = ptmr->SysTmrPeriod;
                      } else {
                          remain = ptmr->SysTmrDly;
                      }
                      SysTmr_Unlock();
                      *perr  = SYS_ERR_NONE;
                      break;

                 case SYS_TMR_OPT_ONE_SHOT:
                 default:
                      remain = ptmr->SysTmrDly;
                      SysTmr_Unlock();
                      *perr  = SYS_ERR_NONE;
                      break;
             }
             return (remain);

        case SYS_TMR_STATE_COMPLETED:                   /* Only ONE-SHOT that timed out can be in this state           */
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (0);

        case SYS_TMR_STATE_UNUSED:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INACTIVE;
             return (0);

        default:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INVALID_STATE;
             return (0);
    }
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                    FIND OUT WHAT STATE A TIMER IS IN
*
* Description: This function is called to determine what state the timer is in:
*
*                  SYS_TMR_STATE_UNUSED     the timer has not been created
*                  SYS_TMR_STATE_STOPPED    the timer has been created but has not been started or has been stopped
*                  SYS_TMR_COMPLETED        the timer is in ONE-SHOT mode and has completed it's timeout
*                  SYS_TMR_RUNNING          the timer is currently running
*
* Arguments  : ptmr          Is a pointer to the desired timer
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID        'ptmr' is a NULL pointer
*                               SYS_ERR_TMR_INVALID_TYPE   'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_TMR_ISR            if the call was made from an ISR
*                               SYS_ERR_TMR_INACTIVE       'ptmr' points to a timer that is not active
*                               SYS_ERR_TMR_INVALID_STATE  if the timer is not in a valid state
*
* Returns    : The current state of the timer (see description).
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
INT8U  SysTmrStateGet (SYS_TMR  *ptmr,
                      INT8U   *perr)
{
    INT8U  state;


#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {
        return (0);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (0);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {              /* Validate timer structure                                    */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (0);
    }
    if (SysIntNesting > 0) {                            /* See if trying to call from an ISR                           */
        *perr = SYS_ERR_TMR_ISR;
        return (0);
    }
    SysTmr_Lock();
    state = ptmr->SysTmrState;
    switch (state) {
        case SYS_TMR_STATE_UNUSED:   
        case SYS_TMR_STATE_STOPPED:  
        case SYS_TMR_STATE_COMPLETED:
        case SYS_TMR_STATE_RUNNING:  
             *perr = SYS_ERR_NONE;
             break;
             
        default:
             *perr = SYS_ERR_TMR_INVALID_STATE;
             break;
    }
    SysTmr_Unlock();
    return (state);
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   START A TIMER
*
* Description: This function is called by your application code to start a timer.
*
* Arguments  : ptmr          Is a pointer to an SYS_TMR
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID
*                               SYS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_TMR_ISR             if the call was made from an ISR
*                               SYS_ERR_TMR_INACTIVE        if the timer was not created
*                               SYS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*
* Returns    : SYS_TRUE    if the timer was started
*              SYS_FALSE   if an error was detected
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
BOOLEAN  SysTmrStart (SYS_TMR   *ptmr,
                     INT8U    *perr)
{
#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                               /* Validate arguments                                     */
        return (SYS_FALSE);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (SYS_FALSE);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {                   /* Validate timer structure                               */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (SYS_FALSE);
    }
    if (SysIntNesting > 0) {                                 /* See if trying to call from an ISR                      */
        *perr  = SYS_ERR_TMR_ISR;
        return (SYS_FALSE);
    }
    SysTmr_Lock();
    switch (ptmr->SysTmrState) {
        case SYS_TMR_STATE_RUNNING:                          /* Restart the timer                                      */
             SysTmr_Unlink(ptmr);                            /* ... Stop the timer                                     */
             SysTmr_Link(ptmr, SYS_TMR_LINK_DLY);             /* ... Link timer to timer wheel                          */
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (SYS_TRUE);

        case SYS_TMR_STATE_STOPPED:                          /* Start the timer                                        */
        case SYS_TMR_STATE_COMPLETED:
             SysTmr_Link(ptmr, SYS_TMR_LINK_DLY);             /* ... Link timer to timer wheel                          */
             SysTmr_Unlock();
             *perr = SYS_ERR_NONE;
             return (SYS_TRUE);

        case SYS_TMR_STATE_UNUSED:                           /* Timer not created                                      */
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INACTIVE;
             return (SYS_FALSE);

        default:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INVALID_STATE;
             return (SYS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   STOP A TIMER
*
* Description: This function is called by your application code to stop a timer.
*
* Arguments  : ptmr          Is a pointer to the timer to stop.
*
*              opt           Allows you to specify an option to this functions which can be:
*
*                               SYS_TMR_OPT_NONE          Do nothing special but stop the timer
*                               SYS_TMR_OPT_CALLBACK      Execute the callback function, pass it the callback argument
*                                                        specified when the timer was created.
*                               SYS_TMR_OPT_CALLBACK_ARG  Execute the callback function, pass it the callback argument
*                                                        specified in THIS function call
*
*              callback_arg  Is a pointer to a 'new' callback argument that can be passed to the callback function
*                               instead of the timer's callback argument.  In other words, use 'callback_arg' passed in
*                               THIS function INSTEAD of ptmr->SysTmrCallbackArg
*
*              perr          Is a pointer to an error code.  '*perr' will contain one of the following:
*                               SYS_ERR_NONE
*                               SYS_ERR_TMR_INVALID         'ptmr' is a NULL pointer
*                               SYS_ERR_TMR_INVALID_TYPE    'ptmr'  is not pointing to an SYS_TMR
*                               SYS_ERR_TMR_ISR             if the function was called from an ISR
*                               SYS_ERR_TMR_INACTIVE        if the timer was not created
*                               SYS_ERR_TMR_INVALID_OPT     if you specified an invalid option for 'opt'
*                               SYS_ERR_TMR_STOPPED         if the timer was already stopped
*                               SYS_ERR_TMR_INVALID_STATE   the timer is in an invalid state
*                               SYS_ERR_TMR_NO_CALLBACK     if the timer does not have a callback function defined
*
* Returns    : SYS_TRUE       If we stopped the timer (if the timer is already stopped, we also return SYS_TRUE)
*              SYS_FALSE      If not
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
BOOLEAN  SysTmrStop (SYS_TMR  *ptmr,
                    INT8U    opt,
                    void    *callback_arg,
                    INT8U   *perr)
{
    SYS_TMR_CALLBACK  pfnct;


#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                                     /* Validate arguments                               */
        return (SYS_FALSE);
    }
    if (ptmr == (SYS_TMR *)0) {
        *perr = SYS_ERR_TMR_INVALID;
        return (SYS_FALSE);
    }
#endif
    if (ptmr->SysTmrType != SYS_TMR_TYPE) {                         /* Validate timer structure                         */
        *perr = SYS_ERR_TMR_INVALID_TYPE;
        return (SYS_FALSE);
    }
    if (SysIntNesting > 0) {                                       /* See if trying to call from an ISR                */
        *perr  = SYS_ERR_TMR_ISR;
        return (SYS_FALSE);
    }
    SysTmr_Lock();
    switch (ptmr->SysTmrState) {
        case SYS_TMR_STATE_RUNNING:
             SysTmr_Unlink(ptmr);                                  /* Remove from current wheel spoke                  */
             *perr = SYS_ERR_NONE;
             switch (opt) {
                 case SYS_TMR_OPT_CALLBACK:
                      pfnct = ptmr->SysTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (SYS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, ptmr->SysTmrCallbackArg);  /* Use callback arg when timer was created */
                      } else {
                          *perr = SYS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case SYS_TMR_OPT_CALLBACK_ARG:
                      pfnct = ptmr->SysTmrCallback;                /* Execute callback function if available ...       */
                      if (pfnct != (SYS_TMR_CALLBACK)0) {
                          (*pfnct)((void *)ptmr, callback_arg);   /* ... using the 'callback_arg' provided in call    */
                      } else {
                          *perr = SYS_ERR_TMR_NO_CALLBACK;
                      }
                      break;

                 case SYS_TMR_OPT_NONE:
                      break;

                 default:
                     *perr = SYS_ERR_TMR_INVALID_OPT;
                     break;
             }
             SysTmr_Unlock();
             return (SYS_TRUE);

        case SYS_TMR_STATE_COMPLETED:                              /* Timer has already completed the ONE-SHOT or ...  */
        case SYS_TMR_STATE_STOPPED:                                /* ... timer has not started yet.                   */
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_STOPPED;
             return (SYS_TRUE);

        case SYS_TMR_STATE_UNUSED:                                 /* Timer was not created                            */
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INACTIVE;
             return (SYS_FALSE);

        default:
             SysTmr_Unlock();
             *perr = SYS_ERR_TMR_INVALID_STATE;
             return (SYS_FALSE);
    }
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                      SIGNAL THAT IT'S TIME TO UPDATE THE TIMERS
*
* Description: This function is typically called by the ISR that occurs at the timer tick rate and is used to signal to
*              SysTmr_Task() that it's time to update the timers.
*
* Arguments  : none
*
* Returns    : SYS_ERR_NONE         The call was successful and the timer task was signaled.
*              SYS_ERR_SEM_OVF      If SysTmrSignal() was called more often than SysTmr_Task() can handle the timers.  
*                                  This would indicate that your system is heavily loaded.
*              SYS_ERR_EVENT_TYPE   Unlikely you would get this error because the semaphore used for signaling is created 
*                                  by uC/Sys-II.
*              SYS_ERR_PEVENT_NULL  Again, unlikely you would ever get this error because the semaphore used for signaling 
*                                  is created by uC/Sys-II.
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
INT8U  SysTmrSignal (void)
{
    INT8U  err;


    err = SysSemPost(SysTmrSemSignal);
    return (err);
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                               ALLOCATE AND FREE A TIMER
*
* Description: This function is called to allocate a timer.
*
* Arguments  : none
*
* Returns    : a pointer to a timer if one is available
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  SYS_TMR  *SysTmr_Alloc (void)
{
    SYS_TMR *ptmr;


    if (SysTmrFreeList == (SYS_TMR *)0) {
        return ((SYS_TMR *)0);
    }
    ptmr            = (SYS_TMR *)SysTmrFreeList;
    SysTmrFreeList   = (SYS_TMR *)ptmr->SysTmrNext;
    ptmr->SysTmrNext = (SYS_TCB *)0;
    ptmr->SysTmrPrev = (SYS_TCB *)0;
    SysTmrUsed++;
    SysTmrFree--;
    return (ptmr);
}
#endif


/*
************************************************************************************************************************
*                                             RETURN A TIMER TO THE FREE LIST
*
* Description: This function is called to return a timer object to the free list of timers.
*
* Arguments  : ptmr     is a pointer to the timer to free
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_Free (SYS_TMR *ptmr)
{
    ptmr->SysTmrState       = SYS_TMR_STATE_UNUSED;      /* Clear timer object fields                                   */
    ptmr->SysTmrOpt         = SYS_TMR_OPT_NONE;
    ptmr->SysTmrPeriod      = 0;
    ptmr->SysTmrMatch       = 0;
    ptmr->SysTmrCallback    = (SYS_TMR_CALLBACK)0;
    ptmr->SysTmrCallbackArg = (void *)0;
#if SYS_TMR_CFG_NAME_SIZE > 1
    ptmr->SysTmrName[0]     = '?';                      /* Unknown name                                                */
    ptmr->SysTmrName[1]     = SYS_ASCII_NUL;
#endif

    ptmr->SysTmrPrev        = (SYS_TCB *)0;              /* Chain timer to free list                                    */
    ptmr->SysTmrNext        = SysTmrFreeList;
    SysTmrFreeList          = ptmr;

    SysTmrUsed--;                                       /* Update timer object statistics                              */
    SysTmrFree++;
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                    INITIALIZATION
*                                          INITIALIZE THE FREE LIST OF TIMERS
*
* Description: This function is called by SysInit() to initialize the free list of SYS_TMRs.
*
* Arguments  : none
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
void  SysTmr_Init (void)
{
#if SYS_EVENT_NAME_SIZE > 10
    INT8U    err;
#endif
    INT16U   i;
    SYS_TMR  *ptmr1;
    SYS_TMR  *ptmr2;


    SYS_MemClr((INT8U *)&SysTmrTbl[0],      sizeof(SysTmrTbl));            /* Clear all the TMRs                         */
    SYS_MemClr((INT8U *)&SysTmrWheelTbl[0], sizeof(SysTmrWheelTbl));       /* Clear the timer wheel                      */

    ptmr1 = &SysTmrTbl[0];
    ptmr2 = &SysTmrTbl[1];
    for (i = 0; i < (SYS_TMR_CFG_MAX - 1); i++) {                        /* Init. list of free TMRs                    */
        ptmr1->SysTmrType    = SYS_TMR_TYPE;
        ptmr1->SysTmrState   = SYS_TMR_STATE_UNUSED;                      /* Indicate that timer is inactive            */
        ptmr1->SysTmrNext    = (void *)ptmr2;                            /* Link to next timer                         */
#if SYS_TMR_CFG_NAME_SIZE > 1
        ptmr1->SysTmrName[0] = '?';                                      /* Unknown name                               */
        ptmr1->SysTmrName[1] = SYS_ASCII_NUL;
#endif
        ptmr1++;
        ptmr2++;
    }
    ptmr1->SysTmrType    = SYS_TMR_TYPE;
    ptmr1->SysTmrState   = SYS_TMR_STATE_UNUSED;                          /* Indicate that timer is inactive            */
    ptmr1->SysTmrNext    = (void *)0;                                    /* Last SYS_TMR                                */
#if SYS_TMR_CFG_NAME_SIZE > 1
    ptmr1->SysTmrName[0] = '?';                                          /* Unknown name                               */
    ptmr1->SysTmrName[1] = SYS_ASCII_NUL;
#endif
    SysTmrTime           = 0;
    SysTmrUsed           = 0;
    SysTmrFree           = SYS_TMR_CFG_MAX;
    SysTmrFreeList       = &SysTmrTbl[0];
    SysTmrSem            = SysSemCreate(1);
    SysTmrSemSignal      = SysSemCreate(0);

#if SYS_EVENT_NAME_SIZE > 18
    SysEventNameSet(SysTmrSem,       (INT8U *)"uC/Sys-II TmrLock",   &err);/* Assign names to semaphores                 */
#else
#if SYS_EVENT_NAME_SIZE > 10
    SysEventNameSet(SysTmrSem,       (INT8U *)"Sys-TmrLock",         &err);
#endif
#endif

#if SYS_EVENT_NAME_SIZE > 18
    SysEventNameSet(SysTmrSemSignal, (INT8U *)"uC/Sys-II TmrSignal", &err);
#else
#if SYS_EVENT_NAME_SIZE > 10
    SysEventNameSet(SysTmrSemSignal, (INT8U *)"Sys-TmrSig",          &err);
#endif
#endif

    SysTmr_InitTask();
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                          INITIALIZE THE TIMER MANAGEMENT TASK
*
* Description: This function is called by SysTmrInit() to create the timer management task.
*
* Arguments  : none
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_InitTask (void)
{
#if SYS_TASK_NAME_SIZE > 6
    INT8U  err;
#endif


#if SYS_TASK_CREATE_EXT_EN > 0
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreateExt(SysTmr_Task,
                          (void *)0,                                       /* No arguments passed to SysTmrTask()      */
                          &SysTmrTaskStk[SYS_TASK_TMR_STK_SIZE - 1],         /* Set Top-Of-Stack                        */
                          SYS_TASK_TMR_PRIO,
                          SYS_TASK_TMR_ID,
                          &SysTmrTaskStk[0],                                /* Set Bottom-Of-Stack                     */
                          SYS_TASK_TMR_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #else
    (void)SysTaskCreateExt(SysTmr_Task,
                          (void *)0,                                       /* No arguments passed to SysTmrTask()      */
                          &SysTmrTaskStk[0],                                /* Set Top-Of-Stack                        */
                          SYS_TASK_TMR_PRIO,
                          SYS_TASK_TMR_ID,
                          &SysTmrTaskStk[SYS_TASK_TMR_STK_SIZE - 1],         /* Set Bottom-Of-Stack                     */
                          SYS_TASK_TMR_STK_SIZE,
                          (void *)0,                                       /* No TCB extension                        */
                          SYS_TASK_OPT_STK_CHK | SYS_TASK_OPT_STK_CLR);      /* Enable stack checking + clear stack     */
    #endif
#else
    #if SYS_STK_GROWTH == 1
    (void)SysTaskCreate(SysTmr_Task,
                       (void *)0,
                       &SysTmrTaskStk[SYS_TASK_TMR_STK_SIZE - 1],
                       SYS_TASK_TMR_PRIO);
    #else
    (void)SysTaskCreate(SysTmr_Task,
                       (void *)0,
                       &SysTmrTaskStk[0],
                       SYS_TASK_TMR_PRIO);
    #endif
#endif

#if SYS_TASK_NAME_SIZE > 12
    SysTaskNameSet(SYS_TASK_TMR_PRIO, (INT8U *)"uC/Sys-II Tmr", &err);
#else
#if SYS_TASK_NAME_SIZE > 6
    SysTaskNameSet(SYS_TASK_TMR_PRIO, (INT8U *)"Sys-Tmr", &err);
#endif
#endif
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                         INSERT A TIMER INTO THE TIMER WHEEL
*
* Description: This function is called to insert the timer into the timer wheel.  The timer is always inserted at the
*              beginning of the list.
*
* Arguments  : ptmr          Is a pointer to the timer to insert.
*
*              type          Is either:
*                               SYS_TMR_LINK_PERIODIC    Means to re-insert the timer after a period expired
*                               SYS_TMR_LINK_DLY         Means to insert    the timer the first time
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_Link (SYS_TMR *ptmr, INT8U type)
{
    SYS_TMR       *ptmr1;
    SYS_TMR_WHEEL *pspoke;
    INT16U        spoke;


    ptmr->SysTmrState = SYS_TMR_STATE_RUNNING;
    if (type == SYS_TMR_LINK_PERIODIC) {                            /* Determine when timer will expire                */
        ptmr->SysTmrMatch = ptmr->SysTmrPeriod + SysTmrTime;
    } else {
        if (ptmr->SysTmrDly == 0) {
            ptmr->SysTmrMatch = ptmr->SysTmrPeriod + SysTmrTime;
        } else {
            ptmr->SysTmrMatch = ptmr->SysTmrDly    + SysTmrTime;
        }
    }
    spoke  = (INT16U)(ptmr->SysTmrMatch % SYS_TMR_CFG_WHEEL_SIZE);
    pspoke = &SysTmrWheelTbl[spoke];

    if (pspoke->SysTmrFirst == (SYS_TMR *)0) {                       /* Link into timer wheel                           */
        pspoke->SysTmrFirst   = ptmr;
        ptmr->SysTmrNext      = (SYS_TMR *)0;
        pspoke->SysTmrEntries = 1;
    } else {
        ptmr1                = pspoke->SysTmrFirst;                 /* Point to first timer in the spoke               */
        pspoke->SysTmrFirst   = ptmr;
        ptmr->SysTmrNext      = (void *)ptmr1;
        ptmr1->SysTmrPrev     = (void *)ptmr;
        pspoke->SysTmrEntries++;
    }
    ptmr->SysTmrPrev = (void *)0;                                   /* Timer always inserted as first node in list     */
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                         REMOVE A TIMER FROM THE TIMER WHEEL
*
* Description: This function is called to remove the timer from the timer wheel.
*
* Arguments  : ptmr          Is a pointer to the timer to remove.
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_Unlink (SYS_TMR *ptmr)
{
    SYS_TMR        *ptmr1;
    SYS_TMR        *ptmr2;
    SYS_TMR_WHEEL  *pspoke;
    INT16U         spoke;


    spoke  = (INT16U)(ptmr->SysTmrMatch % SYS_TMR_CFG_WHEEL_SIZE);
    pspoke = &SysTmrWheelTbl[spoke];

    if (pspoke->SysTmrFirst == ptmr) {                       /* See if timer to remove is at the beginning of list     */
        ptmr1              = (SYS_TMR *)ptmr->SysTmrNext;
        pspoke->SysTmrFirst = (SYS_TMR *)ptmr1;
        if (ptmr1 != (SYS_TMR *)0) {
            ptmr1->SysTmrPrev = (void *)0;
        }
    } else {
        ptmr1            = (SYS_TMR *)ptmr->SysTmrPrev;       /* Remove timer from somewhere in the list                */
        ptmr2            = (SYS_TMR *)ptmr->SysTmrNext;
        ptmr1->SysTmrNext = ptmr2;
        if (ptmr2 != (SYS_TMR *)0) {
            ptmr2->SysTmrPrev = (void *)ptmr1;
        }
    }
    ptmr->SysTmrState = SYS_TMR_STATE_STOPPED;
    ptmr->SysTmrNext  = (void *)0;
    ptmr->SysTmrPrev  = (void *)0;
    pspoke->SysTmrEntries--;
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                       TIMER MANAGER DATA STRUCTURE LOCKING MECHANISM
*
* Description: These functions are used to gain exclusive access to timer management data structures.
*
* Arguments  : none
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_Lock (void)
{
    INT8U  err;


    SysSemPend(SysTmrSem, 0, &err);
    (void)err;
}
#endif



#if SYS_TMR_EN > 0
static  void  SysTmr_Unlock (void)
{
    (void)SysSemPost(SysTmrSem);
}
#endif

/*$PAGE*/
/*
************************************************************************************************************************
*                                                 TIMER MANAGEMENT TASK
*
* Description: This task is created by SysTmrInit().
*
* Arguments  : none
*
* Returns    : none
************************************************************************************************************************
*/

#if SYS_TMR_EN > 0
static  void  SysTmr_Task (void *p_arg)
{
    INT8U            err;
    SYS_TMR          *ptmr;
    SYS_TMR          *ptmr_next;
    SYS_TMR_CALLBACK  pfnct;
    SYS_TMR_WHEEL    *pspoke;
    INT16U           spoke;


    (void)p_arg;                                                 /* Not using 'p_arg', prevent compiler warning       */
    for (;;) {
        SysSemPend(SysTmrSemSignal, 0, &err);                      /* Wait for signal indicating time to update timers  */
        SysTmr_Lock();
        SysTmrTime++;                                             /* Increment the current time                        */
        spoke  = (INT16U)(SysTmrTime % SYS_TMR_CFG_WHEEL_SIZE);    /* Position on current timer wheel entry             */
        pspoke = &SysTmrWheelTbl[spoke];
        ptmr   = pspoke->SysTmrFirst;
        while (ptmr != (SYS_TMR *)0) {
            ptmr_next = (SYS_TMR *)ptmr->SysTmrNext;               /* Point to next timer to update because current ... */
                                                                 /* ... timer could get unlinked from the wheel.      */
            if (SysTmrTime == ptmr->SysTmrMatch) {                 /* Process each timer that expires                   */
                pfnct = ptmr->SysTmrCallback;                     /* Execute callback function if available            */
                if (pfnct != (SYS_TMR_CALLBACK)0) {
                    (*pfnct)((void *)ptmr, ptmr->SysTmrCallbackArg);
                }
                SysTmr_Unlink(ptmr);                              /* Remove from current wheel spoke                   */
                if (ptmr->SysTmrOpt == SYS_TMR_OPT_PERIODIC) {
                    SysTmr_Link(ptmr, SYS_TMR_LINK_PERIODIC);      /* Recalculate new position of timer in wheel        */
                } else {
                    ptmr->SysTmrState = SYS_TMR_STATE_COMPLETED;   /* Indicate that the timer has completed             */
                }
            }
            ptmr = ptmr_next;
        }
        SysTmr_Unlock();
    }
}
#endif
