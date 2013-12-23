/*
*********************************************************************************************************
*                                                uC/Sys-II
*                                          The Real-Time Kernel
*
*                              (c) Copyright 1992-2007, Micrium, Weston, FL
*                                           All Rights Reserved
*
* File    : uCSYS_II.H
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

#ifndef   SYS_uCSYS_II_H
#define   SYS_uCSYS_II_H

#ifdef __cplusplus
extern "C" {
#endif

/*
*********************************************************************************************************
*                                          uC/Sys-II VERSION NUMBER
*********************************************************************************************************
*/

#define  SYS_VERSION                 286u                /* Version of uC/Sys-II (Vx.yy mult. by 100)    */

/*
*********************************************************************************************************
*                                           INCLUDE HEADER FILES
*********************************************************************************************************
*/

#include <sys_cfg.h>
#include <sys_cpu.h>

/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/

#ifdef   SYS_GLOBALS
#define  SYS_EXT
#else
#define  SYS_EXT  extern
#endif

#ifndef  SYS_FALSE
#define  SYS_FALSE                     0u
#endif

#ifndef  SYS_TRUE
#define  SYS_TRUE                      1u
#endif

#define  SYS_ASCII_NUL          (INT8U)0

#define  SYS_PRIO_SELF              0xFFu                /* Indicate SELF priority                      */

#if SYS_TASK_STAT_EN > 0
#define  SYS_N_SYS_TASKS               2u                /* Number of system tasks                      */
#else
#define  SYS_N_SYS_TASKS               1u
#endif

#define  SYS_TASK_STAT_PRIO  (SYS_LOWEST_PRIO - 1)        /* Statistic task priority                     */
#define  SYS_TASK_IDLE_PRIO  (SYS_LOWEST_PRIO)            /* IDLE      task priority                     */

#if SYS_LOWEST_PRIO <= 63
#define  SYS_EVENT_TBL_SIZE ((SYS_LOWEST_PRIO) / 8 + 1)   /* Size of event table                         */
#define  SYS_RDY_TBL_SIZE   ((SYS_LOWEST_PRIO) / 8 + 1)   /* Size of ready table                         */
#else
#define  SYS_EVENT_TBL_SIZE ((SYS_LOWEST_PRIO) / 16 + 1)  /* Size of event table                         */
#define  SYS_RDY_TBL_SIZE   ((SYS_LOWEST_PRIO) / 16 + 1)  /* Size of ready table                         */
#endif

#define  SYS_TASK_IDLE_ID          65535u                /* ID numbers for Idle, Stat and Timer tasks   */
#define  SYS_TASK_STAT_ID          65534u
#define  SYS_TASK_TMR_ID           65533u

#define  SYS_EVENT_EN           (((SYS_Q_EN > 0) && (SYS_MAX_QS > 0)) || (SYS_MBOX_EN > 0) || (SYS_SEM_EN > 0) || (SYS_MUTEX_EN > 0))

#define  SYS_TCB_RESERVED        ((SYS_TCB *)1)

/*$PAGE*/
/*
*********************************************************************************************************
*                              TASK STATUS (Bit definition for SysTCBStat)
*********************************************************************************************************
*/
#define  SYS_STAT_RDY               0x00u    /* Ready to run                                            */
#define  SYS_STAT_SEM               0x01u    /* Pending on semaphore                                    */
#define  SYS_STAT_MBOX              0x02u    /* Pending on mailbox                                      */
#define  SYS_STAT_Q                 0x04u    /* Pending on queue                                        */
#define  SYS_STAT_SUSPEND           0x08u    /* Task is suspended                                       */
#define  SYS_STAT_MUTEX             0x10u    /* Pending on mutual exclusion semaphore                   */
#define  SYS_STAT_FLAG              0x20u    /* Pending on event flag group                             */
#define  SYS_STAT_MULTI             0x80u    /* Pending on multiple events                              */

#define  SYS_STAT_PEND_ANY         (SYS_STAT_SEM | SYS_STAT_MBOX | SYS_STAT_Q | SYS_STAT_MUTEX | SYS_STAT_FLAG)

/*
*********************************************************************************************************
*                           TASK PEND STATUS (Status codes for SysTCBStatPend)
*********************************************************************************************************
*/
#define  SYS_STAT_PEND_OK              0u    /* Pending status OK, not pending, or pending complete     */
#define  SYS_STAT_PEND_TO              1u    /* Pending timed out                                       */
#define  SYS_STAT_PEND_ABORT           2u    /* Pending aborted                                         */

/*
*********************************************************************************************************
*                                        SYS_EVENT types
*********************************************************************************************************
*/
#define  SYS_EVENT_TYPE_UNUSED         0u
#define  SYS_EVENT_TYPE_MBOX           1u
#define  SYS_EVENT_TYPE_Q              2u
#define  SYS_EVENT_TYPE_SEM            3u
#define  SYS_EVENT_TYPE_MUTEX          4u
#define  SYS_EVENT_TYPE_FLAG           5u

#define  SYS_TMR_TYPE                100u    /* Used to identify Timers ...                             */
                                            /* ... (Must be different value than SYS_EVENT_TYPE_xxx)    */

/*
*********************************************************************************************************
*                                         EVENT FLAGS
*********************************************************************************************************
*/
#define  SYS_FLAG_WAIT_CLR_ALL         0u    /* Wait for ALL    the bits specified to be CLR (i.e. 0)   */
#define  SYS_FLAG_WAIT_CLR_AND         0u

#define  SYS_FLAG_WAIT_CLR_ANY         1u    /* Wait for ANY of the bits specified to be CLR (i.e. 0)   */
#define  SYS_FLAG_WAIT_CLR_OR          1u

#define  SYS_FLAG_WAIT_SET_ALL         2u    /* Wait for ALL    the bits specified to be SET (i.e. 1)   */
#define  SYS_FLAG_WAIT_SET_AND         2u

#define  SYS_FLAG_WAIT_SET_ANY         3u    /* Wait for ANY of the bits specified to be SET (i.e. 1)   */
#define  SYS_FLAG_WAIT_SET_OR          3u


#define  SYS_FLAG_CONSUME           0x80u    /* Consume the flags if condition(s) satisfied             */


#define  SYS_FLAG_CLR                  0u
#define  SYS_FLAG_SET                  1u

/*
*********************************************************************************************************
*                                   Values for SysTickStepState
*
* Note(s): This feature is used by uC/Sys-View.
*********************************************************************************************************
*/

#if SYS_TICK_STEP_EN > 0
#define  SYS_TICK_STEP_DIS             0u    /* Stepping is disabled, tick runs as mormal               */
#define  SYS_TICK_STEP_WAIT            1u    /* Waiting for uC/Sys-View to set SysTickStepState to _ONCE  */
#define  SYS_TICK_STEP_ONCE            2u    /* Process tick once and wait for next cmd from uC/Sys-View */
#endif

/*
*********************************************************************************************************
*       Possible values for 'opt' argument of SysSemDel(), SysMboxDel(), SysQDel() and SysMutexDel()
*********************************************************************************************************
*/
#define  SYS_DEL_NO_PEND               0u
#define  SYS_DEL_ALWAYS                1u

/*
*********************************************************************************************************
*                                        Sys???Pend() OPTIONS
*
* These #defines are used to establish the options for Sys???PendAbort().
*********************************************************************************************************
*/
#define  SYS_PEND_OPT_NONE             0u    /* NO option selected                                      */
#define  SYS_PEND_OPT_BROADCAST        1u    /* Broadcast action to ALL tasks waiting                   */

/*
*********************************************************************************************************
*                                     Sys???PostOpt() OPTIONS
*
* These #defines are used to establish the options for SysMboxPostOpt() and SysQPostOpt().
*********************************************************************************************************
*/
#define  SYS_PSysT_OPT_NONE          0x00u    /* NO option selected                                      */
#define  SYS_PSysT_OPT_BROADCAST     0x01u    /* Broadcast message to ALL tasks waiting                  */
#define  SYS_PSysT_OPT_FRONT         0x02u    /* Post to highest priority task waiting                   */
#define  SYS_PSysT_OPT_NO_SCHED      0x04u    /* Do not call the scheduler if this option is selected    */

/*
*********************************************************************************************************
*                                 TASK OPTIONS (see SysTaskCreateExt())
*********************************************************************************************************
*/
#define  SYS_TASK_OPT_NONE        0x0000u    /* NO option selected                                      */
#define  SYS_TASK_OPT_STK_CHK     0x0001u    /* Enable stack checking for the task                      */
#define  SYS_TASK_OPT_STK_CLR     0x0002u    /* Clear the stack when the task is create                 */
#define  SYS_TASK_OPT_SAVE_FP     0x0004u    /* Save the contents of any floating-point registers       */

/*
*********************************************************************************************************
*                            TIMER OPTIONS (see SysTmrStart() and SysTmrStop())
*********************************************************************************************************
*/
#define  SYS_TMR_OPT_NONE              0u    /* No option selected                                      */

#define  SYS_TMR_OPT_ONE_SHOT          1u    /* Timer will not automatically restart when it expires    */
#define  SYS_TMR_OPT_PERIODIC          2u    /* Timer will     automatically restart when it expires    */

#define  SYS_TMR_OPT_CALLBACK          3u    /* SysTmrStop() option to call 'callback' w/ timer arg.     */
#define  SYS_TMR_OPT_CALLBACK_ARG      4u    /* SysTmrStop() option to call 'callback' w/ new   arg.     */

/*
*********************************************************************************************************
*                                            TIMER STATES
*********************************************************************************************************
*/
#define  SYS_TMR_STATE_UNUSED          0u
#define  SYS_TMR_STATE_STOPPED         1u
#define  SYS_TMR_STATE_COMPLETED       2u
#define  SYS_TMR_STATE_RUNNING         3u

/*
*********************************************************************************************************
*                                             ERROR CODES
*********************************************************************************************************
*/
#define SYS_ERR_NONE                   0u

#define SYS_ERR_EVENT_TYPE             1u
#define SYS_ERR_PEND_ISR               2u
#define SYS_ERR_PSysT_NULL_PTR          3u
#define SYS_ERR_PEVENT_NULL            4u
#define SYS_ERR_PSysT_ISR               5u
#define SYS_ERR_QUERY_ISR              6u
#define SYS_ERR_INVALID_OPT            7u
#define SYS_ERR_PDATA_NULL             9u

#define SYS_ERR_TIMEOUT               10u
#define SYS_ERR_EVENT_NAME_TOO_LONG   11u
#define SYS_ERR_PNAME_NULL            12u
#define SYS_ERR_PEND_LOCKED           13u
#define SYS_ERR_PEND_ABORT            14u
#define SYS_ERR_DEL_ISR               15u
#define SYS_ERR_CREATE_ISR            16u
#define SYS_ERR_NAME_GET_ISR          17u
#define SYS_ERR_NAME_SET_ISR          18u

#define SYS_ERR_MBOX_FULL             20u

#define SYS_ERR_Q_FULL                30u
#define SYS_ERR_Q_EMPTY               31u

#define SYS_ERR_PRIO_EXIST            40u
#define SYS_ERR_PRIO                  41u
#define SYS_ERR_PRIO_INVALID          42u

#define SYS_ERR_SEM_OVF               50u

#define SYS_ERR_TASK_CREATE_ISR       60u
#define SYS_ERR_TASK_DEL              61u
#define SYS_ERR_TASK_DEL_IDLE         62u
#define SYS_ERR_TASK_DEL_REQ          63u
#define SYS_ERR_TASK_DEL_ISR          64u
#define SYS_ERR_TASK_NAME_TOO_LONG    65u
#define SYS_ERR_TASK_NO_MORE_TCB      66u
#define SYS_ERR_TASK_NOT_EXIST        67u
#define SYS_ERR_TASK_NOT_SUSPENDED    68u
#define SYS_ERR_TASK_OPT              69u
#define SYS_ERR_TASK_RESUME_PRIO      70u
#define SYS_ERR_TASK_SUSPEND_IDLE     71u
#define SYS_ERR_TASK_SUSPEND_PRIO     72u
#define SYS_ERR_TASK_WAITING          73u

#define SYS_ERR_TIME_NOT_DLY          80u
#define SYS_ERR_TIME_INVALID_MINUTES  81u
#define SYS_ERR_TIME_INVALID_SECONDS  82u
#define SYS_ERR_TIME_INVALID_MS       83u
#define SYS_ERR_TIME_ZERO_DLY         84u
#define SYS_ERR_TIME_DLY_ISR          85u

#define SYS_ERR_MEM_INVALID_PART      90u
#define SYS_ERR_MEM_INVALID_BLKS      91u
#define SYS_ERR_MEM_INVALID_SIZE      92u
#define SYS_ERR_MEM_NO_FREE_BLKS      93u
#define SYS_ERR_MEM_FULL              94u
#define SYS_ERR_MEM_INVALID_PBLK      95u
#define SYS_ERR_MEM_INVALID_PMEM      96u
#define SYS_ERR_MEM_INVALID_PDATA     97u
#define SYS_ERR_MEM_INVALID_ADDR      98u
#define SYS_ERR_MEM_NAME_TOO_LONG     99u

#define SYS_ERR_NOT_MUTEX_OWNER      100u

#define SYS_ERR_FLAG_INVALID_PGRP    110u
#define SYS_ERR_FLAG_WAIT_TYPE       111u
#define SYS_ERR_FLAG_NOT_RDY         112u
#define SYS_ERR_FLAG_INVALID_OPT     113u
#define SYS_ERR_FLAG_GRP_DEPLETED    114u
#define SYS_ERR_FLAG_NAME_TOO_LONG   115u

#define SYS_ERR_PIP_LOWER            120u

#define SYS_ERR_TMR_INVALID_DLY      130u
#define SYS_ERR_TMR_INVALID_PERIOD   131u
#define SYS_ERR_TMR_INVALID_OPT      132u
#define SYS_ERR_TMR_INVALID_NAME     133u
#define SYS_ERR_TMR_NON_AVAIL        134u
#define SYS_ERR_TMR_INACTIVE         135u
#define SYS_ERR_TMR_INVALID_DEST     136u
#define SYS_ERR_TMR_INVALID_TYPE     137u
#define SYS_ERR_TMR_INVALID          138u
#define SYS_ERR_TMR_ISR              139u
#define SYS_ERR_TMR_NAME_TOO_LONG    140u
#define SYS_ERR_TMR_INVALID_STATE    141u
#define SYS_ERR_TMR_STOPPED          142u
#define SYS_ERR_TMR_NO_CALLBACK      143u

/*
*********************************************************************************************************
*                                    OLD ERROR CODE NAMES (< V2.84)
*********************************************************************************************************
*/
#define SYS_NO_ERR                    SYS_ERR_NONE
#define SYS_TIMEOUT                   SYS_ERR_TIMEOUT
#define SYS_TASK_NOT_EXIST            SYS_ERR_TASK_NOT_EXIST
#define SYS_MBOX_FULL                 SYS_ERR_MBOX_FULL
#define SYS_Q_FULL                    SYS_ERR_Q_FULL
#define SYS_Q_EMPTY                   SYS_ERR_Q_EMPTY
#define SYS_PRIO_EXIST                SYS_ERR_PRIO_EXIST
#define SYS_PRIO_ERR                  SYS_ERR_PRIO
#define SYS_PRIO_INVALID              SYS_ERR_PRIO_INVALID
#define SYS_SEM_OVF                   SYS_ERR_SEM_OVF
#define SYS_TASK_DEL_ERR              SYS_ERR_TASK_DEL
#define SYS_TASK_DEL_IDLE             SYS_ERR_TASK_DEL_IDLE
#define SYS_TASK_DEL_REQ              SYS_ERR_TASK_DEL_REQ
#define SYS_TASK_DEL_ISR              SYS_ERR_TASK_DEL_ISR
#define SYS_NO_MORE_TCB               SYS_ERR_TASK_NO_MORE_TCB
#define SYS_TIME_NOT_DLY              SYS_ERR_TIME_NOT_DLY
#define SYS_TIME_INVALID_MINUTES      SYS_ERR_TIME_INVALID_MINUTES
#define SYS_TIME_INVALID_SECONDS      SYS_ERR_TIME_INVALID_SECONDS
#define SYS_TIME_INVALID_MS           SYS_ERR_TIME_INVALID_MS
#define SYS_TIME_ZERO_DLY             SYS_ERR_TIME_ZERO_DLY
#define SYS_TASK_SUSPEND_PRIO         SYS_ERR_TASK_SUSPEND_PRIO
#define SYS_TASK_SUSPEND_IDLE         SYS_ERR_TASK_SUSPEND_IDLE
#define SYS_TASK_RESUME_PRIO          SYS_ERR_TASK_RESUME_PRIO
#define SYS_TASK_NOT_SUSPENDED        SYS_ERR_TASK_NOT_SUSPENDED
#define SYS_MEM_INVALID_PART          SYS_ERR_MEM_INVALID_PART
#define SYS_MEM_INVALID_BLKS          SYS_ERR_MEM_INVALID_BLKS
#define SYS_MEM_INVALID_SIZE          SYS_ERR_MEM_INVALID_SIZE
#define SYS_MEM_NO_FREE_BLKS          SYS_ERR_MEM_NO_FREE_BLKS
#define SYS_MEM_FULL                  SYS_ERR_MEM_FULL
#define SYS_MEM_INVALID_PBLK          SYS_ERR_MEM_INVALID_PBLK
#define SYS_MEM_INVALID_PMEM          SYS_ERR_MEM_INVALID_PMEM
#define SYS_MEM_INVALID_PDATA         SYS_ERR_MEM_INVALID_PDATA
#define SYS_MEM_INVALID_ADDR          SYS_ERR_MEM_INVALID_ADDR
#define SYS_MEM_NAME_TOO_LONG         SYS_ERR_MEM_NAME_TOO_LONG
#define SYS_TASK_OPT_ERR              SYS_ERR_TASK_OPT
#define SYS_FLAG_INVALID_PGRP         SYS_ERR_FLAG_INVALID_PGRP
#define SYS_FLAG_ERR_WAIT_TYPE        SYS_ERR_FLAG_WAIT_TYPE
#define SYS_FLAG_ERR_NOT_RDY          SYS_ERR_FLAG_NOT_RDY
#define SYS_FLAG_INVALID_OPT          SYS_ERR_FLAG_INVALID_OPT
#define SYS_FLAG_GRP_DEPLETED         SYS_ERR_FLAG_GRP_DEPLETED

/*$PAGE*/
/*
*********************************************************************************************************
*                                          EVENT CONTROL BLOCK
*********************************************************************************************************
*/

#if (SYS_EVENT_EN) && (SYS_MAX_EVENTS > 0)
typedef struct os_event {
    INT8U    SysEventType;                    /* Type of event control block (see SYS_EVENT_TYPE_xxxx)    */
    void    *SysEventPtr;                     /* Pointer to message or queue structure                   */
    INT16U   SysEventCnt;                     /* Semaphore Count (not used if other EVENT type)          */
#if SYS_LOWEST_PRIO <= 63
    INT8U    SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    INT8U    SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
#else
    INT16U   SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
    INT16U   SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
#endif

#if SYS_EVENT_NAME_SIZE > 1
    INT8U    SysEventName[SYS_EVENT_NAME_SIZE];
#endif
} SYS_EVENT;
#endif


/*
*********************************************************************************************************
*                                       EVENT FLAGS CONTROL BLOCK
*********************************************************************************************************
*/

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)

#if SYS_FLAGS_NBITS == 8                     /* Determine the size of SYS_FLAGS (8, 16 or 32 bits)       */
typedef  INT8U    SYS_FLAGS;
#endif

#if SYS_FLAGS_NBITS == 16
typedef  INT16U   SYS_FLAGS;
#endif

#if SYS_FLAGS_NBITS == 32
typedef  INT32U   SYS_FLAGS;
#endif


typedef struct os_flag_grp {                /* Event Flag Group                                        */
    INT8U         SysFlagType;               /* Should be set to SYS_EVENT_TYPE_FLAG                     */
    void         *SysFlagWaitList;           /* Pointer to first NODE of task waiting on event flag     */
    SYS_FLAGS      SysFlagFlags;              /* 8, 16 or 32 bit flags                                   */
#if SYS_FLAG_NAME_SIZE > 1
    INT8U         SysFlagName[SYS_FLAG_NAME_SIZE];
#endif
} SYS_FLAG_GRP;



typedef struct os_flag_node {               /* Event Flag Wait List Node                               */
    void         *SysFlagNodeNext;           /* Pointer to next     NODE in wait list                   */
    void         *SysFlagNodePrev;           /* Pointer to previous NODE in wait list                   */
    void         *SysFlagNodeTCB;            /* Pointer to TCB of waiting task                          */
    void         *SysFlagNodeFlagGrp;        /* Pointer to Event Flag Group                             */
    SYS_FLAGS      SysFlagNodeFlags;          /* Event flag to wait on                                   */
    INT8U         SysFlagNodeWaitType;       /* Type of wait:                                           */
                                            /*      SYS_FLAG_WAIT_AND                                   */
                                            /*      SYS_FLAG_WAIT_ALL                                   */
                                            /*      SYS_FLAG_WAIT_OR                                    */
                                            /*      SYS_FLAG_WAIT_ANY                                   */
} SYS_FLAG_NODE;
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          MESSAGE MAILBOX DATA
*********************************************************************************************************
*/

#if SYS_MBOX_EN > 0
typedef struct os_mbox_data {
    void   *SysMsg;                         /* Pointer to message in mailbox                            */
#if SYS_LOWEST_PRIO <= 63
    INT8U   SysEventTbl[SYS_EVENT_TBL_SIZE]; /* List of tasks waiting for event to occur                 */
    INT8U   SysEventGrp;                    /* Group corresponding to tasks waiting for event to occur  */
#else
    INT16U  SysEventTbl[SYS_EVENT_TBL_SIZE]; /* List of tasks waiting for event to occur                 */
    INT16U  SysEventGrp;                    /* Group corresponding to tasks waiting for event to occur  */
#endif
} SYS_MBOX_DATA;
#endif

/*
*********************************************************************************************************
*                                     MEMORY PARTITION DATA STRUCTURES
*********************************************************************************************************
*/

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
typedef struct os_mem {                   /* MEMORY CONTROL BLOCK                                      */
    void   *SysMemAddr;                    /* Pointer to beginning of memory partition                  */
    void   *SysMemFreeList;                /* Pointer to list of free memory blocks                     */
    INT32U  SysMemBlkSize;                 /* Size (in bytes) of each block of memory                   */
    INT32U  SysMemNBlks;                   /* Total number of blocks in this partition                  */
    INT32U  SysMemNFree;                   /* Number of memory blocks remaining in this partition       */
#if SYS_MEM_NAME_SIZE > 1
    INT8U   SysMemName[SYS_MEM_NAME_SIZE];  /* Memory partition name                                     */
#endif
} SYS_MEM;


typedef struct os_mem_data {
    void   *SysAddr;                    /* Pointer to the beginning address of the memory partition     */
    void   *SysFreeList;                /* Pointer to the beginning of the free list of memory blocks   */
    INT32U  SysBlkSize;                 /* Size (in bytes) of each memory block                         */
    INT32U  SysNBlks;                   /* Total number of blocks in the partition                      */
    INT32U  SysNFree;                   /* Number of memory blocks free                                 */
    INT32U  SysNUsed;                   /* Number of memory blocks used                                 */
} SYS_MEM_DATA;
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                    MUTUAL EXCLUSION SEMAPHORE DATA
*********************************************************************************************************
*/

#if SYS_MUTEX_EN > 0
typedef struct os_mutex_data {
#if SYS_LOWEST_PRIO <= 63
    INT8U   SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT8U   SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
#else
    INT16U  SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT16U  SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
#endif
    BOOLEAN SysValue;                        /* Mutex value (SYS_FALSE = used, SYS_TRUE = available)      */
    INT8U   SysOwnerPrio;                    /* Mutex owner's task priority or 0xFF if no owner         */
    INT8U   SysMutexPIP;                     /* Priority Inheritance Priority or 0xFF if no owner       */
} SYS_MUTEX_DATA;
#endif

/*
*********************************************************************************************************
*                                          MESSAGE QUEUE DATA
*********************************************************************************************************
*/

#if SYS_Q_EN > 0
typedef struct os_q {                   /* QUEUE CONTROL BLOCK                                         */
    struct os_q   *SysQPtr;              /* Link to next queue control block in list of free blocks     */
    void         **SysQStart;            /* Pointer to start of queue data                              */
    void         **SysQEnd;              /* Pointer to end   of queue data                              */
    void         **SysQIn;               /* Pointer to where next message will be inserted  in   the Q  */
    void         **SysQOut;              /* Pointer to where next message will be extracted from the Q  */
    INT16U         SysQSize;             /* Size of queue (maximum number of entries)                   */
    INT16U         SysQEntries;          /* Current number of entries in the queue                      */
} SYS_Q;


typedef struct os_q_data {
    void          *SysMsg;               /* Pointer to next message to be extracted from queue          */
    INT16U         SysNMsgs;             /* Number of messages in message queue                         */
    INT16U         SysQSize;             /* Size of message queue                                       */
#if SYS_LOWEST_PRIO <= 63
    INT8U          SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur         */
    INT8U          SysEventGrp;          /* Group corresponding to tasks waiting for event to occur     */
#else
    INT16U         SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur         */
    INT16U         SysEventGrp;          /* Group corresponding to tasks waiting for event to occur     */
#endif
} SYS_Q_DATA;
#endif

/*
*********************************************************************************************************
*                                           SEMAPHORE DATA
*********************************************************************************************************
*/

#if SYS_SEM_EN > 0
typedef struct os_sem_data {
    INT16U  SysCnt;                          /* Semaphore count                                         */
#if SYS_LOWEST_PRIO <= 63
    INT8U   SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT8U   SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
#else
    INT16U  SysEventTbl[SYS_EVENT_TBL_SIZE];  /* List of tasks waiting for event to occur                */
    INT16U  SysEventGrp;                     /* Group corresponding to tasks waiting for event to occur */
#endif
} SYS_SEM_DATA;
#endif

/*
*********************************************************************************************************
*                                            TASK STACK DATA
*********************************************************************************************************
*/

#if SYS_TASK_CREATE_EXT_EN > 0
typedef struct os_stk_data {
    INT32U  SysFree;                    /* Number of free bytes on the stack                            */
    INT32U  SysUsed;                    /* Number of bytes used on the stack                            */
} SYS_STK_DATA;
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          TASK CONTROL BLOCK
*********************************************************************************************************
*/

typedef struct os_tcb {
    SYS_STK          *SysTCBStkPtr;           /* Pointer to current top of stack                         */

#if SYS_TASK_CREATE_EXT_EN > 0
    void            *SysTCBExtPtr;           /* Pointer to user definable data for TCB extension        */
    SYS_STK          *SysTCBStkBottom;        /* Pointer to bottom of stack                              */
    INT32U           SysTCBStkSize;          /* Size of task stack (in number of stack elements)        */
    INT16U           SysTCBOpt;              /* Task options as passed by SysTaskCreateExt()             */
    INT16U           SysTCBId;               /* Task ID (0..65535)                                      */
#endif

    struct os_tcb   *SysTCBNext;             /* Pointer to next     TCB in the TCB list                 */
    struct os_tcb   *SysTCBPrev;             /* Pointer to previous TCB in the TCB list                 */

#if (SYS_EVENT_EN) || (SYS_FLAG_EN > 0)
    SYS_EVENT        *SysTCBEventPtr;         /* Pointer to          event control block                 */
#endif

#if (SYS_EVENT_EN) && (SYS_EVENT_MULTI_EN > 0)
    SYS_EVENT       **SysTCBEventMultiPtr;    /* Pointer to multiple event control blocks                */
#endif

#if ((SYS_Q_EN > 0) && (SYS_MAX_QS > 0)) || (SYS_MBOX_EN > 0)
    void            *SysTCBMsg;              /* Message received from SysMboxPost() or SysQPost()         */
#endif

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
#if SYS_TASK_DEL_EN > 0
    SYS_FLAG_NODE    *SysTCBFlagNode;         /* Pointer to event flag node                              */
#endif
    SYS_FLAGS         SysTCBFlagsRdy;         /* Event flags that made task ready to run                 */
#endif

    INT16U           SysTCBDly;              /* Nbr ticks to delay task or, timeout waiting for event   */
    INT8U            SysTCBStat;             /* Task      status                                        */
    INT8U            SysTCBStatPend;         /* Task PEND status                                        */
    INT8U            SysTCBPrio;             /* Task priority (0 == highest)                            */

    INT8U            SysTCBX;                /* Bit position in group  corresponding to task priority   */
    INT8U            SysTCBY;                /* Index into ready table corresponding to task priority   */
#if SYS_LOWEST_PRIO <= 63
    INT8U            SysTCBBitX;             /* Bit mask to access bit position in ready table          */
    INT8U            SysTCBBitY;             /* Bit mask to access bit position in ready group          */
#else
    INT16U           SysTCBBitX;             /* Bit mask to access bit position in ready table          */
    INT16U           SysTCBBitY;             /* Bit mask to access bit position in ready group          */
#endif

#if SYS_TASK_DEL_EN > 0
    INT8U            SysTCBDelReq;           /* Indicates whether a task needs to delete itself         */
#endif

#if SYS_TASK_PROFILE_EN > 0
    INT32U           SysTCBCtxSwCtr;         /* Number of time the task was switched in                 */
    INT32U           SysTCBCyclesTot;        /* Total number of clock cycles the task has been running  */
    INT32U           SysTCBCyclesStart;      /* Snapshot of cycle counter at start of task resumption   */
    SYS_STK          *SysTCBStkBase;          /* Pointer to the beginning of the task stack              */
    INT32U           SysTCBStkUsed;          /* Number of bytes used from the stack                     */
#endif

#if SYS_TASK_NAME_SIZE > 1
    INT8U            SysTCBTaskName[SYS_TASK_NAME_SIZE];
#endif
} SYS_TCB;

/*$PAGE*/
/*
************************************************************************************************************************
*                                                   TIMER DATA TYPES
************************************************************************************************************************
*/

#define SYS_TASK_TMR_PRIO					SYS_LOWEST_PRIO - 2

#if SYS_TMR_EN > 0
typedef  void (*SYS_TMR_CALLBACK)(void *ptmr, void *parg);



typedef  struct  os_tmr {
    INT8U            SysTmrType;                       /* Should be set to SYS_TMR_TYPE                                  */
    SYS_TMR_CALLBACK  SysTmrCallback;                   /* Function to call when timer expires                           */
    void            *SysTmrCallbackArg;                /* Argument to pass to function when timer expires               */
    void            *SysTmrNext;                       /* Double link list pointers                                     */
    void            *SysTmrPrev;
    INT32U           SysTmrMatch;                      /* Timer expires when SysTmrTime == SysTmrMatch                    */
    INT32U           SysTmrDly;                        /* Delay time before periodic update starts                      */
    INT32U           SysTmrPeriod;                     /* Period to repeat timer                                        */
#if SYS_TMR_CFG_NAME_SIZE > 0
    INT8U            SysTmrName[SYS_TMR_CFG_NAME_SIZE]; /* Name to give the timer                                        */
#endif
    INT8U            SysTmrOpt;                        /* Options (see SYS_TMR_OPT_xxx)                                  */
    INT8U            SysTmrState;                      /* Indicates the state of the timer:                             */
                                                      /*     SYS_TMR_STATE_UNUSED                                       */
                                                      /*     SYS_TMR_STATE_RUNNING                                      */
                                                      /*     SYS_TMR_STATE_STOPPED                                      */
} SYS_TMR;



typedef  struct  os_tmr_wheel {
    SYS_TMR          *SysTmrFirst;                      /* Pointer to first timer in linked list                         */
    INT16U           SysTmrEntries;
} SYS_TMR_WHEEL;
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            GLOBAL VARIABLES
*********************************************************************************************************
*/

SYS_EXT  INT32U            SysCtxSwCtr;               /* Counter of number of context switches           */

#if (SYS_EVENT_EN) && (SYS_MAX_EVENTS > 0)
SYS_EXT  SYS_EVENT         *SysEventFreeList;          /* Pointer to list of free EVENT control blocks    */
SYS_EXT  SYS_EVENT          SysEventTbl[SYS_MAX_EVENTS];/* Table of EVENT control blocks                   */
#endif

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
SYS_EXT  SYS_FLAG_GRP       SysFlagTbl[SYS_MAX_FLAGS];  /* Table containing event flag groups              */
SYS_EXT  SYS_FLAG_GRP      *SysFlagFreeList;           /* Pointer to free list of event flag groups       */
#endif

#if SYS_TASK_STAT_EN > 0
SYS_EXT  INT8U             SysCPUUsage;               /* Percentage of CPU used                          */
SYS_EXT  INT32U            SysIdleCtrMax;             /* Max. value that idle ctr can take in 1 sec.     */
SYS_EXT  INT32U            SysIdleCtrRun;             /* Val. reached by idle ctr at run time in 1 sec.  */
SYS_EXT  BOOLEAN           SysStatRdy;                /* Flag indicating that the statistic task is rdy  */
SYS_EXT  SYS_STK            SysTaskStatStk[SYS_TASK_STAT_STK_SIZE];      /* Statistics task stack          */
#endif

SYS_EXT  INT8U             SysIntNesting;             /* Interrupt nesting level                         */

SYS_EXT  INT8U             SysLockNesting;            /* Multitasking lock nesting level                 */

SYS_EXT  INT8U             SysPrioCur;                /* Priority of current task                        */
SYS_EXT  INT8U             SysPrioHighRdy;            /* Priority of highest priority task               */

#if SYS_LOWEST_PRIO <= 63
SYS_EXT  INT8U             SysRdyGrp;                        /* Ready list group                         */
SYS_EXT  INT8U             SysRdyTbl[SYS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */
#else
SYS_EXT  INT16U            SysRdyGrp;                        /* Ready list group                         */
SYS_EXT  INT16U            SysRdyTbl[SYS_RDY_TBL_SIZE];       /* Table of tasks which are ready to run    */
#endif

SYS_EXT  BOOLEAN           SysRunning;                       /* Flag indicating that kernel is running   */

SYS_EXT  INT8U             SysTaskCtr;                       /* Number of tasks created                  */

SYS_EXT  volatile  INT32U  SysIdleCtr;                                 /* Idle counter                   */

SYS_EXT  SYS_STK            SysTaskIdleStk[SYS_TASK_IDLE_STK_SIZE];      /* Idle task stack                */


SYS_EXT  SYS_TCB           *SysTCBCur;                        /* Pointer to currently running TCB         */
SYS_EXT  SYS_TCB           *SysTCBFreeList;                   /* Pointer to list of free TCBs             */
SYS_EXT  SYS_TCB           *SysTCBHighRdy;                    /* Pointer to highest priority TCB R-to-R   */
SYS_EXT  SYS_TCB           *SysTCBList;                       /* Pointer to doubly linked list of TCBs    */
SYS_EXT  SYS_TCB           *SysTCBPrioTbl[SYS_LOWEST_PRIO + 1];/* Table of pointers to created TCBs        */
SYS_EXT  SYS_TCB            SysTCBTbl[SYS_MAX_TASKS + SYS_N_SYS_TASKS];   /* Table of TCBs                  */

#if SYS_TICK_STEP_EN > 0
SYS_EXT  INT8U             SysTickStepState;          /* Indicates the state of the tick step feature    */
#endif

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
SYS_EXT  SYS_MEM           *SysMemFreeList;            /* Pointer to free list of memory partitions       */
SYS_EXT  SYS_MEM            SysMemTbl[SYS_MAX_MEM_PART];/* Storage for memory partition manager            */
#endif

#if (SYS_Q_EN > 0) && (SYS_MAX_QS > 0)
SYS_EXT  SYS_Q             *SysQFreeList;              /* Pointer to list of free QUEUE control blocks    */
SYS_EXT  SYS_Q              SysQTbl[SYS_MAX_QS];        /* Table of QUEUE control blocks                   */
#endif

#if SYS_TIME_GET_SET_EN > 0
SYS_EXT  volatile  INT32U  SysTime;                   /* Current value of system time (in ticks)         */
#endif

#if SYS_TMR_EN > 0
SYS_EXT  INT16U            SysTmrFree;                /* Number of free entries in the timer pool        */
SYS_EXT  INT16U            SysTmrUsed;                /* Number of timers used                           */
SYS_EXT  INT32U            SysTmrTime;                /* Current timer time                              */

SYS_EXT  SYS_EVENT         *SysTmrSem;                 /* Sem. used to gain exclusive access to timers    */
SYS_EXT  SYS_EVENT         *SysTmrSemSignal;           /* Sem. used to signal the update of timers        */

SYS_EXT  SYS_TMR            SysTmrTbl[SYS_TMR_CFG_MAX]; /* Table containing pool of timers                 */
SYS_EXT  SYS_TMR           *SysTmrFreeList;            /* Pointer to free list of timers                  */
SYS_EXT  SYS_STK            SysTmrTaskStk[SYS_TASK_TMR_STK_SIZE];

SYS_EXT  SYS_TMR_WHEEL      SysTmrWheelTbl[SYS_TMR_CFG_WHEEL_SIZE];
#endif

extern  INT8U   const     SysUnMapTbl[256];          /* Priority->Index    lookup table                 */

/*$PAGE*/
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                     (Target Independent Functions)
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            MISCELLANEOUS
*********************************************************************************************************
*/

#if (SYS_EVENT_EN)

#if (SYS_EVENT_NAME_SIZE > 1)
INT8U         SysEventNameGet          (SYS_EVENT        *pevent,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          SysEventNameSet          (SYS_EVENT        *pevent,
                                       INT8U           *pname,
                                       INT8U           *perr);
#endif

#if (SYS_EVENT_MULTI_EN > 0)
INT16U        SysEventPendMulti        (SYS_EVENT       **pevents_pend,
                                       SYS_EVENT       **pevents_rdy,
                                       void           **pmsgs_rdy,
                                       INT16U           timeout,
                                       INT8U           *perr);
#endif

#endif

/*
*********************************************************************************************************
*                                         EVENT FLAGS MANAGEMENT
*********************************************************************************************************
*/

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)

#if SYS_FLAG_ACCEPT_EN > 0
SYS_FLAGS      SysFlagAccept            (SYS_FLAG_GRP     *pgrp,
                                       SYS_FLAGS         flags,
                                       INT8U            wait_type,
                                       INT8U           *perr);
#endif

SYS_FLAG_GRP  *SysFlagCreate            (SYS_FLAGS         flags,
                                      INT8U            *perr);

#if SYS_FLAG_DEL_EN > 0
SYS_FLAG_GRP  *SysFlagDel               (SYS_FLAG_GRP     *pgrp,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

#if (SYS_FLAG_EN > 0) && (SYS_FLAG_NAME_SIZE > 1)
INT8U         SysFlagNameGet           (SYS_FLAG_GRP     *pgrp,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          SysFlagNameSet           (SYS_FLAG_GRP     *pgrp,
                                       INT8U           *pname,
                                       INT8U           *perr);
#endif

SYS_FLAGS      SysFlagPend              (SYS_FLAG_GRP     *pgrp,
                                       SYS_FLAGS         flags,
                                       INT8U            wait_type,
                                       INT16U           timeout,
                                       INT8U           *perr);

SYS_FLAGS      SysFlagPendGetFlagsRdy   (void);
SYS_FLAGS      SysFlagPost              (SYS_FLAG_GRP     *pgrp,
                                       SYS_FLAGS         flags,
                                       INT8U            opt,
                                       INT8U           *perr);

#if SYS_FLAG_QUERY_EN > 0
SYS_FLAGS      SysFlagQuery             (SYS_FLAG_GRP     *pgrp,
                                       INT8U           *perr);
#endif
#endif

/*
*********************************************************************************************************
*                                        MESSAGE MAILBOX MANAGEMENT
*********************************************************************************************************
*/

#if SYS_MBOX_EN > 0

#if SYS_MBOX_ACCEPT_EN > 0
void         *SysMboxAccept            (SYS_EVENT        *pevent);
#endif

SYS_EVENT     *SysMboxCreate            (void            *pmsg);

#if SYS_MBOX_DEL_EN > 0
SYS_EVENT     *SysMboxDel               (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

void         *SysMboxPend              (SYS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

#if SYS_MBOX_PEND_ABORT_EN > 0
INT8U         SysMboxPendAbort         (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

#if SYS_MBOX_PSysT_EN > 0
INT8U         SysMboxPost              (SYS_EVENT        *pevent,
                                       void            *pmsg);
#endif

#if SYS_MBOX_PSysT_OPT_EN > 0
INT8U         SysMboxPostOpt           (SYS_EVENT        *pevent,
                                       void            *pmsg,
                                       INT8U            opt);
#endif

#if SYS_MBOX_QUERY_EN > 0
INT8U         SysMboxQuery             (SYS_EVENT        *pevent,
                                       SYS_MBOX_DATA    *p_mbox_data);
#endif
#endif

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)

SYS_MEM       *SysMemCreate             (void            *addr,
                                       INT32U           nblks,
                                       INT32U           blksize,
                                       INT8U           *perr);

void         *SysMemGet                (SYS_MEM          *pmem,
                                       INT8U           *perr);
#if SYS_MEM_NAME_SIZE > 1
INT8U         SysMemNameGet            (SYS_MEM          *pmem,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          SysMemNameSet            (SYS_MEM          *pmem,
                                       INT8U           *pname,
                                       INT8U           *perr);
#endif
INT8U         SysMemPut                (SYS_MEM          *pmem,
                                       void            *pblk);

#if SYS_MEM_QUERY_EN > 0
INT8U         SysMemQuery              (SYS_MEM          *pmem,
                                       SYS_MEM_DATA     *p_mem_data);
#endif

#endif

/*
*********************************************************************************************************
*                                MUTUAL EXCLUSION SEMAPHORE MANAGEMENT
*********************************************************************************************************
*/

#if SYS_MUTEX_EN > 0

#if SYS_MUTEX_ACCEPT_EN > 0
BOOLEAN       SysMutexAccept           (SYS_EVENT        *pevent,
                                       INT8U           *perr);
#endif

SYS_EVENT     *SysMutexCreate           (INT8U            prio,
                                       INT8U           *perr);

#if SYS_MUTEX_DEL_EN > 0
SYS_EVENT     *SysMutexDel              (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

void          SysMutexPend             (SYS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

INT8U         SysMutexPost             (SYS_EVENT        *pevent);

#if SYS_MUTEX_QUERY_EN > 0
INT8U         SysMutexQuery            (SYS_EVENT        *pevent,
                                       SYS_MUTEX_DATA   *p_mutex_data);
#endif

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                         MESSAGE QUEUE MANAGEMENT
*********************************************************************************************************
*/

#if (SYS_Q_EN > 0) && (SYS_MAX_QS > 0)

#if SYS_Q_ACCEPT_EN > 0
void         *SysQAccept               (SYS_EVENT        *pevent,
                                       INT8U           *perr);
#endif

SYS_EVENT     *SysQCreate               (void           **start,
                                       INT16U           size);

#if SYS_Q_DEL_EN > 0
SYS_EVENT     *SysQDel                  (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

#if SYS_Q_FLUSH_EN > 0
INT8U         SysQFlush                (SYS_EVENT        *pevent);
#endif

void         *SysQPend                 (SYS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

#if SYS_Q_PEND_ABORT_EN > 0
INT8U         SysQPendAbort            (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

#if SYS_Q_PSysT_EN > 0
INT8U         SysQPost                 (SYS_EVENT        *pevent,
                                       void            *pmsg);
#endif

#if SYS_Q_PSysT_FRONT_EN > 0
INT8U         SysQPostFront            (SYS_EVENT        *pevent,
                                       void            *pmsg);
#endif

#if SYS_Q_PSysT_OPT_EN > 0
INT8U         SysQPostOpt              (SYS_EVENT        *pevent,
                                       void            *pmsg,
                                       INT8U            opt);
#endif

#if SYS_Q_QUERY_EN > 0
INT8U         SysQQuery                (SYS_EVENT        *pevent,
                                       SYS_Q_DATA       *p_q_data);
#endif

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          SEMAPHORE MANAGEMENT
*********************************************************************************************************
*/
#if SYS_SEM_EN > 0

#if SYS_SEM_ACCEPT_EN > 0
INT16U        SysSemAccept             (SYS_EVENT        *pevent);
#endif

SYS_EVENT     *SysSemCreate             (INT16U           cnt);

#if SYS_SEM_DEL_EN > 0
SYS_EVENT     *SysSemDel                (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

void          SysSemPend               (SYS_EVENT        *pevent,
                                       INT16U           timeout,
                                       INT8U           *perr);

#if SYS_SEM_PEND_ABORT_EN > 0
INT8U         SysSemPendAbort          (SYS_EVENT        *pevent,
                                       INT8U            opt,
                                       INT8U           *perr);
#endif

INT8U         SysSemPost               (SYS_EVENT        *pevent);

#if SYS_SEM_QUERY_EN > 0
INT8U         SysSemQuery              (SYS_EVENT        *pevent,
                                       SYS_SEM_DATA     *p_sem_data);
#endif

#if SYS_SEM_SET_EN > 0
void          SysSemSet                (SYS_EVENT        *pevent,
                                       INT16U           cnt,
                                       INT8U           *perr);
#endif

#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            TASK MANAGEMENT
*********************************************************************************************************
*/
#if SYS_TASK_CHANGE_PRIO_EN > 0
INT8U         SysTaskChangePrio        (INT8U            oldprio,
                                       INT8U            newprio);
#endif

#if SYS_TASK_CREATE_EN > 0
INT8U         SysTaskCreate            (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       SYS_STK          *ptos,
                                       INT8U            prio);
#endif

#if SYS_TASK_CREATE_EXT_EN > 0
INT8U         SysTaskCreateExt         (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       SYS_STK          *ptos,
                                       INT8U            prio,
                                       INT16U           id,
                                       SYS_STK          *pbos,
                                       INT32U           stk_size,
                                       void            *pext,
                                       INT16U           opt);
#endif

#if SYS_TASK_DEL_EN > 0
INT8U         SysTaskDel               (INT8U            prio);
INT8U         SysTaskDelReq            (INT8U            prio);
#endif

#if SYS_TASK_NAME_SIZE > 1
INT8U         SysTaskNameGet           (INT8U            prio,
                                       INT8U           *pname,
                                       INT8U           *perr);

void          SysTaskNameSet           (INT8U            prio,
                                       INT8U           *pname,
                                       INT8U           *perr);
#endif

#if SYS_TASK_SUSPEND_EN > 0
INT8U         SysTaskResume            (INT8U            prio);
INT8U         SysTaskSuspend           (INT8U            prio);
#endif

#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
INT8U         SysTaskStkChk            (INT8U            prio,
                                       SYS_STK_DATA     *p_stk_data);
#endif

#if SYS_TASK_QUERY_EN > 0
INT8U         SysTaskQuery             (INT8U            prio,
                                       SYS_TCB          *p_task_data);
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                            TIME MANAGEMENT
*********************************************************************************************************
*/

void          SysTimeDly               (INT16U           ticks);

#if SYS_TIME_DLY_HMSM_EN > 0
INT8U         SysTimeDlyHMSM           (INT8U            hours,
                                       INT8U            minutes,
                                       INT8U            seconds,
                                       INT16U           milli);
#endif

#if SYS_TIME_DLY_RESUME_EN > 0
INT8U         SysTimeDlyResume         (INT8U            prio);
#endif

#if SYS_TIME_GET_SET_EN > 0
INT32U        SysTimeGet               (void);
void          SysTimeSet               (INT32U           ticks);
#endif

void          SysTimeTick              (void);

/*
*********************************************************************************************************
*                                            TIMER MANAGEMENT
*********************************************************************************************************
*/

#if SYS_TMR_EN > 0
SYS_TMR      *SysTmrCreate              (INT32U           dly,
                                       INT32U           period,
                                       INT8U            opt,
                                       SYS_TMR_CALLBACK  callback,
                                       void            *callback_arg,
                                       INT8U           *pname,
                                       INT8U           *perr);

BOOLEAN      SysTmrDel                 (SYS_TMR          *ptmr,
                                       INT8U           *perr);

#if SYS_TMR_CFG_NAME_SIZE > 0
INT8U        SysTmrNameGet             (SYS_TMR          *ptmr,
                                       INT8U           *pdest,
                                       INT8U           *perr);
#endif
INT32U       SysTmrRemainGet           (SYS_TMR          *ptmr,
                                       INT8U           *perr);

INT8U        SysTmrStateGet            (SYS_TMR          *ptmr,
                                       INT8U           *perr);

BOOLEAN      SysTmrStart               (SYS_TMR          *ptmr,
                                       INT8U           *perr);

BOOLEAN      SysTmrStop                (SYS_TMR          *ptmr,
                                       INT8U            opt,
                                       void            *callback_arg,
                                       INT8U           *perr);

INT8U        SysTmrSignal              (void);
#endif

/*
*********************************************************************************************************
*                                             MISCELLANEOUS
*********************************************************************************************************
*/

void          SysInit                  (void);

void          SysIntEnter              (void);
void          SysIntExit               (void);

#if SYS_SCHED_LOCK_EN > 0
void          SysSchedLock             (void);
void          SysSchedUnlock           (void);
#endif

void          SysStart                 (void);

void          SysStatInit              (void);

INT16U        SysVersion               (void);

/*$PAGE*/
/*
*********************************************************************************************************
*                                      INTERNAL FUNCTION PROTOTYPES
*                            (Your application MUST NOT call these functions)
*********************************************************************************************************
*/

#if SYS_TASK_DEL_EN > 0
void          SYS_Dummy                (void);
#endif

#if (SYS_EVENT_EN)
INT8U         SYS_EventTaskRdy         (SYS_EVENT        *pevent,
                                       void            *pmsg,
                                       INT8U            msk,
                                       INT8U            pend_stat);

void          SYS_EventTaskWait        (SYS_EVENT        *pevent);

void          SYS_EventTaskRemove      (SYS_TCB          *ptcb,
                                       SYS_EVENT        *pevent);

#if (SYS_EVENT_MULTI_EN > 0)
void          SYS_EventTaskWaitMulti   (SYS_EVENT       **pevents_wait);

void          SYS_EventTaskRemoveMulti (SYS_TCB          *ptcb,
                                       SYS_EVENT       **pevents_multi);
#endif

void          SYS_EventWaitListInit    (SYS_EVENT        *pevent);
#endif

#if (SYS_FLAG_EN > 0) && (SYS_MAX_FLAGS > 0)
void          SYS_FlagInit             (void);
void          SYS_FlagUnlink           (SYS_FLAG_NODE    *pnode);
#endif

void          SYS_MemClr               (INT8U           *pdest,
                                       INT16U           size);

void          SYS_MemCopy              (INT8U           *pdest,
                                       INT8U           *psrc,
                                       INT16U           size);

#if (SYS_MEM_EN > 0) && (SYS_MAX_MEM_PART > 0)
void          SYS_MemInit              (void);
#endif

#if SYS_Q_EN > 0
void          SYS_QInit                (void);
#endif

void          SYS_Sched                (void);

#if (SYS_EVENT_NAME_SIZE > 1) || (SYS_FLAG_NAME_SIZE > 1) || (SYS_MEM_NAME_SIZE > 1) || (SYS_TASK_NAME_SIZE > 1)
INT8U         SYS_StrCopy              (INT8U           *pdest,
                                       INT8U           *psrc);

INT8U         SYS_StrLen               (INT8U           *psrc);
#endif

void          SYS_TaskIdle             (void            *p_arg);

#if SYS_TASK_STAT_EN > 0
void          SYS_TaskStat             (void            *p_arg);
#endif

#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
void          SYS_TaskStkClr           (SYS_STK          *pbos,
                                       INT32U           size,
                                       INT16U           opt);
#endif

#if (SYS_TASK_STAT_STK_CHK_EN > 0) && (SYS_TASK_CREATE_EXT_EN > 0)
void          SYS_TaskStatStkChk       (void);
#endif

INT8U         SYS_TCBInit              (INT8U            prio,
                                       SYS_STK          *ptos,
                                       SYS_STK          *pbos,
                                       INT16U           id,
                                       INT32U           stk_size,
                                       void            *pext,
                                       INT16U           opt);

#if SYS_TMR_EN > 0
void          SysTmr_Init              (void);
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                      (Target Specific Functions)
*********************************************************************************************************
*/

#if SYS_DEBUG_EN > 0
void          SysDebugInit             (void);
#endif

void          SysInitHookBegin         (void);
void          SysInitHookEnd           (void);

void          SysTaskCreateHook        (SYS_TCB          *ptcb);
void          SysTaskDelHook           (SYS_TCB          *ptcb);

void          SysTaskIdleHook          (void);

void          SysTaskStatHook          (void);
SYS_STK       *SysTaskStkInit           (void           (*task)(void *p_arg),
                                       void            *p_arg,
                                       SYS_STK          *ptos,
                                       INT16U           opt);

#if SYS_TASK_SW_HOOK_EN > 0
void          SysTaskSwHook            (void);
#endif

void          SysTCBInitHook           (SYS_TCB          *ptcb);

#if SYS_TIME_TICK_HOOK_EN > 0
void          SysTimeTickHook          (void);
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*                                   (Application Specific Functions)
*********************************************************************************************************
*/

#if SYS_APP_HOOKS_EN > 0
void          App_TaskCreateHook      (SYS_TCB          *ptcb);
void          App_TaskDelHook         (SYS_TCB          *ptcb);
void          App_TaskIdleHook        (void);

void          App_TaskStatHook        (void);

#if SYS_TASK_SW_HOOK_EN > 0
void          App_TaskSwHook          (void);
#endif

void          App_TCBInitHook         (SYS_TCB          *ptcb);

#if SYS_TIME_TICK_HOOK_EN > 0
void          App_TimeTickHook        (void);
#endif
#endif

/*
*********************************************************************************************************
*                                          FUNCTION PROTOTYPES
*
* IMPORTANT: These prototypes MUST be placed in SYS_CPU.H
*********************************************************************************************************
*/

#if 0
void          SysStartHighRdy          (void);
void          SysIntCtxSw              (void);
void          SysCtxSw                 (void);
#endif

/*$PAGE*/
/*
*********************************************************************************************************
*                                   LOOK FOR MISSING #define CONSTANTS
*
* This section is used to generate ERROR messages at compile time if certain #define constants are
* MISSING in SYS_CFG.H.  This allows you to quickly determine the source of the error.
*
* You SHOULD NOT change this section UNLESS you would like to add more comments as to the source of the
* compile time error.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                            EVENT FLAGS
*********************************************************************************************************
*/

#ifndef SYS_FLAG_EN
#error  "SYS_CFG.H, Missing SYS_FLAG_EN: Enable (1) or Disable (0) code generation for Event Flags"
#else
    #ifndef SYS_MAX_FLAGS
    #error  "SYS_CFG.H, Missing SYS_MAX_FLAGS: Max. number of Event Flag Groups in your application"
    #else
        #if     SYS_MAX_FLAGS > 65500u
        #error  "SYS_CFG.H, SYS_MAX_FLAGS must be <= 65500"
        #endif
    #endif

    #ifndef SYS_FLAGS_NBITS
    #error  "SYS_CFG.H, Missing SYS_FLAGS_NBITS: Determine #bits used for event flags, MUST be either 8, 16 or 32"
    #endif

    #ifndef SYS_FLAG_WAIT_CLR_EN
    #error  "SYS_CFG.H, Missing SYS_FLAG_WAIT_CLR_EN: Include code for Wait on Clear EVENT FLAGS"
    #endif

    #ifndef SYS_FLAG_ACCEPT_EN
    #error  "SYS_CFG.H, Missing SYS_FLAG_ACCEPT_EN: Include code for SysFlagAccept()"
    #endif

    #ifndef SYS_FLAG_DEL_EN
    #error  "SYS_CFG.H, Missing SYS_FLAG_DEL_EN: Include code for SysFlagDel()"
    #endif

    #ifndef SYS_FLAG_NAME_SIZE
    #error  "SYS_CFG.H, Missing SYS_FLAG_NAME_SIZE: Determines the size of flag group names"
    #endif

    #ifndef SYS_FLAG_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_FLAG_QUERY_EN: Include code for SysFlagQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                           MESSAGE MAILBOXES
*********************************************************************************************************
*/

#ifndef SYS_MBOX_EN
#error  "SYS_CFG.H, Missing SYS_MBOX_EN: Enable (1) or Disable (0) code generation for MAILBOXES"
#else
    #ifndef SYS_MBOX_ACCEPT_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_ACCEPT_EN: Include code for SysMboxAccept()"
    #endif

    #ifndef SYS_MBOX_DEL_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_DEL_EN: Include code for SysMboxDel()"
    #endif

    #ifndef SYS_MBOX_PEND_ABORT_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_PEND_ABORT_EN: Include code for SysMboxPendAbort()"
    #endif

    #ifndef SYS_MBOX_PSysT_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_PSysT_EN: Include code for SysMboxPost()"
    #endif

    #ifndef SYS_MBOX_PSysT_OPT_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_PSysT_OPT_EN: Include code for SysMboxPostOpt()"
    #endif

    #ifndef SYS_MBOX_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_MBOX_QUERY_EN: Include code for SysMboxQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                           MEMORY MANAGEMENT
*********************************************************************************************************
*/

#ifndef SYS_MEM_EN
#error  "SYS_CFG.H, Missing SYS_MEM_EN: Enable (1) or Disable (0) code generation for MEMORY MANAGER"
#else
    #ifndef SYS_MAX_MEM_PART
    #error  "SYS_CFG.H, Missing SYS_MAX_MEM_PART: Max. number of memory partitions"
    #else
        #if     SYS_MAX_MEM_PART > 65500u
        #error  "SYS_CFG.H, SYS_MAX_MEM_PART must be <= 65500"
        #endif
    #endif

    #ifndef SYS_MEM_NAME_SIZE
    #error  "SYS_CFG.H, Missing SYS_MEM_NAME_SIZE: Determines the size of memory partition names"
    #endif

    #ifndef SYS_MEM_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_MEM_QUERY_EN: Include code for SysMemQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                       MUTUAL EXCLUSION SEMAPHORES
*********************************************************************************************************
*/

#ifndef SYS_MUTEX_EN
#error  "SYS_CFG.H, Missing SYS_MUTEX_EN: Enable (1) or Disable (0) code generation for MUTEX"
#else
    #ifndef SYS_MUTEX_ACCEPT_EN
    #error  "SYS_CFG.H, Missing SYS_MUTEX_ACCEPT_EN: Include code for SysMutexAccept()"
    #endif

    #ifndef SYS_MUTEX_DEL_EN
    #error  "SYS_CFG.H, Missing SYS_MUTEX_DEL_EN: Include code for SysMutexDel()"
    #endif

    #ifndef SYS_MUTEX_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_MUTEX_QUERY_EN: Include code for SysMutexQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                              MESSAGE QUEUES
*********************************************************************************************************
*/

#ifndef SYS_Q_EN
#error  "SYS_CFG.H, Missing SYS_Q_EN: Enable (1) or Disable (0) code generation for QUEUES"
#else
    #ifndef SYS_MAX_QS
    #error  "SYS_CFG.H, Missing SYS_MAX_QS: Max. number of queue control blocks"
    #else
        #if     SYS_MAX_QS > 65500u
        #error  "SYS_CFG.H, SYS_MAX_QS must be <= 65500"
        #endif
    #endif

    #ifndef SYS_Q_ACCEPT_EN
    #error  "SYS_CFG.H, Missing SYS_Q_ACCEPT_EN: Include code for SysQAccept()"
    #endif

    #ifndef SYS_Q_DEL_EN
    #error  "SYS_CFG.H, Missing SYS_Q_DEL_EN: Include code for SysQDel()"
    #endif

    #ifndef SYS_Q_FLUSH_EN
    #error  "SYS_CFG.H, Missing SYS_Q_FLUSH_EN: Include code for SysQFlush()"
    #endif

    #ifndef SYS_Q_PEND_ABORT_EN
    #error  "SYS_CFG.H, Missing SYS_Q_PEND_ABORT_EN: Include code for SysQPendAbort()"
    #endif

    #ifndef SYS_Q_PSysT_EN
    #error  "SYS_CFG.H, Missing SYS_Q_PSysT_EN: Include code for SysQPost()"
    #endif

    #ifndef SYS_Q_PSysT_FRONT_EN
    #error  "SYS_CFG.H, Missing SYS_Q_PSysT_FRONT_EN: Include code for SysQPostFront()"
    #endif

    #ifndef SYS_Q_PSysT_OPT_EN
    #error  "SYS_CFG.H, Missing SYS_Q_PSysT_OPT_EN: Include code for SysQPostOpt()"
    #endif

    #ifndef SYS_Q_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_Q_QUERY_EN: Include code for SysQQuery()"
    #endif
#endif

/*
*********************************************************************************************************
*                                              SEMAPHORES
*********************************************************************************************************
*/

#ifndef SYS_SEM_EN
#error  "SYS_CFG.H, Missing SYS_SEM_EN: Enable (1) or Disable (0) code generation for SEMAPHORES"
#else
    #ifndef SYS_SEM_ACCEPT_EN
    #error  "SYS_CFG.H, Missing SYS_SEM_ACCEPT_EN: Include code for SysSemAccept()"
    #endif

    #ifndef SYS_SEM_DEL_EN
    #error  "SYS_CFG.H, Missing SYS_SEM_DEL_EN: Include code for SysSemDel()"
    #endif

    #ifndef SYS_SEM_PEND_ABORT_EN
    #error  "SYS_CFG.H, Missing SYS_SEM_PEND_ABORT_EN: Include code for SysSemPendAbort()"
    #endif

    #ifndef SYS_SEM_QUERY_EN
    #error  "SYS_CFG.H, Missing SYS_SEM_QUERY_EN: Include code for SysSemQuery()"
    #endif

    #ifndef SYS_SEM_SET_EN
    #error  "SYS_CFG.H, Missing SYS_SEM_SET_EN: Include code for SysSemSet()"
    #endif
#endif

/*
*********************************************************************************************************
*                                             TASK MANAGEMENT
*********************************************************************************************************
*/

#ifndef SYS_MAX_TASKS
#error  "SYS_CFG.H, Missing SYS_MAX_TASKS: Max. number of tasks in your application"
#else
    #if     SYS_MAX_TASKS < 2
    #error  "SYS_CFG.H,         SYS_MAX_TASKS must be >= 2"
    #endif

    #if     SYS_MAX_TASKS >  ((SYS_LOWEST_PRIO - SYS_N_SYS_TASKS) + 1)
    #error  "SYS_CFG.H,         SYS_MAX_TASKS must be <= SYS_LOWEST_PRIO - SYS_N_SYS_TASKS + 1"
    #endif

#endif

#if     SYS_LOWEST_PRIO >  254
#error  "SYS_CFG.H,         SYS_LOWEST_PRIO must be <= 254 in V2.8x and higher"
#endif

#ifndef SYS_TASK_IDLE_STK_SIZE
#error  "SYS_CFG.H, Missing SYS_TASK_IDLE_STK_SIZE: Idle task stack size"
#endif

#ifndef SYS_TASK_STAT_EN
#error  "SYS_CFG.H, Missing SYS_TASK_STAT_EN: Enable (1) or Disable(0) the statistics task"
#endif

#ifndef SYS_TASK_STAT_STK_SIZE
#error  "SYS_CFG.H, Missing SYS_TASK_STAT_STK_SIZE: Statistics task stack size"
#endif

#ifndef SYS_TASK_STAT_STK_CHK_EN
#error  "SYS_CFG.H, Missing SYS_TASK_STAT_STK_CHK_EN: Check task stacks from statistics task"
#endif

#ifndef SYS_TASK_CHANGE_PRIO_EN
#error  "SYS_CFG.H, Missing SYS_TASK_CHANGE_PRIO_EN: Include code for SysTaskChangePrio()"
#endif

#ifndef SYS_TASK_CREATE_EN
#error  "SYS_CFG.H, Missing SYS_TASK_CREATE_EN: Include code for SysTaskCreate()"
#endif

#ifndef SYS_TASK_CREATE_EXT_EN
#error  "SYS_CFG.H, Missing SYS_TASK_CREATE_EXT_EN: Include code for SysTaskCreateExt()"
#endif

#ifndef SYS_TASK_DEL_EN
#error  "SYS_CFG.H, Missing SYS_TASK_DEL_EN: Include code for SysTaskDel()"
#endif

#ifndef SYS_TASK_NAME_SIZE
#error  "SYS_CFG.H, Missing SYS_TASK_NAME_SIZE: Determine the size of task names"
#endif

#ifndef SYS_TASK_SUSPEND_EN
#error  "SYS_CFG.H, Missing SYS_TASK_SUSPEND_EN: Include code for SysTaskSuspend() and SysTaskResume()"
#endif

#ifndef SYS_TASK_QUERY_EN
#error  "SYS_CFG.H, Missing SYS_TASK_QUERY_EN: Include code for SysTaskQuery()"
#endif

/*
*********************************************************************************************************
*                                             TIME MANAGEMENT
*********************************************************************************************************
*/

#ifndef SYS_TICKS_PER_SEC
#error  "SYS_CFG.H, Missing SYS_TICKS_PER_SEC: Sets the number of ticks in one second"
#endif

#ifndef SYS_TIME_DLY_HMSM_EN
#error  "SYS_CFG.H, Missing SYS_TIME_DLY_HMSM_EN: Include code for SysTimeDlyHMSM()"
#endif

#ifndef SYS_TIME_DLY_RESUME_EN
#error  "SYS_CFG.H, Missing SYS_TIME_DLY_RESUME_EN: Include code for SysTimeDlyResume()"
#endif

#ifndef SYS_TIME_GET_SET_EN
#error  "SYS_CFG.H, Missing SYS_TIME_GET_SET_EN: Include code for SysTimeGet() and SysTimeSet()"
#endif

/*
*********************************************************************************************************
*                                             TIMER MANAGEMENT
*********************************************************************************************************
*/

#ifndef SYS_TMR_EN
#error  "SYS_CFG.H, Missing SYS_TMR_EN: When (1) enables code generation for Timer Management"
#elif   SYS_TMR_EN > 0
    #if     SYS_SEM_EN == 0
    #error  "SYS_CFG.H, Semaphore management is required (set SYS_SEM_EN to 1) when enabling Timer Management."
    #error  "          Timer management require TWO semaphores."
    #endif

    #ifndef SYS_TMR_CFG_MAX
    #error  "SYS_CFG.H, Missing SYS_TMR_CFG_MAX: Determines the total number of timers in an application (2 .. 65500)"
    #else
        #if SYS_TMR_CFG_MAX < 2
        #error  "SYS_CFG.H, SYS_TMR_CFG_MAX should be between 2 and 65500"
        #endif

        #if SYS_TMR_CFG_MAX > 65500
        #error  "SYS_CFG.H, SYS_TMR_CFG_MAX should be between 2 and 65500"
        #endif
    #endif

    #ifndef SYS_TMR_CFG_WHEEL_SIZE
    #error  "SYS_CFG.H, Missing SYS_TMR_CFG_WHEEL_SIZE: Sets the size of the timer wheel (1 .. 1023)"
    #else
        #if SYS_TMR_CFG_WHEEL_SIZE < 2
        #error  "SYS_CFG.H, SYS_TMR_CFG_WHEEL_SIZE should be between 2 and 1024"
        #endif

        #if SYS_TMR_CFG_WHEEL_SIZE > 1024
        #error  "SYS_CFG.H, SYS_TMR_CFG_WHEEL_SIZE should be between 2 and 1024"
        #endif
    #endif

    #ifndef SYS_TMR_CFG_NAME_SIZE
    #error  "SYS_CFG.H, Missing SYS_TMR_CFG_NAME_SIZE: Determines the number of characters used for Timer names"
    #endif

    #ifndef SYS_TMR_CFG_TICKS_PER_SEC
    #error  "SYS_CFG.H, Missing SYS_TMR_CFG_TICKS_PER_SEC: Determines the rate at which tiem timer management task will run (Hz)"
    #endif

    #ifndef SYS_TASK_TMR_STK_SIZE
    #error  "SYS_CFG.H, Missing SYS_TASK_TMR_STK_SIZE: Determines the size of the Timer Task's stack"
    #endif
#endif


/*
*********************************************************************************************************
*                                            MISCELLANEOUS
*********************************************************************************************************
*/

#ifndef SYS_ARG_CHK_EN
#error  "SYS_CFG.H, Missing SYS_ARG_CHK_EN: Enable (1) or Disable (0) argument checking"
#endif


#ifndef SYS_CPU_HOOKS_EN
#error  "SYS_CFG.H, Missing SYS_CPU_HOOKS_EN: uC/Sys-II hooks are found in the processor port files when 1"
#endif


#ifndef SYS_APP_HOOKS_EN
#error  "SYS_CFG.H, Missing SYS_APP_HOOKS_EN: Application-defined hooks are called from the uC/Sys-II hooks"
#endif


#ifndef SYS_DEBUG_EN
#error  "SYS_CFG.H, Missing SYS_DEBUG_EN: Allows you to include variables for debugging or not"
#endif


#ifndef SYS_LOWEST_PRIO
#error  "SYS_CFG.H, Missing SYS_LOWEST_PRIO: Defines the lowest priority that can be assigned"
#endif


#ifndef SYS_MAX_EVENTS
#error  "SYS_CFG.H, Missing SYS_MAX_EVENTS: Max. number of event control blocks in your application"
#else
    #if     SYS_MAX_EVENTS > 65500u
    #error  "SYS_CFG.H, SYS_MAX_EVENTS must be <= 65500"
    #endif
#endif


#ifndef SYS_SCHED_LOCK_EN
#error  "SYS_CFG.H, Missing SYS_SCHED_LOCK_EN: Include code for SysSchedLock() and SysSchedUnlock()"
#endif


#ifndef SYS_EVENT_MULTI_EN
#error  "SYS_CFG.H, Missing SYS_EVENT_MULTI_EN: Include code for SysEventPendMulti()"
#endif


#ifndef SYS_TASK_PROFILE_EN
#error  "SYS_CFG.H, Missing SYS_TASK_PROFILE_EN: Include data structure for run-time task profiling"
#endif


#ifndef SYS_TASK_SW_HOOK_EN
#error  "SYS_CFG.H, Missing SYS_TASK_SW_HOOK_EN: Allows you to include the code for SysTaskSwHook() or not"
#endif


#ifndef SYS_TICK_STEP_EN
#error  "SYS_CFG.H, Missing SYS_TICK_STEP_EN: Allows to 'step' one tick at a time with uC/Sys-View"
#endif


#ifndef SYS_TIME_TICK_HOOK_EN
#error  "SYS_CFG.H, Missing SYS_TIME_TICK_HOOK_EN: Allows you to include the code for SysTimeTickHook() or not"
#endif

/*
*********************************************************************************************************
*                                         SAFETY CRITICAL USE
*********************************************************************************************************
*/

#ifdef SAFETY_CRITICAL_RELEASE

#if    SYS_ARG_CHK_EN < 1
#error "SYS_CFG.H, SYS_ARG_CHK_EN must be enabled for safety-critical release code"
#endif

#if    SYS_APP_HOOKS_EN > 0
#error "SYS_CFG.H, SYS_APP_HOOKS_EN must be disabled for safety-critical release code"
#endif

#if    SYS_DEBUG_EN > 0
#error "SYS_CFG.H, SYS_DEBUG_EN must be disabled for safety-critical release code"
#endif

#ifdef CANTATA
#error "SYS_CFG.H, CANTATA must be disabled for safety-critical release code"
#endif

#ifdef SYS_SCHED_LOCK_EN
#error "SYS_CFG.H, SYS_SCHED_LOCK_EN must be disabled for safety-critical release code"
#endif

#ifdef VSC_VALIDATION_MODE
#error "SYS_CFG.H, VSC_VALIDATION_MODE must be disabled for safety-critical release code"
#endif

#if    SYS_TASK_STAT_EN > 0
#error "SYS_CFG.H, SYS_TASK_STAT_EN must be disabled for safety-critical release code"
#endif

#if    SYS_TICK_STEP_EN > 0
#error "SYS_CFG.H, SYS_TICK_STEP_EN must be disabled for safety-critical release code"
#endif

#if    SYS_FLAG_EN > 0
    #if    SYS_FLAG_DEL_EN > 0
    #error "SYS_CFG.H, SYS_FLAG_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_MBOX_EN > 0
    #if    SYS_MBOX_DEL_EN > 0
    #error "SYS_CFG.H, SYS_MBOX_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_MUTEX_EN > 0
    #if    SYS_MUTEX_DEL_EN > 0
    #error "SYS_CFG.H, SYS_MUTEX_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_Q_EN > 0
    #if    SYS_Q_DEL_EN > 0
    #error "SYS_CFG.H, SYS_Q_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_SEM_EN > 0
    #if    SYS_SEM_DEL_EN > 0
    #error "SYS_CFG.H, SYS_SEM_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_TASK_EN > 0
    #if    SYS_TASK_DEL_EN > 0
    #error "SYS_CFG.H, SYS_TASK_DEL_EN must be disabled for safety-critical release code"
    #endif
#endif

#if    SYS_CRITICAL_METHOD != 3
#error "SYS_CPU.H, SYS_CRITICAL_METHOD must be type 3 for safety-critical release code"
#endif

#endif  /* ------------------------ SAFETY_CRITICAL_RELEASE ------------------------ */

#ifdef __cplusplus
}
#endif

#endif
