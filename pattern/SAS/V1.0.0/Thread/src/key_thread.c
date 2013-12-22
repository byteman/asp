#include "includes.h"
#include "task_msg.h"

#include "usart1.h"
#include "printf_init.h"

#include "app_beep.h"

#include "Keyboard.h"
#include "dt3102_io.h"
#include "guiwindows.h"

static HANDLE KEY_Dev=HANDLENULL;		//key device handle

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
	u32 uKeyCode;
	int gGetKey = 0;
	int gGetKeyEnvent = 0;
	
	int KeyLookVal = 0;			//查询用变量	
	HVL_ERR_CODE err = HVL_NO_ERR;	//设备操作错误											
	GuiMsgInfo guiMsgInfo;
	guiMsgInfo.ID = WM_KEYDOWN;

	#if 1
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
	#else
	
	bsp_InitKey();
	
	#endif
	
	
	//外设初始化放到这儿,因为需要读取FRAM
	fDT3102_IO_Init();

//while(1)
//{
//	Key_swap();
//	SysTimeDly(20);
//}
while(1)			//矩阵键盘测试
	{	
		#if 1
		KeyLookVal = CharDeviceRead(KEY_Dev,0,0);			//获取按键值	
		if(KeyLookVal < 0) continue;
		Beep_Voice(1, 5);

		guiMsgInfo.wParam = KeyLookVal;
		GuiMsgQueuePost(&guiMsgInfo);
		#else
		SysTimeDly(10);
		
		bsp_KeyScan();
		gGetKey = bsp_GetKey();
			if(gGetKey )
			{	
				uKeyCode = gGetKey >> 8;
				gGetKeyEnvent = gGetKey &	0xff	;
				if(uKeyCode != KEY_NONE)
				{
						if(gGetKeyEnvent == KEY_DOWN) 
						{

						}	
						if(uKeyCode == KEY_UP)
						{
								 
						}
						else if(uKeyCode == KEY_DOWN)
						{ 
							
						}				
						
				}						
			
			Beep_Voice(1, 5);
			
			guiMsgInfo.wParam = KeyLookVal;
			GuiMsgQueuePost(&guiMsgInfo);
				
		}
		#endif
	}

 }
