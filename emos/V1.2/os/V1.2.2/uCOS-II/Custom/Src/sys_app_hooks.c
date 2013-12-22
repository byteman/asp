#include "C2.h"
#include "lib_def.h"

/*
*********************************************************************************************************
*********************************************************************************************************
*                                          uC/Sys-II APP HOOKS
*********************************************************************************************************
*********************************************************************************************************
*/

#if (SYS_APP_HOOKS_EN > 0)
/*
*********************************************************************************************************
*                                      TASK CREATION HOOK (APPLICATION)
*
* Description : This function is called when a task is created.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being created.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskCreateHook (SYS_TCB *ptcb)
{
#if ((APP_SYS_PROBE_EN   == DEF_ENABLED) && \
     (SYS_PROBE_HOOKS_EN == DEF_ENABLED))
    SysProbe_TaskCreateHook(ptcb);
#endif
}

/*
*********************************************************************************************************
*                                    TASK DELETION HOOK (APPLICATION)
*
* Description : This function is called when a task is deleted.
*
* Argument(s) : ptcb   is a pointer to the task control block of the task being deleted.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*********************************************************************************************************
*/

void  App_TaskDelHook (SYS_TCB *ptcb)
{
    (void)ptcb;
}

/*
*********************************************************************************************************
*                                      IDLE TASK HOOK (APPLICATION)
*
* Description : This function is called by SysTaskIdleHook(), which is called by the idle task.  This hook
*               has been added to allow you to do such things as STOP the CPU to conserve power.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are enabled during this call.
*********************************************************************************************************
*/

#if SYS_VERSION >= 251
void  App_TaskIdleHook (void)
{
}
#endif

/*
*********************************************************************************************************
*                                        STATISTIC TASK HOOK (APPLICATION)
*
* Description : This function is called by SysTaskStatHook(), which is called every second by uC/Sys-II's
*               statistics task.  This allows your application to add functionality to the statistics task.
*
* Argument(s) : none.
*********************************************************************************************************
*/

void  App_TaskStatHook (void)
{
}

/*
*********************************************************************************************************
*                                        TASK SWITCH HOOK (APPLICATION)
*
* Description : This function is called when a task switch is performed.  This allows you to perform other
*               operations during a context switch.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts are disabled during this call.
*
*               (2) It is assumed that the global pointer 'SysTCBHighRdy' points to the TCB of the task that
*                   will be 'switched in' (i.e. the highest priority task) and, 'SysTCBCur' points to the
*                  task being switched out (i.e. the preempted task).
*********************************************************************************************************
*/

#if SYS_TASK_SW_HOOK_EN > 0
void  App_TaskSwHook (void)
{
#if ((APP_SYS_PROBE_EN   == DEF_ENABLED) && \
     (SYS_PROBE_HOOKS_EN == DEF_ENABLED))
    SysProbe_TaskSwHook();
#endif
}
#endif

/*
*********************************************************************************************************
*                                     SYS_TCBInit() HOOK (APPLICATION)
*
* Description : This function is called by SysTCBInitHook(), which is called by SYS_TCBInit() after setting
*               up most of the TCB.
*
* Argument(s) : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if SYS_VERSION >= 204
void  App_TCBInitHook (SYS_TCB *ptcb)
{
    (void)ptcb;
}
#endif

/*
*********************************************************************************************************
*                                        TICK HOOK (APPLICATION)
*
* Description : This function is called every tick.
*
* Argument(s) : none.
*
* Note(s)     : (1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/

#if SYS_TIME_TICK_HOOK_EN > 0
void  App_TimeTickHook (void)
{
#if ((APP_SYS_PROBE_EN   == DEF_ENABLED) && \
     (SYS_PROBE_HOOKS_EN == DEF_ENABLED))
    SysProbe_TickHook();
#endif
}
#endif
#endif
