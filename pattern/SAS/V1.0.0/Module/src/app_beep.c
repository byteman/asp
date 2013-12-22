#include "includes.h"
#include "task_def.h"
#include "sys_config.h"
#include "debug_info.h"
#include "beep_thread.h"
#include "beep_init.h"
#include "app_beep.h"

#define BeepQueueSize 2

static HANDLE beep_handle;
static SYS_EVENT *BeepEvent = NULL;
static void *BeepBuff[BeepQueueSize];	  //消息队列缓冲区

void Beep_Initial(void)
{
	uint8 err;

	BeepEvent = SysQCreate(BeepBuff, BeepQueueSize);
	if(BEEPJOB_Init(BeepPrio, BeepEvent) == 0) 
	{
		debug(Debug_Error, "Error: EMOS beep init failed! \r\n");
		SysTaskSuspend(SYS_PRIO_SELF);
	}
		
	beep_handle = DeviceOpen(CHAR_BEEPJOB,0,&err);
	if(err != HVL_NO_ERR)
	{
		debug(Debug_Warning, "Warning: can't open beep device!\r\n");
	}

	//根据设置打开或关闭蜂鸣器
	Beep_Power(Get_Beep_Flag());
}

int32 Beep_Voice(uint32 cnt, uint16 time)
{
	static BEEP_STRUCTURE value;

	value.count = cnt;
	value.time = time;

	return DeviceIoctl(beep_handle,BEEPCMD_ON, (u32)&value);
}

void Beep_Power(uint8 type)
{
	DeviceIoctl(beep_handle,BEEPCMD_SETONOFF,type);
}

