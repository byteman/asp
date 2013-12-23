/*
 * Copyright (c) 2001-2003 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __SYS_RTXC_H__
#define __SYS_RTXC_H__

#include "C2.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"


#define T_LWIP_STKSIZE      	512
#define LWIP_TASK_MAX    		1
//#define T_LWIP_START_PRIO  		7

#define MAX_QUEUES        20//number of mboxs
#define MAX_QUEUE_ENTRIES 20
typedef struct {
    SYS_EVENT*   pQ;
    void*       pvQEntries[MAX_QUEUE_ENTRIES];
} TQ_DESCR, *PQ_DESCR;

#if 0
//typedef xSemaphoreHandle sys_sem_t;
//typedef xQueueHandle sys_mbox_t;
//typedef xTaskHandle sys_thread_t;
#else
typedef SYS_EVENT*   sys_sem_t;
typedef PQ_DESCR    sys_mbox_t;
//typedef sys_mbox_t  sys_prot_t;

typedef u8_t        sys_thread_t;
#endif

#if 0
#define SYS_MBOX_NULL (xQueueHandle)0
#define SYS_SEM_NULL  (xSemaphoreHandle)0
#define SYS_DEFAULT_THREAD_STACK_DEPTH	configMINIMAL_STACK_SIZE
#else
#define SYS_MBOX_NULL (sys_mbox_t)0
#define SYS_SEM_NULL  (sys_sem_t)0
 #endif
 
#if 0
typedef struct _sys_arch_state_t
{
	// Task creation data.
	char cTaskName[configMAX_TASK_NAME_LEN];
	unsigned short nStackDepth;
	unsigned short nTaskCount;
} sys_arch_state_t;
#endif


//extern sys_arch_state_t s_sys_arch_state;

//void sys_set_default_state();
//void sys_set_state(signed char *pTaskName, unsigned short nStackSize);

/* Message queue constants. */
#define archMESG_QUEUE_LENGTH	( 6 )
#endif /* __SYS_RTXC_H__ */

