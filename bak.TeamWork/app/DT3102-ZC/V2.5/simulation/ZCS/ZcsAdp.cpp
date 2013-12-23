#include "StdAfx.h"
#include "ZcsAdp.h"
#include "Station.h"
#include "includes.h"
#include "task_msg.h"

CStation* g_pStatin = NULL;

void OutPutMulitlineStr(CString StrOut, CString StrLine = _T("\n"));

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

int i = -1;
int x = 0;
void OutPut(char* pStr)
{
	CA2W Wstr(pStr);

	x = i % 20;

	OutPutMulitlineStr(CString(Wstr));
}

CList<TaskMsg, TaskMsg&> MsgList;
TaskMsg g_msg;

void MsgPush(TaskMsg msg)
{
	MsgList.AddTail(msg);
}

TaskMsg* MsgPull()
{
	if(MsgList.GetCount() > 0)
	{
		g_msg = MsgList.RemoveHead();
		return &g_msg;
	}
	else
	{
		return NULL;
	}
}

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif



