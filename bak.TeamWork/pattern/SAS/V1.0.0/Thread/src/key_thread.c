#include "includes.h"
#include "task_msg.h"

#include "usart1.h"
#include "printf_init.h"

#include "app_beep.h"

#include "Keyboard.h"
#include "dt3102_io.h"
#include "guiwindows.h"
#include "debug_info.h"
#include "sys_param.h"

static HANDLE KEY_Dev=HANDLENULL;		//key device handle

static void fDT3102_IO_Init(void)
{
	uint16 tmp = 0xffff;
	
	DT3102_OutputInit();
	DT3102_InputInit();

	Param_Read(GET_U16_OFFSET(ParamDeviceSignal), &tmp, sizeof(unsigned short));
	if((tmp & 0xf800) != 0)		//电平设置使用两个字节的低11位，高位初始化为0，若高位不等于0时则重新加载默认电平
	{
		//加载默认电平
		DT3102_DeviceSignal_DefaultInit();
	}
	else
	{
		Assign_3102_DeviceSignal(tmp);
	}
}

void Key_Enable(BOOL status)
{
	if(KEY_Dev!=HANDLENULL)
	{
		if(status)
			DeviceIoctl(KEY_Dev,KEYCMD_SETONOFF,1);	//open
		else	
			DeviceIoctl(KEY_Dev,KEYCMD_SETONOFF,0);	//close
	}
}

void Key_Thread(void *arg)
{
	int KeyLookVal = 0;			//查询用变量	
	HVL_ERR_CODE err = HVL_NO_ERR;	//设备操作错误											
	GuiMsgInfo guiMsgInfo;
	guiMsgInfo.ID = WM_KEYDOWN;
    
	//键盘初始化
	if(Key_Config() == 0) 
	{										
		debug(Debug_None, "Error: Emos key init failed!\r\n");
		SysTaskSuspend(SYS_PRIO_SELF);
	}
	
	KEY_Dev = DeviceOpen(CHAR_KEY,0,&err);		//打开键盘
	if(err != HVL_NO_ERR)
		debug(Debug_Warning, "Warning: Emos key open failed!\r\n");
	
	Key_Enable(TRUE);											//开启按键开关 
	//外设初始化放到这儿,因为需要读取FRAM
	fDT3102_IO_Init();

	while(1)			//矩阵键盘测试
	{	
		KeyLookVal = CharDeviceRead(KEY_Dev,0,0);			//获取按键值	
		if(KeyLookVal < 0) continue;

		Beep_Voice(1, 5);
       		
		guiMsgInfo.wParam = KeyLookVal;
		GuiMsgQueuePost(&guiMsgInfo);
	}

 }
