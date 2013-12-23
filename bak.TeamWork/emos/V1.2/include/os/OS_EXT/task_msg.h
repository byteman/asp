/* 
 * File:    task_msg.h
 * Brief:   task message defined
 */
#ifndef TASK_MSG_H
#define TASK_MSG_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"

typedef struct _MsgQ 
{
	SYS_MEM *pMem;
	SYS_EVENT *pEvent;
}MsgQ;

int8 Task_Msg_Create(MsgQ *pMsgQ, void ** pSysQ, void *pMemBlock, uint32 nMsgCount, uint32 nMsgSize);

uint8 Task_QPost(MsgQ *pMsgQ, void *pMsg);

uint8 Task_QPostFront(MsgQ *pMsgQ, void *pMsg);

uint8 Task_QPend(MsgQ *pMsgQ, void *pMsg, int len, uint16 timeout);


#ifdef __cplusplus
}
#endif

#endif


