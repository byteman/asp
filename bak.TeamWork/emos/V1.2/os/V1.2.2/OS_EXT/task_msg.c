/*
 * History:
 * 1. 2012-12-24 修改消息队列发送数据队列维护异常的bug!
 * 2. 2013-2-5   重新封装系统消息队列
 */

#include "includes.h"
#include "task_msg.h"

int8 Task_Msg_Create(MsgQ *pMsgQ, void ** pSysQ, void *pMemBlock, uint32 nMsgCount, uint32 nMsgSize)
{
	uint8 err;

	if((pMsgQ == NULL) || (pSysQ == NULL) || (pMemBlock == NULL)) return -1;
	if((nMsgCount < 2) || (nMsgSize < 4)) return -1;

	pMsgQ->pMem = SysMemCreate(pMemBlock,nMsgCount,nMsgSize,&err);
	pMsgQ->pEvent = SysQCreate(pSysQ, nMsgCount);

  	if(pMsgQ->pEvent == NULL) return -2;

	return err;
}


uint8 Task_QPost(MsgQ *pMsgQ, void *pMsg)
{
	void *pBuf = NULL;
	uint8 err;

	pBuf = SysMemGet(pMsgQ->pMem, &err);
	if(pBuf != NULL) 
	{
		memcpy(pBuf, pMsg, pMsgQ->pMem->SysMemBlkSize);
		err = SysQPost(pMsgQ->pEvent, pBuf); 

	  	if(err != SYS_ERR_NONE) 
	  		SysMemPut(pMsgQ->pMem, pBuf);
	  		
		return err;		
	}

	return err;
}

uint8 Task_QPostFront(MsgQ *pMsgQ, void *pMsg)
{
	void *pBuf = NULL;
	uint8 err;

	pBuf = SysMemGet(pMsgQ->pMem, &err);
	if(pBuf != NULL) 
	{
		memcpy(pBuf, pMsg, pMsgQ->pMem->SysMemBlkSize);
		err = SysQPostFront(pMsgQ->pEvent, pBuf); 

	  	if(err != SYS_ERR_NONE) 
	  		SysMemPut(pMsgQ->pMem, pBuf);
	  		
		return err;		
	}

	return err;
}


uint8 Task_QPend(MsgQ *pMsgQ, void *pMsg, int len, uint16 timeout)
{
	void *ret = NULL;
	uint8 err;	
	
    ret = SysQPend(pMsgQ->pEvent, timeout, &err);
	if(ret != NULL) 
	{
		memcpy(pMsg, ret, len);
		SysMemPut(pMsgQ->pMem, ret);
	}

	return err;
}

