/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                            TASK MANAGEMENT
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : SYS_TASK.C
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

/*$PAGE*/
/*
*********************************************************************************************************
*                                        CHANGE PRIORITY OF A TASK
*
* Description: This function allows you to change the priority of a task dynamically.  Note that the new
*              priority MUST be available.
*
* Arguments  : oldp     is the old priority
*
*              newp     is the new priority
*
* Returns    : SYS_ERR_NONE            is the call was successful
*              SYS_ERR_PRIO_INVALID    if the priority you specify is higher that the maximum allowed
*                                     (i.e. >= SYS_LOWEST_PRIO)
*              SYS_ERR_PRIO_EXIST      if the new priority already exist.
*              SYS_ERR_PRIO            there is no task with the specified OLD priority (i.e. the OLD task does
*                                     not exist.
*              SYS_ERR_TASK_NOT_EXIST  if the task is assigned to a Mutex PIP.
*********************************************************************************************************
*/

#if SYS_TASK_CHANGE_PRIO_EN > 0
INT8U  SysTaskChangePrio (INT8U oldprio, INT8U newprio)
{
#if (SYS_EVENT_EN)
    SYS_EVENT  *pevent;
#if (SYS_EVENT_MULTI_EN > 0)
    SYS_EVENT **pevents;
#endif
#endif
    SYS_TCB    *ptcb;
    INT8U      y_new;
    INT8U      x_new;
    INT8U      y_old;
#if SYS_LOWEST_PRIO <= 63
    INT8U      bity_new;
    INT8U      bitx_new;
    INT8U      bity_old;
    INT8U      bitx_old;
#else
    INT16U     bity_new;
    INT16U     bitx_new;
    INT16U     bity_old;
    INT16U     bitx_old;
#endif
#if SYS_CRITICAL_METHOD == 3
    SYS_CPU_SR  cpu_sr = 0;                                  /* Storage for CPU status register         */
#endif


/*$PAGE*/
#if SYS_ARG_CHK_EN > 0
    if (oldprio >= SYS_LOWEST_PRIO) {
        if (oldprio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
    if (newprio >= SYS_LOWEST_PRIO) {
        return (SYS_ERR_PRIO_INVALID);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (SysTCBPrioTbl[newprio] != (SYS_TCB *)0) {             /* New priority must not already exist     */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_PRIO_EXIST);
    }
    if (oldprio == SYS_PRIO_SELF) {                          /* See if changing self                    */
        oldprio = SysTCBCur->SysTCBPrio;                      /* Yes, get priority                       */
    }
    ptcb = SysTCBPrioTbl[oldprio];
    if (ptcb == (SYS_TCB *)0) {                              /* Does task to change exist?              */
        SYS_EXIT_CRITICAL();                                 /* No, can't change its priority!          */
        return (SYS_ERR_PRIO);
    }
    if (ptcb == SYS_TCB_RESERVED) {                          /* Is task assigned to Mutex               */
        SYS_EXIT_CRITICAL();                                 /* No, can't change its priority!          */
        return (SYS_ERR_TASK_NOT_EXIST);
    }
#if SYS_LOWEST_PRIO <= 63
    y_new                 = (INT8U)(newprio >> 3);          /* Yes, compute new TCB fields             */
    x_new                 = (INT8U)(newprio & 0x07);
    bity_new              = (INT8U)(1 << y_new);
    bitx_new              = (INT8U)(1 << x_new);
#else
    y_new                 = (INT8U)((newprio >> 4) & 0x0F);
    x_new                 = (INT8U)( newprio & 0x0F);
    bity_new              = (INT16U)(1 << y_new);
    bitx_new              = (INT16U)(1 << x_new);
#endif

    SysTCBPrioTbl[oldprio] = (SYS_TCB *)0;                    /* Remove TCB from old priority            */
    SysTCBPrioTbl[newprio] =  ptcb;                          /* Place pointer to TCB @ new priority     */
    y_old                 =  ptcb->SysTCBY;
    bity_old              =  ptcb->SysTCBBitY;
    bitx_old              =  ptcb->SysTCBBitX;
    if ((SysRdyTbl[y_old] &   bitx_old) != 0) {              /* If task is ready make it not            */
         SysRdyTbl[y_old] &= ~bitx_old;
         if (SysRdyTbl[y_old] == 0) {
             SysRdyGrp &= ~bity_old;
         }
         SysRdyGrp        |= bity_new;                       /* Make new priority ready to run          */
         SysRdyTbl[y_new] |= bitx_new;
    }

#if (SYS_EVENT_EN)
    pevent = ptcb->SysTCBEventPtr;
    if (pevent != (SYS_EVENT *)0) {
        pevent->SysEventTbl[y_old] &= ~bitx_old;             /* Remove old task prio from wait list     */
        if (pevent->SysEventTbl[y_old] == 0) {
            pevent->SysEventGrp    &= ~bity_old;
        }
        pevent->SysEventGrp        |= bity_new;              /* Add    new task prio to   wait list     */
        pevent->SysEventTbl[y_new] |= bitx_new;
    }
#if (SYS_EVENT_MULTI_EN > 0)
    if (ptcb->SysTCBEventMultiPtr != (SYS_EVENT **)0) {
        pevents =  ptcb->SysTCBEventMultiPtr;
        pevent  = *pevents;
        while (pevent != (SYS_EVENT *)0) {
            pevent->SysEventTbl[y_old] &= ~bitx_old;         /* Remove old task prio from wait lists    */
            if (pevent->SysEventTbl[y_old] == 0) {
                pevent->SysEventGrp    &= ~bity_old;
            }
            pevent->SysEventGrp        |= bity_new;          /* Add    new task prio to   wait lists    */
            pevent->SysEventTbl[y_new] |= bitx_new;
            pevents++;
            pevent                     = *pevents;
        }
    }
#endif
#endif

    ptcb->SysTCBPrio = newprio;                              /* Set new task priority                   */
    ptcb->SysTCBY    = y_new;
    ptcb->SysTCBX    = x_new;
    ptcb->SysTCBBitY = bity_new;
    ptcb->SysTCBBitX = bitx_new;
    SYS_EXIT_CRITICAL();
    if (SysRunning == SYS_TRUE) {
        SYS_Sched();                                         /* Find new highest priority task          */
    }
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            CREATE A TASK
*
* Description: This function is used to have uC/Sys-II manage the execution of a task.  Tasks can either
*              be created prior to the start of multitasking or by a running task.  A task cannot be
*              created by an ISR.
*
* Arguments  : task     is a pointer to the task's code
*
*              p_arg    is a pointer to an optional data area which can be used to pass parameters to
*                       the task when the task first executes.  Where the task is concerned it thinks
*                       it was invoked and passed the argument 'p_arg' as follows:
*
*                           void Task (void *p_arg)
*                           {
*                               for (;;) {
*                                   Task code;
*                               }
*                           }
*
*              ptos     is a pointer to the task's top of stack.  If the configuration constant
*                       SYS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pstk' will thus point to the highest (valid) memory
*                       location of the stack.  If SYS_STK_GROWTH is set to 0, 'pstk' will point to the
*                       lowest memory location of the stack and the stack will grow with increasing
*                       memory locations.
*
*              prio     is the task's priority.  A unique priority MUST be assigned to each task and the
*                       lower the number, the higher the priority.
*
* Returns    : SYS_ERR_NONE             if the function was successful.
*              SYS_PRIO_EXIT            if the task priority already exist
*                                      (each task MUST have a unique priority).
*              SYS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                      (i.e. >= SYS_LOWEST_PRIO)
*              SYS_ERR_TASK_CREATE_ISR  if you tried to create a task from an ISR.
*********************************************************************************************************
*/

#if SYS_TASK_CREATE_EN > 0
INT8U  SysTaskCreate (void (*task)(void *p_arg), void *p_arg, SYS_STK *ptos, INT8U prio)
{
    SYS_STK    *psp;
    INT8U      err;
#if SYS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio > SYS_LOWEST_PRIO) {             /* Make sure priority is within allowable range           */
        return (SYS_ERR_PRIO_INVALID);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (SysIntNesting > 0) {                  /* Make sure we don't create the task from within an ISR  */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_CREATE_ISR);
    }
    if (SysTCBPrioTbl[prio] == (SYS_TCB *)0) { /* Make sure task doesn't already exist at this priority  */
        SysTCBPrioTbl[prio] = SYS_TCB_RESERVED;/* Reserve the priority to prevent others from doing ...  */
                                             /* ... the same thing until task is created.              */
        SYS_EXIT_CRITICAL();
        psp = SysTaskStkInit(task, p_arg, ptos, 0);              /* Initialize the task's stack         */
        err = SYS_TCBInit(prio, psp, (SYS_STK *)0, 0, 0, (void *)0, 0);
        if (err == SYS_ERR_NONE) {
            if (SysRunning == SYS_TRUE) {      /* Find highest priority task if multitasking has started */
                SYS_Sched();
            }
        } else {
            SYS_ENTER_CRITICAL();
            SysTCBPrioTbl[prio] = (SYS_TCB *)0;/* Make this priority available to others                 */
            SYS_EXIT_CRITICAL();
        }
        return (err);
    }
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_PRIO_EXIST);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                     CREATE A TASK (Extended Version)
*
* Description: This function is used to have uC/Sys-II manage the execution of a task.  Tasks can either
*              be created prior to the start of multitasking or by a running task.  A task cannot be
*              created by an ISR.  This function is similar to SysTaskCreate() except that it allows
*              additional information about a task to be specified.
*
* Arguments  : task      is a pointer to the task's code
*
*              p_arg     is a pointer to an optional data area which can be used to pass parameters to
*                        the task when the task first executes.  Where the task is concerned it thinks
*                        it was invoked and passed the argument 'p_arg' as follows:
*
*                            void Task (void *p_arg)
*                            {
*                                for (;;) {
*                                    Task code;
*                                }
*                            }
*
*              ptos      is a pointer to the task's top of stack.  If the configuration constant
*                        SYS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                        memory to low memory).  'ptos' will thus point to the highest (valid) memory
*                        location of the stack.  If SYS_STK_GROWTH is set to 0, 'ptos' will point to the
*                        lowest memory location of the stack and the stack will grow with increasing
*                        memory locations.  'ptos' MUST point to a valid 'free' data item.
*
*              prio      is the task's priority.  A unique priority MUST be assigned to each task and the
*                        lower the number, the higher the priority.
*
*              id        is the task's ID (0..65535)
*
*              pbos      is a pointer to the task's bottom of stack.  If the configuration constant
*                        SYS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                        memory to low memory).  'pbos' will thus point to the LOWEST (valid) memory
*                        location of the stack.  If SYS_STK_GROWTH is set to 0, 'pbos' will point to the
*                        HIGHEST memory location of the stack and the stack will grow with increasing
*                        memory locations.  'pbos' MUST point to a valid 'free' data item.
*
*              stk_size  is the size of the stack in number of elements.  If SYS_STK is set to INT8U,
*                        'stk_size' corresponds to the number of bytes available.  If SYS_STK is set to
*                        INT16U, 'stk_size' contains the number of 16-bit entries available.  Finally, if
*                        SYS_STK is set to INT32U, 'stk_size' contains the number of 32-bit entries
*                        available on the stack.
*
*              pext      is a pointer to a user supplied memory location which is used as a TCB extension.
*                        For example, this user memory can hold the contents of floating-point registers
*                        during a context switch, the time each task takes to execute, the number of times
*                        the task has been switched-in, etc.
*
*              opt       contains additional information (or options) about the behavior of the task.  The
*                        LOWER 8-bits are reserved by uC/Sys-II while the upper 8 bits can be application
*                        specific.  See SYS_TASK_OPT_??? in uCSys-II.H.  Current choices are:
*
*                        SYS_TASK_OPT_STK_CHK      Stack checking to be allowed for the task
*                        SYS_TASK_OPT_STK_CLR      Clear the stack when the task is created
*                        SYS_TASK_OPT_SAVE_FP      If the CPU has floating-point registers, save them
*                                                 during a context switch.
*
* Returns    : SYS_ERR_NONE             if the function was successful.
*              SYS_PRIO_EXIT            if the task priority already exist
*                                      (each task MUST have a unique priority).
*              SYS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                      (i.e. > SYS_LOWEST_PRIO)
*              SYS_ERR_TASK_CREATE_ISR  if you tried to create a task from an ISR.
*********************************************************************************************************
*/
/*$PAGE*/
#if SYS_TASK_CREATE_EXT_EN > 0
INT8U  SysTaskCreateExt (void   (*task)(void *p_arg),
                        void    *p_arg,
                        SYS_STK  *ptos,
                        INT8U    prio,
                        INT16U   id,
                        SYS_STK  *pbos,
                        INT32U   stk_size,
                        void    *pext,
                        INT16U   opt)
{
    SYS_STK    *psp;
    INT8U      err;
#if SYS_CRITICAL_METHOD == 3                  /* Allocate storage for CPU status register               */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio > SYS_LOWEST_PRIO) {             /* Make sure priority is within allowable range           */
        return (SYS_ERR_PRIO_INVALID);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (SysIntNesting > 0) {                  /* Make sure we don't create the task from within an ISR  */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_CREATE_ISR);
    }
    if (SysTCBPrioTbl[prio] == (SYS_TCB *)0) { /* Make sure task doesn't already exist at this priority  */
        SysTCBPrioTbl[prio] = SYS_TCB_RESERVED;/* Reserve the priority to prevent others from doing ...  */
                                             /* ... the same thing until task is created.              */
        SYS_EXIT_CRITICAL();

#if (SYS_TASK_STAT_STK_CHK_EN > 0)
        SYS_TaskStkClr(pbos, stk_size, opt);                    /* Clear the task stack (if needed)     */
#endif

        psp = SysTaskStkInit(task, p_arg, ptos, opt);           /* Initialize the task's stack          */
        err = SYS_TCBInit(prio, psp, pbos, id, stk_size, pext, opt);
        if (err == SYS_ERR_NONE) {
            if (SysRunning == SYS_TRUE) {                        /* Find HPT if multitasking has started */
                SYS_Sched();
            }
        } else {
            SYS_ENTER_CRITICAL();
            SysTCBPrioTbl[prio] = (SYS_TCB *)0;                  /* Make this priority avail. to others  */
            SYS_EXIT_CRITICAL();
        }
        return (err);
    }
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_PRIO_EXIST);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            DELETE A TASK
*
* Description: This function allows you to delete a task.  The calling task can delete itself by
*              its own priority number.  The deleted task is returned to the dormant state and can be
*              re-activated by creating the deleted task again.
*
* Arguments  : prio    is the priority of the task to delete.  Note that you can explicitely delete
*                      the current task without knowing its priority level by setting 'prio' to
*                      SYS_PRIO_SELF.
*
* Returns    : SYS_ERR_NONE             if the call is successful
*              SYS_ERR_TASK_DEL_IDLE    if you attempted to delete uC/Sys-II's idle task
*              SYS_ERR_PRIO_INVALID     if the priority you specify is higher that the maximum allowed
*                                      (i.e. >= SYS_LOWEST_PRIO) or, you have not specified SYS_PRIO_SELF.
*              SYS_ERR_TASK_DEL         if the task is assigned to a Mutex PIP.   
*              SYS_ERR_TASK_NOT_EXIST   if the task you want to delete does not exist.
*              SYS_ERR_TASK_DEL_ISR     if you tried to delete a task from an ISR
*
* Notes      : 1) To reduce interrupt latency, SysTaskDel() 'disables' the task:
*                    a) by making it not ready
*                    b) by removing it from any wait lists
*                    c) by preventing SysTimeTick() from making the task ready to run.
*                 The task can then be 'unlinked' from the miscellaneous structures in uC/Sys-II.
*              2) The function SYS_Dummy() is called after SYS_EXIT_CRITICAL() because, on most processors,
*                 the next instruction following the enable interrupt instruction is ignored.
*              3) An ISR cannot delete a task.
*              4) The lock nesting counter is incremented because, for a brief instant, if the current
*                 task is being deleted, the current task would not be able to be rescheduled because it
*                 is removed from the ready list.  Incrementing the nesting counter prevents another task
*                 from being schedule.  This means that an ISR would return to the current task which is
*                 being deleted.  The rest of the deletion would thus be able to be completed.
*********************************************************************************************************
*/

#if SYS_TASK_DEL_EN > 0
INT8U  SysTaskDel (INT8U prio)
{
#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
    SYS_FLAG_NODE *pnode;
#endif
    SYS_TCB       *ptcb;
#if SYS_CRITICAL_METHOD == 3                             /* Allocate storage for CPU status register    */
    SYS_CPU_SR     cpu_sr = 0;
#endif



    if (SysIntNesting > 0) {                             /* See if trying to delete from ISR            */
        return (SYS_ERR_TASK_DEL_ISR);
    }
    if (prio == SYS_TASK_IDLE_PRIO) {                    /* Not allowed to delete idle task             */
        return (SYS_ERR_TASK_DEL_IDLE);
    }
#if SYS_ARG_CHK_EN > 0
    if (prio >= SYS_LOWEST_PRIO) {                       /* Task priority valid ?                       */
        if (prio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
#endif

/*$PAGE*/
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                         /* See if requesting to delete self            */
        prio = SysTCBCur->SysTCBPrio;                     /* Set priority to delete to current           */
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                          /* Task to delete must exist                   */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
    if (ptcb == SYS_TCB_RESERVED) {                      /* Must not be assigned to Mutex               */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_DEL);
    }

    SysRdyTbl[ptcb->SysTCBY] &= ~ptcb->SysTCBBitX;
    if (SysRdyTbl[ptcb->SysTCBY] == 0) {                  /* Make task not ready                         */
        SysRdyGrp           &= ~ptcb->SysTCBBitY;
    }
    
#if (SYS_EVENT_EN)
    if (ptcb->SysTCBEventPtr != (SYS_EVENT *)0) {
        SYS_EventTaskRemove(ptcb, ptcb->SysTCBEventPtr);  /* Remove this task from any event   wait list */
    }
#if (SYS_EVENT_MULTI_EN > 0)
    if (ptcb->SysTCBEventMultiPtr != (SYS_EVENT **)0) {   /* Remove this task from any events' wait lists*/
        SYS_EventTaskRemoveMulti(ptcb, ptcb->SysTCBEventMultiPtr);
    }
#endif
#endif

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
    pnode = ptcb->SysTCBFlagNode;
    if (pnode != (SYS_FLAG_NODE *)0) {                   /* If task is waiting on event flag            */
        SYS_FlagUnlink(pnode);                           /* Remove from wait list                       */
    }
#endif

    ptcb->SysTCBDly      = 0;                            /* Prevent SysTimeTick() from updating          */
    ptcb->SysTCBStat     = SYS_STAT_RDY;                  /* Prevent task from being resumed             */
    ptcb->SysTCBStatPend = SYS_STAT_PEND_OK;
    if (SysLockNesting < 255u) {                         /* Make sure we don't context switch           */
        SysLockNesting++;
    }
    SYS_EXIT_CRITICAL();                                 /* Enabling INT. ignores next instruc.         */
    SYS_Dummy();                                         /* ... Dummy ensures that INTs will be         */
    SYS_ENTER_CRITICAL();                                /* ... disabled HERE!                          */
    if (SysLockNesting > 0) {                            /* Remove context switch lock                  */
        SysLockNesting--;
    }
    SysTaskDelHook(ptcb);                                /* Call user defined hook                      */
    SysTaskCtr--;                                        /* One less task being managed                 */
    SysTCBPrioTbl[prio] = (SYS_TCB *)0;                   /* Clear old priority entry                    */
    if (ptcb->SysTCBPrev == (SYS_TCB *)0) {               /* Remove from TCB chain                       */
        ptcb->SysTCBNext->SysTCBPrev = (SYS_TCB *)0;
        SysTCBList                  = ptcb->SysTCBNext;
    } else {
        ptcb->SysTCBPrev->SysTCBNext = ptcb->SysTCBNext;
        ptcb->SysTCBNext->SysTCBPrev = ptcb->SysTCBPrev;
    }
    ptcb->SysTCBNext   = SysTCBFreeList;                  /* Return TCB to free TCB list                 */
    SysTCBFreeList     = ptcb;
#if SYS_TASK_NAME_SIZE > 1
    ptcb->SysTCBTaskName[0] = '?';                       /* Unknown name                                */
    ptcb->SysTCBTaskName[1] = SYS_ASCII_NUL;
#endif
    SYS_EXIT_CRITICAL();
    if (SysRunning == SYS_TRUE) {
        SYS_Sched();                                     /* Find new highest priority task              */
    }
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                    REQUEST THAT A TASK DELETE ITSELF
*
* Description: This function is used to:
*                   a) notify a task to delete itself.
*                   b) to see if a task requested that the current task delete itself.
*              This function is a little tricky to understand.  Basically, you have a task that needs
*              to be deleted however, this task has resources that it has allocated (memory buffers,
*              semaphores, mailboxes, queues etc.).  The task cannot be deleted otherwise these
*              resources would not be freed.  The requesting task calls SysTaskDelReq() to indicate that
*              the task needs to be deleted.  Deleting of the task is however, deferred to the task to
*              be deleted.  For example, suppose that task #10 needs to be deleted.  The requesting task
*              example, task #5, would call SysTaskDelReq(10).  When task #10 gets to execute, it calls
*              this function by specifying SYS_PRIO_SELF and monitors the returned value.  If the return
*              value is SYS_ERR_TASK_DEL_REQ, another task requested a task delete.  Task #10 would look like
*              this:
*
*                   void Task(void *p_arg)
*                   {
*                       .
*                       .
*                       while (1) {
*                           SysTimeDly(1);
*                           if (SysTaskDelReq(SYS_PRIO_SELF) == SYS_ERR_TASK_DEL_REQ) {
*                               Release any owned resources;
*                               De-allocate any dynamic memory;
*                               SysTaskDel(SYS_PRIO_SELF);
*                           }
*                       }
*                   }
*
* Arguments  : prio    is the priority of the task to request the delete from
*
* Returns    : SYS_ERR_NONE            if the task exist and the request has been registered
*              SYS_ERR_TASK_NOT_EXIST  if the task has been deleted.  This allows the caller to know whether
*                                     the request has been executed.
*              SYS_ERR_TASK_DEL        if the task is assigned to a Mutex.
*              SYS_ERR_TASK_DEL_IDLE   if you requested to delete uC/Sys-II's idle task
*              SYS_ERR_PRIO_INVALID    if the priority you specify is higher that the maximum allowed
*                                     (i.e. >= SYS_LOWEST_PRIO) or, you have not specified SYS_PRIO_SELF.
*              SYS_ERR_TASK_DEL_REQ    if a task (possibly another task) requested that the running task be
*                                     deleted.
*********************************************************************************************************
*/
/*$PAGE*/
#if SYS_TASK_DEL_EN > 0
INT8U  SysTaskDelReq (INT8U prio)
{
    INT8U      stat;
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



    if (prio == SYS_TASK_IDLE_PRIO) {                            /* Not allowed to delete idle task     */
        return (SYS_ERR_TASK_DEL_IDLE);
    }
#if SYS_ARG_CHK_EN > 0
    if (prio >= SYS_LOWEST_PRIO) {                               /* Task priority valid ?               */
        if (prio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
#endif
    if (prio == SYS_PRIO_SELF) {                                 /* See if a task is requesting to ...  */
        SYS_ENTER_CRITICAL();                                    /* ... this task to delete itself      */
        stat = SysTCBCur->SysTCBDelReq;                           /* Return request status to caller     */
        SYS_EXIT_CRITICAL();
        return (stat);
    }
    SYS_ENTER_CRITICAL();
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                                  /* Task to delete must exist           */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);                         /* Task must already be deleted        */
    }
    if (ptcb == SYS_TCB_RESERVED) {                              /* Must NOT be assigned to a Mutex     */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_DEL);
    }
    ptcb->SysTCBDelReq = SYS_ERR_TASK_DEL_REQ;                    /* Set flag indicating task to be DEL. */
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                        GET THE NAME OF A TASK
*
* Description: This function is called to obtain the name of a task.
*
* Arguments  : prio      is the priority of the task that you want to obtain the name from.
*
*              pname     is a pointer to an ASCII string that will receive the name of the task.  The
*                        string must be able to hold at least SYS_TASK_NAME_SIZE characters.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the requested task is resumed
*                        SYS_ERR_TASK_NOT_EXIST      if the task has not been created or is assigned to a Mutex
*                        SYS_ERR_PRIO_INVALID        if you specified an invalid priority:
*                                                   A higher value than the idle task or not SYS_PRIO_SELF.
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_NAME_GET_ISR        You called this function from an ISR
*                        
*
* Returns    : The length of the string or 0 if the task does not exist.
*********************************************************************************************************
*/

#if SYS_TASK_NAME_SIZE > 1
INT8U  SysTaskNameGet (INT8U prio, INT8U *pname, INT8U *perr)
{
    SYS_TCB    *ptcb;
    INT8U      len;
#if SYS_CRITICAL_METHOD == 3                              /* Allocate storage for CPU status register   */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                            /* Validate 'perr'                            */
        return (0);
    }
    if (prio > SYS_LOWEST_PRIO) {                         /* Task priority valid ?                      */
        if (prio != SYS_PRIO_SELF) {
            *perr = SYS_ERR_PRIO_INVALID;                 /* No                                         */
            return (0);
        }
    }
    if (pname == (INT8U *)0) {                           /* Is 'pname' a NULL pointer?                 */
        *perr = SYS_ERR_PNAME_NULL;                       /* Yes                                        */
        return (0);
    }
#endif
    if (SysIntNesting > 0) {                              /* See if trying to call from an ISR          */
        *perr = SYS_ERR_NAME_GET_ISR;
        return (0);
    }
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                          /* See if caller desires it's own name        */
        prio = SysTCBCur->SysTCBPrio;
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                           /* Does task exist?                           */
        SYS_EXIT_CRITICAL();                              /* No                                         */
        *perr = SYS_ERR_TASK_NOT_EXIST;
        return (0);
    }
    if (ptcb == SYS_TCB_RESERVED) {                       /* Task assigned to a Mutex?                  */
        SYS_EXIT_CRITICAL();                              /* Yes                                        */
        *perr = SYS_ERR_TASK_NOT_EXIST;
        return (0);
    }
    len   = SYS_StrCopy(pname, ptcb->SysTCBTaskName);      /* Yes, copy name from TCB                    */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
    return (len);
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        ASSIGN A NAME TO A TASK
*
* Description: This function is used to set the name of a task.
*
* Arguments  : prio      is the priority of the task that you want the assign a name to.
*
*              pname     is a pointer to an ASCII string that contains the name of the task.  The ASCII
*                        string must be NUL terminated.
*
*              perr       is a pointer to an error code that can contain one of the following values:
*
*                        SYS_ERR_NONE                if the requested task is resumed
*                        SYS_ERR_TASK_NOT_EXIST      if the task has not been created or is assigned to a Mutex
*                        SYS_ERR_TASK_NAME_TOO_LONG  if the name you are giving to the task exceeds the
*                                                   storage capacity of a task name as specified by
*                                                   SYS_TASK_NAME_SIZE.
*                        SYS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        SYS_ERR_PRIO_INVALID        if you specified an invalid priority:
*                                                   A higher value than the idle task or not SYS_PRIO_SELF.
*                        SYS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/
#if SYS_TASK_NAME_SIZE > 1
void  SysTaskNameSet (INT8U prio, INT8U *pname, INT8U *perr)
{
    INT8U      len;
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                          /* Allocate storage for CPU status register       */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (perr == (INT8U *)0) {                        /* Validate 'perr'                                */
        return;
    }
    if (prio > SYS_LOWEST_PRIO) {                     /* Task priority valid ?                          */
        if (prio != SYS_PRIO_SELF) {
            *perr = SYS_ERR_PRIO_INVALID;             /* No                                             */
            return;
        }
    }
    if (pname == (INT8U *)0) {                       /* Is 'pname' a NULL pointer?                     */
        *perr = SYS_ERR_PNAME_NULL;                   /* Yes                                            */
        return;
    }
#endif
    if (SysIntNesting > 0) {                          /* See if trying to call from an ISR              */
        *perr = SYS_ERR_NAME_SET_ISR;
        return;
    }
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                      /* See if caller desires to set it's own name     */
        prio = SysTCBCur->SysTCBPrio;
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                       /* Does task exist?                               */
        SYS_EXIT_CRITICAL();                          /* No                                             */
        *perr = SYS_ERR_TASK_NOT_EXIST;
        return;
    }
    if (ptcb == SYS_TCB_RESERVED) {                   /* Task assigned to a Mutex?                      */
        SYS_EXIT_CRITICAL();                          /* Yes                                            */
        *perr = SYS_ERR_TASK_NOT_EXIST;
        return;
    }
    len = SYS_StrLen(pname);                          /* Yes, Can we fit the string in the TCB?         */
    if (len > (SYS_TASK_NAME_SIZE - 1)) {             /*      No                                        */
        SYS_EXIT_CRITICAL();
        *perr = SYS_ERR_TASK_NAME_TOO_LONG;
        return;
    }
    (void)SYS_StrCopy(ptcb->SysTCBTaskName, pname);    /*      Yes, copy to TCB                          */
    SYS_EXIT_CRITICAL();
    *perr = SYS_ERR_NONE;
}
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                        RESUME A SUSPENDED TASK
*
* Description: This function is called to resume a previously suspended task.  This is the only call that
*              will remove an explicit task suspension.
*
* Arguments  : prio     is the priority of the task to resume.
*
* Returns    : SYS_ERR_NONE                if the requested task is resumed
*              SYS_ERR_PRIO_INVALID        if the priority you specify is higher that the maximum allowed
*                                         (i.e. >= SYS_LOWEST_PRIO)
*              SYS_ERR_TASK_RESUME_PRIO    if the task to resume does not exist
*              SYS_ERR_TASK_NOT_EXIST      if the task is assigned to a Mutex PIP
*              SYS_ERR_TASK_NOT_SUSPENDED  if the task to resume has not been suspended
*********************************************************************************************************
*/

#if SYS_TASK_SUSPEND_EN > 0
INT8U  SysTaskResume (INT8U prio)
{
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                                   /* Storage for CPU status register       */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio >= SYS_LOWEST_PRIO) {                             /* Make sure task priority is valid      */
        return (SYS_ERR_PRIO_INVALID);
    }
#endif
    SYS_ENTER_CRITICAL();
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                                /* Task to suspend must exist            */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_RESUME_PRIO);
    }
    if (ptcb == SYS_TCB_RESERVED) {                            /* See if assigned to Mutex              */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
    if ((ptcb->SysTCBStat & SYS_STAT_SUSPEND) != SYS_STAT_RDY) { /* Task must be suspended                */
        ptcb->SysTCBStat &= ~(INT8U)SYS_STAT_SUSPEND;           /* Remove suspension                     */
        if (ptcb->SysTCBStat == SYS_STAT_RDY) {                 /* See if task is now ready              */
            if (ptcb->SysTCBDly == 0) {
                SysRdyGrp               |= ptcb->SysTCBBitY;    /* Yes, Make task ready to run           */
                SysRdyTbl[ptcb->SysTCBY] |= ptcb->SysTCBBitX;
                SYS_EXIT_CRITICAL();
                if (SysRunning == SYS_TRUE) {
                    SYS_Sched();                               /* Find new highest priority task        */
                }
            } else {
                SYS_EXIT_CRITICAL();
            }
        } else {                                              /* Must be pending on event              */
            SYS_EXIT_CRITICAL();
        }
        return (SYS_ERR_NONE);
    }
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_TASK_NOT_SUSPENDED);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                             STACK CHECKING
*
* Description: This function is called to check the amount of free memory left on the specified task's
*              stack.
*
* Arguments  : prio          is the task priority
*
*              p_stk_data    is a pointer to a data structure of type SYS_STK_DATA.
*
* Returns    : SYS_ERR_NONE            upon success
*              SYS_ERR_PRIO_INVALID    if the priority you specify is higher that the maximum allowed
*                                     (i.e. > SYS_LOWEST_PRIO) or, you have not specified SYS_PRIO_SELF.
*              SYS_ERR_TASK_NOT_EXIST  if the desired task has not been created or is assigned to a Mutex PIP
*              SYS_ERR_TASK_OPT        if you did NOT specified SYS_TASK_OPT_STK_CHK when the task was created
*              SYS_ERR_PDATA_NULL      if 'p_stk_data' is a NULL pointer
*********************************************************************************************************
*/
#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
INT8U  SysTaskStkChk (INT8U prio, SYS_STK_DATA *p_stk_data)
{
    SYS_TCB    *ptcb;
    SYS_STK    *pchk;
    INT32U     nfree;
    INT32U     size;
#if SYS_CRITICAL_METHOD == 3                            /* Allocate storage for CPU status register     */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio > SYS_LOWEST_PRIO) {                       /* Make sure task priority is valid             */
        if (prio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
    if (p_stk_data == (SYS_STK_DATA *)0) {              /* Validate 'p_stk_data'                        */
        return (SYS_ERR_PDATA_NULL);
    }
#endif
    p_stk_data->SysFree = 0;                            /* Assume failure, set to 0 size                */
    p_stk_data->SysUsed = 0;
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                        /* See if check for SELF                        */
        prio = SysTCBCur->SysTCBPrio;
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                         /* Make sure task exist                         */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
    if (ptcb == SYS_TCB_RESERVED) {
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
    if ((ptcb->SysTCBOpt & SYS_TASK_OPT_STK_CHK) == 0) { /* Make sure stack checking option is set       */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_OPT);
    }
    nfree = 0;
    size  = ptcb->SysTCBStkSize;
    pchk  = ptcb->SysTCBStkBottom;
    SYS_EXIT_CRITICAL();
#if SYS_STK_GROWTH == 1
    while (*pchk++ == (SYS_STK)0) {                    /* Compute the number of zero entries on the stk */
        nfree++;
    }
#else
    while (*pchk-- == (SYS_STK)0) {
        nfree++;
    }
#endif
    p_stk_data->SysFree = nfree * sizeof(SYS_STK);          /* Compute number of free bytes on the stack */
    p_stk_data->SysUsed = (size - nfree) * sizeof(SYS_STK); /* Compute number of bytes used on the stack */
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            SUSPEND A TASK
*
* Description: This function is called to suspend a task.  The task can be the calling task if the
*              priority passed to SysTaskSuspend() is the priority of the calling task or SYS_PRIO_SELF.
*
* Arguments  : prio     is the priority of the task to suspend.  If you specify SYS_PRIO_SELF, the
*                       calling task will suspend itself and rescheduling will occur.
*
* Returns    : SYS_ERR_NONE               if the requested task is suspended
*              SYS_ERR_TASK_SUSPEND_IDLE  if you attempted to suspend the idle task which is not allowed.
*              SYS_ERR_PRIO_INVALID       if the priority you specify is higher that the maximum allowed
*                                        (i.e. >= SYS_LOWEST_PRIO) or, you have not specified SYS_PRIO_SELF.
*              SYS_ERR_TASK_SUSPEND_PRIO  if the task to suspend does not exist
*              SYS_ERR_TASK_NOT_EXITS     if the task is assigned to a Mutex PIP
*
* Note       : You should use this function with great care.  If you suspend a task that is waiting for
*              an event (i.e. a message, a semaphore, a queue ...) you will prevent this task from
*              running when the event arrives.
*********************************************************************************************************
*/

#if SYS_TASK_SUSPEND_EN > 0
INT8U  SysTaskSuspend (INT8U prio)
{
    BOOLEAN    self;
    SYS_TCB    *ptcb;
    INT8U      y;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio == SYS_TASK_IDLE_PRIO) {                            /* Not allowed to suspend idle task    */
        return (SYS_ERR_TASK_SUSPEND_IDLE);
    }
    if (prio >= SYS_LOWEST_PRIO) {                               /* Task priority valid ?               */
        if (prio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
#endif
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                                 /* See if suspend SELF                 */
        prio = SysTCBCur->SysTCBPrio;
        self = SYS_TRUE;
    } else if (prio == SysTCBCur->SysTCBPrio) {                   /* See if suspending self              */
        self = SYS_TRUE;
    } else {
        self = SYS_FALSE;                                        /* No suspending another task          */
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                                  /* Task to suspend must exist          */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_SUSPEND_PRIO);
    }
    if (ptcb == SYS_TCB_RESERVED) {                              /* See if assigned to Mutex            */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
    y            = ptcb->SysTCBY;
    SysRdyTbl[y] &= ~ptcb->SysTCBBitX;                            /* Make task not ready                 */
    if (SysRdyTbl[y] == 0) {
        SysRdyGrp &= ~ptcb->SysTCBBitY;
    }
    ptcb->SysTCBStat |= SYS_STAT_SUSPEND;                         /* Status of task is 'SUSPENDED'       */
    SYS_EXIT_CRITICAL();
    if (self == SYS_TRUE) {                                      /* Context switch only if SELF         */
        SYS_Sched();                                             /* Find new highest priority task      */
    }
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                            QUERY A TASK
*
* Description: This function is called to obtain a copy of the desired task's TCB.
*
* Arguments  : prio         is the priority of the task to obtain information from.
*
*              p_task_data  is a pointer to where the desired task's SYS_TCB will be stored.
*
* Returns    : SYS_ERR_NONE            if the requested task is suspended
*              SYS_ERR_PRIO_INVALID    if the priority you specify is higher that the maximum allowed
*                                     (i.e. > SYS_LOWEST_PRIO) or, you have not specified SYS_PRIO_SELF.
*              SYS_ERR_PRIO            if the desired task has not been created
*              SYS_ERR_TASK_NOT_EXIST  if the task is assigned to a Mutex PIP
*              SYS_ERR_PDATA_NULL      if 'p_task_data' is a NULL pointer
*********************************************************************************************************
*/

#if SYS_TASK_QUERY_EN > 0
INT8U  SysTaskQuery (INT8U prio, SYS_TCB *p_task_data)
{
    SYS_TCB    *ptcb;
#if SYS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    SYS_CPU_SR  cpu_sr = 0;
#endif



#if SYS_ARG_CHK_EN > 0
    if (prio > SYS_LOWEST_PRIO) {                 /* Task priority valid ?                              */
        if (prio != SYS_PRIO_SELF) {
            return (SYS_ERR_PRIO_INVALID);
        }
    }
    if (p_task_data == (SYS_TCB *)0) {            /* Validate 'p_task_data'                             */
        return (SYS_ERR_PDATA_NULL);
    }
#endif
    SYS_ENTER_CRITICAL();
    if (prio == SYS_PRIO_SELF) {                  /* See if suspend SELF                                */
        prio = SysTCBCur->SysTCBPrio;
    }
    ptcb = SysTCBPrioTbl[prio];
    if (ptcb == (SYS_TCB *)0) {                   /* Task to query must exist                           */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_PRIO);
    }
    if (ptcb == SYS_TCB_RESERVED) {               /* Task to query must not be assigned to a Mutex      */
        SYS_EXIT_CRITICAL();
        return (SYS_ERR_TASK_NOT_EXIST);
    }
                                                 /* Copy TCB into user storage area                    */
    SYS_MemCopy((INT8U *)p_task_data, (INT8U *)ptcb, sizeof(SYS_TCB));
    SYS_EXIT_CRITICAL();
    return (SYS_ERR_NONE);
}
#endif
/*$PAGE*/
/*
*********************************************************************************************************
*                                        CLEAR TASK STACK
*
* Description: This function is used to clear the stack of a task (i.e. write all zeros)
*
* Arguments  : pbos     is a pointer to the task's bottom of stack.  If the configuration constant
*                       SYS_STK_GROWTH is set to 1, the stack is assumed to grow downward (i.e. from high
*                       memory to low memory).  'pbos' will thus point to the lowest (valid) memory
*                       location of the stack.  If SYS_STK_GROWTH is set to 0, 'pbos' will point to the
*                       highest memory location of the stack and the stack will grow with increasing
*                       memory locations.  'pbos' MUST point to a valid 'free' data item.
*
*              size     is the number of 'stack elements' to clear.
*
*              opt      contains additional information (or options) about the behavior of the task.  The
*                       LOWER 8-bits are reserved by uC/Sys-II while the upper 8 bits can be application
*                       specific.  See SYS_TASK_OPT_??? in uCSys-II.H.
*
* Returns    : none
*********************************************************************************************************
*/
#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
void  SYS_TaskStkClr (SYS_STK *pbos, INT32U size, INT16U opt)
{
    if ((opt & SYS_TASK_OPT_STK_CHK) != 0x0000) {       /* See if stack checking has been enabled       */
        if ((opt & SYS_TASK_OPT_STK_CLR) != 0x0000) {   /* See if stack needs to be cleared             */
#if SYS_STK_GROWTH == 1
            while (size > 0) {                         /* Stack grows from HIGH to LOW memory          */
                size--;
                *pbos++ = (SYS_STK)0;                   /* Clear from bottom of stack and up!           */
            }
#else
            while (size > 0) {                         /* Stack grows from LOW to HIGH memory          */
                size--;
                *pbos-- = (SYS_STK)0;                   /* Clear from bottom of stack and down          */
            }
#endif
        }
    }
}

#endif
