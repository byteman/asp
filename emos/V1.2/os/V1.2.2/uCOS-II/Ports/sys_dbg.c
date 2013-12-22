/*
*********************************************************************************************************
*                                               uC/Sys-II
*                                         The Real-Time Kernel
*                                          DEBUGGER CONSTANTS
*
*                                (c) Copyright 2006, Micrium, Weston, FL
*                                          All Rights Reserved
*
*                                           ARM Cortex-M3 Port
*
* File      : SYS_DBG.C
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

#include <C2.h>

#define  SYS_COMPILER_OPT  

/*
*********************************************************************************************************
*                                             DEBUG DATA
*********************************************************************************************************
*/

SYS_COMPILER_OPT  INT16U  const  SysDebugEn          = SYS_DEBUG_EN;                /* Debug constants are defined below   */

#if SYS_DEBUG_EN > 0

SYS_COMPILER_OPT  INT32U  const  SysEndiannessTest   = 0x12345678L;                /* Variable to test CPU endianness     */

SYS_COMPILER_OPT  INT16U  const  SysEventMax         = SYS_MAX_EVENTS;              /* Number of event control blocks      */
SYS_COMPILER_OPT  INT16U  const  SysEventNameSize    = SYS_EVENT_NAME_SIZE;         /* Size (in bytes) of event names      */
SYS_COMPILER_OPT  INT16U  const  SysEventEn          = SYS_EVENT_EN;
#if (SYS_EVENT_EN > 0) && (SYS_MAX_EVENTS > 0)
SYS_COMPILER_OPT  INT16U  const  SysEventSize        = sizeof(SYS_EVENT);           /* Size in Bytes of SYS_EVENT           */
SYS_COMPILER_OPT  INT16U  const  SysEventTblSize     = sizeof(SysEventTbl);         /* Size of SysEventTbl[] in bytes       */
#else
SYS_COMPILER_OPT  INT16U  const  SysEventSize        = 0;
SYS_COMPILER_OPT  INT16U  const  SysEventTblSize     = 0;
#endif

SYS_COMPILER_OPT  INT16U  const  SysFlagEn           = SYS_FLAG_EN;
#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
SYS_COMPILER_OPT  INT16U  const  SysFlagGrpSize      = sizeof(SYS_FLAG_GRP);        /* Size in Bytes of SYS_FLAG_GRP        */
SYS_COMPILER_OPT  INT16U  const  SysFlagNodeSize     = sizeof(SYS_FLAG_NODE);       /* Size in Bytes of SYS_FLAG_NODE       */
SYS_COMPILER_OPT  INT16U  const  SysFlagWidth        = sizeof(SYS_FLAGS);           /* Width (in bytes) of SYS_FLAGS        */
#else
SYS_COMPILER_OPT  INT16U  const  SysFlagGrpSize      = 0;
SYS_COMPILER_OPT  INT16U  const  SysFlagNodeSize     = 0;
SYS_COMPILER_OPT  INT16U  const  SysFlagWidth        = 0;
#endif
SYS_COMPILER_OPT  INT16U  const  SysFlagMax          = SYS_MAX_FLAGS;
SYS_COMPILER_OPT  INT16U  const  SysFlagNameSize     = SYS_FLAG_NAME_SIZE;          /* Size (in bytes) of flag names       */

SYS_COMPILER_OPT  INT16U  const  SysLowestPrio       = SYS_LOWEST_PRIO;

SYS_COMPILER_OPT  INT16U  const  SysMboxEn           = SYS_MBOX_EN;

SYS_COMPILER_OPT  INT16U  const  SysMemEn            = SYS_MEM_EN;
SYS_COMPILER_OPT  INT16U  const  SysMemMax           = SYS_MAX_MEM_PART;            /* Number of memory partitions         */
SYS_COMPILER_OPT  INT16U  const  SysMemNameSize      = SYS_MEM_NAME_SIZE;           /* Size (in bytes) of partition names  */
#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
SYS_COMPILER_OPT  INT16U  const  SysMemSize          = sizeof(SYS_MEM);             /* Mem. Partition header sine (bytes)  */
SYS_COMPILER_OPT  INT16U  const  SysMemTblSize       = sizeof(SysMemTbl);
#else
SYS_COMPILER_OPT  INT16U  const  SysMemSize          = 0;
SYS_COMPILER_OPT  INT16U  const  SysMemTblSize       = 0;
#endif
SYS_COMPILER_OPT  INT16U  const  SysMutexEn          = SYS_MUTEX_EN;

SYS_COMPILER_OPT  INT16U  const  SysPtrSize          = sizeof(void *);             /* Size in Bytes of a pointer          */

SYS_COMPILER_OPT  INT16U  const  SysQEn              = SYS_Q_EN;
SYS_COMPILER_OPT  INT16U  const  SysQMax             = SYS_MAX_QS;                  /* Number of queues                    */
#if (SYS_Q_EN > 0) && (SYS_MAX_QS > 0)
SYS_COMPILER_OPT  INT16U  const  SysQSize            = sizeof(SYS_Q);               /* Size in bytes of SYS_Q structure     */
#else
SYS_COMPILER_OPT  INT16U  const  SysQSize            = 0;
#endif

SYS_COMPILER_OPT  INT16U  const  SysRdyTblSize       = SYS_RDY_TBL_SIZE;            /* Number of bytes in the ready table  */

SYS_COMPILER_OPT  INT16U  const  SysSemEn            = SYS_SEM_EN;

SYS_COMPILER_OPT  INT16U  const  SysStkWidth         = sizeof(SYS_STK);             /* Size in Bytes of a stack entry      */

SYS_COMPILER_OPT  INT16U  const  SysTaskCreateEn     = SYS_TASK_CREATE_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskCreateExtEn  = SYS_TASK_CREATE_EXT_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskDelEn        = SYS_TASK_DEL_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskIdleStkSize  = SYS_TASK_IDLE_STK_SIZE;
SYS_COMPILER_OPT  INT16U  const  SysTaskProfileEn    = SYS_TASK_PROFILE_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskMax          = SYS_MAX_TASKS + SYS_N_SYS_TASKS;  /* Total max. number of tasks      */
SYS_COMPILER_OPT  INT16U  const  SysTaskNameSize     = SYS_TASK_NAME_SIZE;              /* Size (in bytes) of task names   */
SYS_COMPILER_OPT  INT16U  const  SysTaskStatEn       = SYS_TASK_STAT_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskStatStkSize  = SYS_TASK_STAT_STK_SIZE;
SYS_COMPILER_OPT  INT16U  const  SysTaskStatStkChkEn = SYS_TASK_STAT_STK_CHK_EN;
SYS_COMPILER_OPT  INT16U  const  SysTaskSwHookEn     = SYS_TASK_SW_HOOK_EN;

SYS_COMPILER_OPT  INT16U  const  SysTCBPrioTblMax    = SYS_LOWEST_PRIO + 1;         /* Number of entries in SysTCBPrioTbl[] */
SYS_COMPILER_OPT  INT16U  const  SysTCBSize          = sizeof(SYS_TCB);             /* Size in Bytes of SYS_TCB             */
SYS_COMPILER_OPT  INT16U  const  SysTicksPerSec      = SYS_TICKS_PER_SEC;
SYS_COMPILER_OPT  INT16U  const  SysTimeTickHookEn   = SYS_TIME_TICK_HOOK_EN;
SYS_COMPILER_OPT  INT16U  const  SysVersionNbr       = SYS_VERSION;

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                             DEBUG DATA
*                            TOTAL DATA SPACE (i.e. RAM) USED BY uC/Sys-II
*********************************************************************************************************
*/
#if SYS_DEBUG_EN > 0

SYS_COMPILER_OPT  INT16U  const  SysDataSize = sizeof(SysCtxSwCtr)
#if (SYS_EVENT_EN > 0) && (SYS_MAX_EVENTS > 0)
                                           + sizeof(SysEventFreeList)
                                           + sizeof(SysEventTbl)
#endif
#if (SYS_VERSION >= 251) && (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
                                           + sizeof(SysFlagTbl)
                                           + sizeof(SysFlagFreeList)
#endif
#if SYS_TASK_STAT_EN > 0
                                           + sizeof(SysCPUUsage)
                                           + sizeof(SysIdleCtrMax)
                                           + sizeof(SysIdleCtrRun)
                                           + sizeof(SysStatRdy)
                                           + sizeof(SysTaskStatStk)
#endif
#if SYS_TICK_STEP_EN > 0
                                           + sizeof(SysTickStepState)
#endif
#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
                                           + sizeof(SysMemFreeList)
                                           + sizeof(SysMemTbl)
#endif
#if (SYS_Q_EN > 0) && (SYS_MAX_QS > 0)
                                           + sizeof(SysQFreeList)
                                           + sizeof(SysQTbl)
#endif
#if SYS_TIME_GET_SET_EN > 0
                                           + sizeof(SysTime)
#endif
                                           + sizeof(SysIntNesting)
                                           + sizeof(SysLockNesting)
                                           + sizeof(SysPrioCur)
                                           + sizeof(SysPrioHighRdy)
                                           + sizeof(SysRdyGrp)
                                           + sizeof(SysRdyTbl)
                                           + sizeof(SysRunning)
                                           + sizeof(SysTaskCtr)
                                           + sizeof(SysIdleCtr)
                                           + sizeof(SysTaskIdleStk)
                                           + sizeof(SysTCBCur)
                                           + sizeof(SysTCBFreeList)
                                           + sizeof(SysTCBHighRdy)
                                           + sizeof(SysTCBList)
                                           + sizeof(SysTCBPrioTbl)
                                           + sizeof(SysTCBTbl);

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                       Sys DEBUG INITIALIZAZTION
*
* Description: This function is used to make sure that debug variables that are unused in the application
*              are not optimized away.  This function might not be necessary for all compilers.  In this
*              case, you should simply DELETE the code in this function while still leaving the declaration
*              of the function itself.
*
* Arguments  : none
*
* Returns    : none
*
* Note(s)    : (1) This code doesn't do anything, it simply prevents the compiler from optimizing out
*                  the 'const' variables which are declared in this file.
*********************************************************************************************************
*/

#if SYS_VERSION >= 270 && SYS_DEBUG_EN > 0
void  SysDebugInit (void)
{
    void  *ptemp;


    ptemp = (void *)&SysDebugEn;

    ptemp = (void *)&SysEndiannessTest;

    ptemp = (void *)&SysEventMax;
    ptemp = (void *)&SysEventNameSize;
    ptemp = (void *)&SysEventEn;
    ptemp = (void *)&SysEventSize;
    ptemp = (void *)&SysEventTblSize;

    ptemp = (void *)&SysFlagEn;
    ptemp = (void *)&SysFlagGrpSize;
    ptemp = (void *)&SysFlagNodeSize;
    ptemp = (void *)&SysFlagWidth;
    ptemp = (void *)&SysFlagMax;
    ptemp = (void *)&SysFlagNameSize;

    ptemp = (void *)&SysLowestPrio;

    ptemp = (void *)&SysMboxEn;

    ptemp = (void *)&SysMemEn;
    ptemp = (void *)&SysMemMax;
    ptemp = (void *)&SysMemNameSize;
    ptemp = (void *)&SysMemSize;
    ptemp = (void *)&SysMemTblSize;

    ptemp = (void *)&SysMutexEn;

    ptemp = (void *)&SysPtrSize;

    ptemp = (void *)&SysQEn;
    ptemp = (void *)&SysQMax;
    ptemp = (void *)&SysQSize;

    ptemp = (void *)&SysRdyTblSize;

    ptemp = (void *)&SysSemEn;

    ptemp = (void *)&SysStkWidth;

    ptemp = (void *)&SysTaskCreateEn;
    ptemp = (void *)&SysTaskCreateExtEn;
    ptemp = (void *)&SysTaskDelEn;
    ptemp = (void *)&SysTaskIdleStkSize;
    ptemp = (void *)&SysTaskProfileEn;
    ptemp = (void *)&SysTaskMax;
    ptemp = (void *)&SysTaskNameSize;
    ptemp = (void *)&SysTaskStatEn;
    ptemp = (void *)&SysTaskStatStkSize;
    ptemp = (void *)&SysTaskStatStkChkEn;
    ptemp = (void *)&SysTaskSwHookEn;

    ptemp = (void *)&SysTCBPrioTblMax;
    ptemp = (void *)&SysTCBSize;

    ptemp = (void *)&SysTicksPerSec;
    ptemp = (void *)&SysTimeTickHookEn;

    ptemp = (void *)&SysVersionNbr;

    ptemp = (void *)&SysDataSize;

    ptemp = ptemp;                             /* Prevent compiler warning for 'ptemp' not being used! */
}
#endif
