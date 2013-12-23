/*
*********************************************************************************************************
*                                               uC/Sys-II
*                                         The Real-Time Kernel
*
*
*                                (c) Copyright 2006, Micrium, Weston, FL
*                                          All Rights Reserved
*
*                                           ARM Cortex-M3 Port
*
* File      : SYS_CPU.H
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

#ifndef  SYS_CPU_H
#define  SYS_CPU_H


#ifdef   SYS_CPU_GLOBALS
#define  SYS_CPU_EXT
#else
#define  SYS_CPU_EXT  extern
#endif

/*
*********************************************************************************************************
*                                              DATA TYPES
*                                         (Compiler Specific)
*********************************************************************************************************
*/

typedef unsigned char  BOOLEAN;
typedef unsigned char  INT8U;                    /* Unsigned  8 bit quantity                           */
typedef signed   char  INT8S;                    /* Signed    8 bit quantity                           */
typedef unsigned short INT16U;                   /* Unsigned 16 bit quantity                           */
typedef signed   short INT16S;                   /* Signed   16 bit quantity                           */
typedef unsigned int   INT32U;                   /* Unsigned 32 bit quantity                           */
typedef signed   int   INT32S;                   /* Signed   32 bit quantity                           */
typedef float          FP32;                     /* Single precision floating point                    */
typedef double         FP64;                     /* Double precision floating point                    */

typedef unsigned int   SYS_STK;                   /* Each stack entry is 32-bit wide                    */
typedef unsigned int   SYS_CPU_SR;                /* Define size of CPU status register (PSR = 32 bits) */


//add by kady
typedef INT8U   BYTE ;          //兼容以前版本的数据类型
typedef INT16U  WORD ;
typedef INT32U  LONG;
typedef	INT32U  DWORD;

typedef	INT8U	uint8;		
typedef	INT8S	int8;		
typedef	INT16U	uint16;		
typedef	INT16S	int16;		
typedef	INT32U	uint32;	
typedef	INT32S	int32;		
typedef	FP32	fp32;		
typedef	FP64	fp64;
	
#define reentrant
#define SYS_OPT_SEG
#define idata
#define xdata

#ifndef BOOL
#define	BOOL			BOOLEAN
#endif

#ifndef FALSE
#define FALSE	0
#define TRUE	1
#endif 




//end by kady





/*
*********************************************************************************************************
*                                              Cortex-M1
*                                      Critical Section Management
*
* Method #1:  Disable/Enable interrupts using simple instructions.  After critical section, interrupts
*             will be enabled even if they were disabled before entering the critical section.
*             NOT IMPLEMENTED
*
* Method #2:  Disable/Enable interrupts by preserving the state of interrupts.  In other words, if
*             interrupts were disabled before entering the critical section, they will be disabled when
*             leaving the critical section.
*             NOT IMPLEMENTED
*
* Method #3:  Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
*             would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
*             disable interrupts.  'cpu_sr' is allocated in all of uC/Sys-II's functions that need to
*             disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
*             into the CPU's status register.
*********************************************************************************************************
*/

#define  SYS_CRITICAL_METHOD   3

#if SYS_CRITICAL_METHOD == 3
#define  SYS_ENTER_CRITICAL()  {cpu_sr = SYS_CPU_SR_Save();}
#define  SYS_EXIT_CRITICAL()   {SYS_CPU_SR_Restore(cpu_sr);}
#endif

/*
*********************************************************************************************************
*                                        Cortex-M3 Miscellaneous
*********************************************************************************************************
*/

#define  SYS_STK_GROWTH        1                   /* Stack grows from HIGH to LOW memory on ARM        */

#define  SYS_TASK_SW()         SysCtxSw()

/*
*********************************************************************************************************
*                                              PROTOTYPES
*********************************************************************************************************
*/

#if SYS_CRITICAL_METHOD == 3                       /* See SYS_CPU_A.ASM                                  */
SYS_CPU_SR  SYS_CPU_SR_Save(void);
void       SYS_CPU_SR_Restore(SYS_CPU_SR cpu_sr);
#endif

void       SysCtxSw(void);
void       SysIntCtxSw(void);
void       SysStartHighRdy(void);

void       SYS_CPU_PendSVHandler(void);

                                                  /* See SYS_CPU_C.C                                    */
void       SYS_CPU_SysTickHandler(void);
void       SYS_CPU_SysTickInit(void);

                                                  /* See BSP.C                                         */
INT32U     SYS_CPU_SysTickClkFreq(void);
#endif
