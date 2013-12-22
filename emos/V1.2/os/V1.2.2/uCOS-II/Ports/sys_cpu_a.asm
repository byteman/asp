;********************************************************************************************************
;                                               uC/Sys-II
;                                         The Real-Time Kernel
;
;                               (c) Copyright 1992-2006, Micrium, Weston, FL
;                                          All Rights Reserved
;
;                                           Generic ARM Port
;
; File      : SYS_CPU_A.ASM
; Version   : V2.86
; By        : Jean J. Labrosse
;
; For       : ARMv7M Cortex-M3
; Mode      : Thumb2
; Toolchain : RealView Development Suite
;             RealView Microcontroller Development Kit (MDK)
;             ARM Developer Suite (ADS)
;             Keil uVision
;********************************************************************************************************

;********************************************************************************************************
;                                           PUBLIC FUNCTIONS
;********************************************************************************************************

    EXTERN  SysRunning                                           ; External references
    EXTERN  SysPrioCur
    EXTERN  SysPrioHighRdy
    EXTERN  SysTCBCur
    EXTERN  SysTCBHighRdy
    EXTERN  SysIntNesting
    EXTERN  SysIntExit
    EXTERN  SysTaskSwHook


    EXPORT  SYS_CPU_SR_Save                                      ; Functions declared in this file
    EXPORT  SYS_CPU_SR_Restore
    EXPORT  SysStartHighRdy
    EXPORT  SysCtxSw
    EXPORT  SysIntCtxSw
    EXPORT  SYS_CPU_PendSVHandler

;********************************************************************************************************
;                                                EQUATES
;********************************************************************************************************

NVIC_INT_CTRL   EQU     0xE000ED04                              ; Interrupt control state register.
NVIC_SYSPRI14   EQU     0xE000ED22                              ; System priority register (priority 14).
NVIC_PENDSV_PRI EQU           0xFF                              ; PendSV priority value (lowest).
NVIC_PENDSVSET  EQU     0x10000000                              ; Value to trigger PendSV exception.

;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

    AREA |.text|, CODE, READONLY, ALIGN=2
    THUMB
    REQUIRE8
    PRESERVE8

;********************************************************************************************************
;                                   CRITICAL SECTION METHOD 3 FUNCTIONS
;
; Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
;              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
;              disable interrupts.  'cpu_sr' is allocated in all of uC/Sys-II's functions that need to
;              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
;              into the CPU's status register.
;
; Prototypes :     SYS_CPU_SR  SYS_CPU_SR_Save(void);
;                  void       SYS_CPU_SR_Restore(SYS_CPU_SR cpu_sr);
;
;
; Note(s)    : 1) These functions are used in general like this:
;
;                 void Task (void *p_arg)
;                 {
;                 #if SYS_CRITICAL_METHOD == 3          /* Allocate storage for CPU status register */
;                     SYS_CPU_SR  cpu_sr;
;                 #endif
;
;                          :
;                          :
;                     SYS_ENTER_CRITICAL();             /* cpu_sr = SYS_CPU_SaveSR();                */
;                          :
;                          :
;                     SYS_EXIT_CRITICAL();              /* SYS_CPU_RestoreSR(cpu_sr);                */
;                          :
;                          :
;                 }
;********************************************************************************************************

SYS_CPU_SR_Save
    MRS     R0, PRIMASK                                         ; Set prio int mask to mask all (except faults)
    CPSID   I
    BX      LR

SYS_CPU_SR_Restore
    MSR     PRIMASK, R0
    BX      LR

;********************************************************************************************************
;                                          START MULTITASKING
;                                       void SysStartHighRdy(void)
;
; Note(s) : 1) This function triggers a PendSV exception (essentially, causes a context switch) to cause
;              the first task to start.
;
;           2) SysStartHighRdy() MUST:
;              a) Setup PendSV exception priority to lowest;
;              b) Set initial PSP to 0, to tell context switcher this is first run;
;              c) Set SysRunning to TRUE;
;              d) Trigger PendSV exception;
;              e) Enable interrupts (tasks will run with interrupts enabled).
;********************************************************************************************************

SysStartHighRdy
    LDR     R0, =NVIC_SYSPRI14                                  ; Set the PendSV exception priority
    LDR     R1, =NVIC_PENDSV_PRI
    STRB    R1, [R0]

    MOVS    R0, #0                                              ; Set the PSP to 0 for initial context switch call
    MSR     PSP, R0

    LDR     R0, =SysRunning                                      ; SysRunning = TRUE
    MOVS    R1, #1
    STRB    R1, [R0]

    LDR     R0, =NVIC_INT_CTRL                                  ; Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]

    CPSIE   I                                                   ; Enable interrupts at processor level

SysStartHang
    B       SysStartHang                                         ; Should never get here


;********************************************************************************************************
;                               PERFORM A CONTEXT SWITCH (From task level)
;                                           void SysCtxSw(void)
;
; Note(s) : 1) SysCtxSw() is called when Sys wants to perform a task context switch.  This function
;              triggers the PendSV exception which is where the real work is done.
;********************************************************************************************************

SysCtxSw
    LDR     R0, =NVIC_INT_CTRL                                  ; Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR

;********************************************************************************************************
;                             PERFORM A CONTEXT SWITCH (From interrupt level)
;                                         void SysIntCtxSw(void)
;
; Notes:    1) SysIntCtxSw() is called by SysIntExit() when it determines a context switch is needed as
;              the result of an interrupt.  This function simply triggers a PendSV exception which will
;              be handled when there are no more interrupts active and interrupts are enabled.
;********************************************************************************************************

SysIntCtxSw
    LDR     R0, =NVIC_INT_CTRL                                  ; Trigger the PendSV exception (causes context switch)
    LDR     R1, =NVIC_PENDSVSET
    STR     R1, [R0]
    BX      LR

;********************************************************************************************************
;                                         HANDLE PendSV EXCEPTION
;                                     void SYS_CPU_PendSVHandler(void)
;
; Note(s) : 1) PendSV is used to cause a context switch.  This is a recommended method for performing
;              context switches with Cortex-M3.  This is because the Cortex-M3 auto-saves half of the
;              processor context on any exception, and restores same on return from exception.  So only
;              saving of R4-R11 is required and fixing up the stack pointers.  Using the PendSV exception
;              this way means that context saving and restoring is identical whether it is initiated from
;              a thread or occurs due to an interrupt or exception.
;
;           2) Pseudo-code is:
;              a) Get the process SP, if 0 then skip (goto d) the saving part (first context switch);
;              b) Save remaining regs r4-r11 on process stack;
;              c) Save the process SP in its TCB, SysTCBCur->SysTCBStkPtr = SP;
;              d) Call SysTaskSwHook();
;              e) Get current high priority, SysPrioCur = SysPrioHighRdy;
;              f) Get current ready thread TCB, SysTCBCur = SysTCBHighRdy;
;              g) Get new process SP from TCB, SP = SysTCBHighRdy->SysTCBStkPtr;
;              h) Restore R4-R11 from new process stack;
;              i) Perform exception return which will restore remaining context.
;
;           3) On entry into PendSV handler:
;              a) The following have been saved on the process stack (by processor):
;                 xPSR, PC, LR, R12, R0-R3
;              b) Processor mode is switched to Handler mode (from Thread mode)
;              c) Stack is Main stack (switched from Process stack)
;              d) SysTCBCur      points to the SYS_TCB of the task to suspend
;                 SysTCBHighRdy  points to the SYS_TCB of the task to resume
;
;           4) Since PendSV is set to lowest priority in the system (by SysStartHighRdy() above), we
;              know that it will only be run when no other exception or interrupt is active, and
;              therefore safe to assume that context being switched out was using the process stack (PSP).
;********************************************************************************************************

SYS_CPU_PendSVHandler
    CPSID   I                                                   ; Prevent interruption during context switch
    MRS     R0, PSP                                             ; PSP is process stack pointer
    CBZ     R0, SYS_CPU_PendSVHandler_nosave                     ; Skip register save the first time

    SUBS    R0, R0, #0x20                                       ; Save remaining regs r4-11 on process stack
    STM     R0, {R4-R11}

	;add by kady
    SUBS    R0, R0, #0x40                                       ; Save remaining regs s16-s31 on process stack
	VSTM    R0, {S16-S31}
    ;add by kady end


    LDR     R1, =SysTCBCur                                       ; SysTCBCur->SysTCBStkPtr = SP;
    LDR     R1, [R1]
    STR     R0, [R1]                                            ; R0 is SP of process being switched out

                                                                ; At this point, entire context of process has been saved
SYS_CPU_PendSVHandler_nosave
    PUSH    {R14}                                               ; Save LR exc_return value
    LDR     R0, =SysTaskSwHook                                   ; SysTaskSwHook();
    BLX     R0
    POP     {R14}

    LDR     R0, =SysPrioCur                                      ; SysPrioCur = SysPrioHighRdy;
    LDR     R1, =SysPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, =SysTCBCur                                       ; SysTCBCur  = SysTCBHighRdy;
    LDR     R1, =SysTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     R0, [R2]                                            ; R0 is new process SP; SP = SysTCBHighRdy->SysTCBStkPtr;

	;add by kady
    VLDM    R0, {S16-S31}
	ADDS    R0, R0, #0x40                                       ; Restore s16-s31 from new process stack
    ;add by kady end


    LDM     R0, {R4-R11}                                        ; Restore r4-11 from new process stack
    ADDS    R0, R0, #0x20
    MSR     PSP, R0                                             ; Load PSP with new process SP
    ORR     LR, LR, #0x04                                       ; Ensure exception return uses process stack
    CPSIE   I
    BX      LR                                                  ; Exception return will restore remaining context

    END
