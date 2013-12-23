/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*                                  uC/Sys-II Configuration File for V2.8x
*
*                               (c) Copyright 2005-2007, Micrium, Weston, FL
*                                          All Rights Reserved
*
*
* File    : SYS_CFG.H
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

#ifndef SYS_CFG_H
#define SYS_CFG_H


                                       /* ---------------------- MISCELLANEOUS ----------------------- */
#define SYS_APP_HOOKS_EN           1    /* Application-defined hooks are called from the uC/Sys-II hooks */
#define SYS_ARG_CHK_EN             1    /* Enable (1) or Disable (0) argument checking                  */
#define SYS_CPU_HOOKS_EN           1    /* uC/Sys-II hooks are found in the processor port files         */

#define SYS_DEBUG_EN               1    /* Enable(1) debug variables                                    */

#define SYS_EVENT_MULTI_EN         1    /* Include code for SysEventPendMulti()                          */
#define SYS_EVENT_NAME_SIZE       16    /* Determine the size of the name of a Sem, Mutex, Mbox or Q    */

#define SYS_LOWEST_PRIO           40    /* Defines the lowest priority that can be assigned ...         */
                                       /* ... MUST NEVER be higher than 254!                           */

#define SYS_MAX_EVENTS            48    /* Max. number of event control blocks in your application      */
#define SYS_MAX_FLAGS             10    /* Max. number of Event Flag Groups    in your application      */
#define SYS_MAX_MEM_PART          10    /* Max. number of memory partitions                             */
#define SYS_MAX_QS                10    /* Max. number of queue control blocks in your application      */
#define SYS_MAX_TASKS             20    /* Max. number of tasks in your application, MUST be >= 2       */

#define SYS_SCHED_LOCK_EN          1    /* Include code for SysSchedLock() and SysSchedUnlock()           */

#define SYS_TICK_STEP_EN           1    /* Enable tick stepping feature for uC/Sys-View                  */
#define SYS_TICKS_PER_SEC        100    /* Set the number of ticks in one second                        */


                                       /* --------------------- TASK STACK SIZE ---------------------- */
#define SYS_TASK_TMR_STK_SIZE    128    /* Timer      task stack size (# of SYS_STK wide entries)        */
#define SYS_TASK_STAT_STK_SIZE   128    /* Statistics task stack size (# of SYS_STK wide entries)        */
#define SYS_TASK_IDLE_STK_SIZE   128    /* Idle       task stack size (# of SYS_STK wide entries)        */


                                       /* --------------------- TASK MANAGEMENT ---------------------- */
#define SYS_TASK_CHANGE_PRIO_EN    1    /*     Include code for SysTaskChangePrio()                      */
#define SYS_TASK_CREATE_EN         1    /*     Include code for SysTaskCreate()                          */
#define SYS_TASK_CREATE_EXT_EN     1    /*     Include code for SysTaskCreateExt()                       */
#define SYS_TASK_DEL_EN            1    /*     Include code for SysTaskDel()                             */
#define SYS_TASK_NAME_SIZE        16    /*     Determine the size of a task name                        */
#define SYS_TASK_PROFILE_EN        1    /*     Include variables in SYS_TCB for profiling                */
#define SYS_TASK_QUERY_EN          1    /*     Include code for SysTaskQuery()                           */
#define SYS_TASK_STAT_EN           1    /*     Enable (1) or Disable(0) the statistics task             */
#define SYS_TASK_STAT_STK_CHK_EN   1    /*     Check task stacks from statistic task                    */
#define SYS_TASK_SUSPEND_EN        1    /*     Include code for SysTaskSuspend() and SysTaskResume()      */
#define SYS_TASK_SW_HOOK_EN        1    /*     Include code for SysTaskSwHook()                          */


                                       /* ----------------------- EVENT FLAGS ------------------------ */
#define SYS_FLAG_EN                1    /* Enable (1) or Disable (0) code generation for EVENT FLAGS    */
#define SYS_FLAG_ACCEPT_EN         1    /*     Include code for SysFlagAccept()                          */
#define SYS_FLAG_DEL_EN            1    /*     Include code for SysFlagDel()                             */
#define SYS_FLAG_NAME_SIZE        16    /*     Determine the size of the name of an event flag group    */
#define SYS_FLAG_QUERY_EN          1    /*     Include code for SysFlagQuery()                           */
#define SYS_FLAG_WAIT_CLR_EN       1    /* Include code for Wait on Clear EVENT FLAGS                   */
#define SYS_FLAGS_NBITS           16    /* Size in #bits of SYS_FLAGS data type (8, 16 or 32)            */


                                       /* -------------------- MESSAGE MAILBOXES --------------------- */
#define SYS_MBOX_EN                1    /* Enable (1) or Disable (0) code generation for MAILBOXES      */
#define SYS_MBOX_ACCEPT_EN         1    /*     Include code for SysMboxAccept()                          */
#define SYS_MBOX_DEL_EN            1    /*     Include code for SysMboxDel()                             */
#define SYS_MBOX_PEND_ABORT_EN     1    /*     Include code for SysMboxPendAbort()                       */
#define SYS_MBOX_PSysT_EN           1    /*     Include code for SysMboxPost()                            */
#define SYS_MBOX_PSysT_OPT_EN       1    /*     Include code for SysMboxPostOpt()                         */
#define SYS_MBOX_QUERY_EN          1    /*     Include code for SysMboxQuery()                           */


                                       /* --------------------- MEMORY MANAGEMENT -------------------- */
#define SYS_MEM_EN                 1    /* Enable (1) or Disable (0) code generation for MEMORY MANAGER */
#define SYS_MEM_NAME_SIZE         16    /*     Determine the size of a memory partition name            */
#define SYS_MEM_QUERY_EN           1    /*     Include code for SysMemQuery()                            */


                                       /* ---------------- MUTUAL EXCLUSION SEMAPHORES --------------- */
#define SYS_MUTEX_EN               1    /* Enable (1) or Disable (0) code generation for MUTEX          */
#define SYS_MUTEX_ACCEPT_EN        1    /*     Include code for SysMutexAccept()                         */
#define SYS_MUTEX_DEL_EN           1    /*     Include code for SysMutexDel()                            */
#define SYS_MUTEX_QUERY_EN         1    /*     Include code for SysMutexQuery()                          */


                                       /* ---------------------- MESSAGE QUEUES ---------------------- */
#define SYS_Q_EN                   1    /* Enable (1) or Disable (0) code generation for QUEUES         */
#define SYS_Q_ACCEPT_EN            1    /*     Include code for SysQAccept()                             */
#define SYS_Q_DEL_EN               1    /*     Include code for SysQDel()                                */
#define SYS_Q_FLUSH_EN             1    /*     Include code for SysQFlush()                              */
#define SYS_Q_PEND_ABORT_EN        1    /*     Include code for SysQPendAbort()                          */
#define SYS_Q_PSysT_EN              1    /*     Include code for SysQPost()                               */
#define SYS_Q_PSysT_FRONT_EN        1    /*     Include code for SysQPostFront()                          */
#define SYS_Q_PSysT_OPT_EN          1    /*     Include code for SysQPostOpt()                            */
#define SYS_Q_QUERY_EN             1    /*     Include code for SysQQuery()                              */


                                       /* ------------------------ SEMAPHORES ------------------------ */
#define SYS_SEM_EN                 1    /* Enable (1) or Disable (0) code generation for SEMAPHORES     */
#define SYS_SEM_ACCEPT_EN          1    /*    Include code for SysSemAccept()                            */
#define SYS_SEM_DEL_EN             1    /*    Include code for SysSemDel()                               */
#define SYS_SEM_PEND_ABORT_EN      1    /*    Include code for SysSemPendAbort()                         */
#define SYS_SEM_QUERY_EN           1    /*    Include code for SysSemQuery()                             */
#define SYS_SEM_SET_EN             1    /*    Include code for SysSemSet()                               */


                                       /* --------------------- TIME MANAGEMENT ---------------------- */
#define SYS_TIME_DLY_HMSM_EN       1    /*     Include code for SysTimeDlyHMSM()                         */
#define SYS_TIME_DLY_RESUME_EN     1    /*     Include code for SysTimeDlyResume()                       */
#define SYS_TIME_GET_SET_EN        1    /*     Include code for SysTimeGet() and SysTimeSet()             */
#define SYS_TIME_TICK_HOOK_EN      1    /*     Include code for SysTimeTickHook()                        */


                                       /* --------------------- TIMER MANAGEMENT --------------------- */
#define SYS_TMR_EN                 1    /* Enable (1) or Disable (0) code generation for TIMERS         */
#define SYS_TMR_CFG_MAX           16    /*     Maximum number of timers                                 */
#define SYS_TMR_CFG_NAME_SIZE     16    /*     Determine the size of a timer name                       */
#define SYS_TMR_CFG_WHEEL_SIZE     8    /*     Size of timer wheel (#Spokes)                            */
#define SYS_TMR_CFG_TICKS_PER_SEC 10    /*     Rate at which timer management task runs (Hz)            */

#endif
