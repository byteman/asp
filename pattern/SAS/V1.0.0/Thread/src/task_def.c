/*
 * File:   task_def.c
 * Author: river
 * Brief:  开机启动，创建线程

 * History:
 * 2012-11-14 创建文件，创建线程;		river
 * 2012-11-29 创建调试线程, 用于业务调试	river
 * 2013-1-7   增加test线程,将看门狗优先级置为最低;原看门狗线程初始化后删除
 */

#include "includes.h"
#include "driver.h"

#include "system_init.h"				//System_Init
#include "bsp_init.h"					//Bsp_Init
#include "task_msg.h"
#include "task_timer.h"
#include "sys_config.h"
#include "task_def.h"
#include "dog_init.h"
#include "car_business.h"
#include "led.h"
#include "overtime_queue.h"
#include "dt3102_io.h"
#include "sys_param.h"
#include "softdog.h"

//串口1
#include "usart1.h"
#include "usart_cfg.h"

//static HANDLE dog_handle;

struct _ThreadArray
{
	void (*ThreadFun)(void *arg);
	void *arg;
	SYS_STK *pstack;
	int8 prio;
};


//任务堆栈大小
enum {
	WatchDogStackSize		= 128,
	KeyStackSize			= 160,	//256,
	BusinessStackSize		= 512,
	AlgorimStackSize		= 800,	//1024,
	SerialRecvStackSize  	= 256,	//640,
	DataManagerStackSize	= 256,	//320,
	USBStackSize			= 512,  //128
	GUIStackSize			= 512,  //128
	NetStackSize			= 512,  //256
	NetSendStackSize		= 256,
	DebugStackSize			= 256,
	StartStackSize			= 128,
};

//任务堆栈
__align(8) static SYS_STK WatchDogThreadStack[WatchDogStackSize];
__align(8) static SYS_STK KeyThreadStack[KeyStackSize];
__align(8) static SYS_STK BusinessThreadStack[BusinessStackSize];
__align(8) static SYS_STK AlgorimThreadStack[AlgorimStackSize];
__align(8) static SYS_STK SerialRecvThreadStack[SerialRecvStackSize];
__align(8) static SYS_STK DataManagerThreadStack[DataManagerStackSize];
__align(8) static SYS_STK USBThreadStack[USBStackSize];
__align(8) static SYS_STK GUIhreadStack[GUIStackSize];
__align(8) static SYS_STK NetThreadStack[NetStackSize];
__align(8) static SYS_STK NetSendStack[NetSendStackSize];
__align(8) static SYS_STK DebugThreadStack[DebugStackSize];
__align(8) static SYS_STK StartThreadStack[StartStackSize];

//任务入口
static void WatchDog_Thread(void *arg);		 	//看门狗任务
extern void Debug_Thread(void *arg);			//串口调试任务
extern void Key_Thread(void *arg);				//按键任务
extern void Business_Thread(void *arg);			//业务处理任务
extern void Algorim_Thread(void *arg);			//称重算法任务
extern void SerialRecv_Thread(void *arg);		//串口2接收任务
extern void DataManager_Thread(void *arg);		//业务数据管理任务
extern void USB_Thread(void *arg);				//u盘管理任务
extern void GUI_Thread(void *arg);				//GUI任务
extern void Net_Business_Thread(void *arg);		//网络任务
extern void Net_Send_Thread(void *arg);
static void Start_Thread(void *arg);			//启动线程

//任务数组
static const struct _ThreadArray ThreadArray[] = {
	{Start_Thread,				NULL, 	&StartThreadStack[StartStackSize-1],			StartPrio},
	{Key_Thread,				NULL, 	&KeyThreadStack[KeyStackSize-1], 				KeyPrio},
	{Business_Thread, 			NULL, 	&BusinessThreadStack[BusinessStackSize-1],		BusinessPrio},
	{Algorim_Thread, 			NULL,	&AlgorimThreadStack[AlgorimStackSize-1],		AlgorimPrio},
	{SerialRecv_Thread, 		NULL, 	&SerialRecvThreadStack[SerialRecvStackSize-1],	SerialRecvPrio},
	{DataManager_Thread,		NULL, 	&DataManagerThreadStack[DataManagerStackSize-1],DataManagerPrio},
	{USB_Thread, 				NULL, 	&USBThreadStack[USBStackSize-1],				USBPrio},
	{GUI_Thread, 				NULL, 	&GUIhreadStack[GUIStackSize-1],					GUIPrio},
	{Net_Business_Thread,   	NULL, 	&NetThreadStack[NetStackSize-1],			    NetPrio},
	{Net_Send_Thread, 			NULL, 	&NetSendStack[NetSendStackSize-1],				NetSendPrio},
	{Debug_Thread,				NULL, 	&DebugThreadStack[DebugStackSize-1],			DebugPrio},
	{WatchDog_Thread, 			NULL, 	&WatchDogThreadStack[WatchDogStackSize-1],		WatchDogPrio},
};

static void WatchDog_Thread(void *arg)
{
	static BOOL led_flag = FALSE;
	static int W_Handle;

	W_Handle = Register_SoftDog(200, SOFTDOG_RUN, "线程调度");
	
 	while(1) 
 	{
		SysTimeDly(50);			//延时500ms 硬件看门狗时间 1s ~ 2.25s
		
		//软件看门狗喂狗
  		if(W_Handle >= 0) SoftDog_Feed(W_Handle);
		
		//流水灯,提示系统是否当机
		if(led_flag) 
		{
			LED1_OFF;
			LED2_ON;
		} 
		else 
		{
			LED1_ON;
			LED2_OFF;
		}
		led_flag = !led_flag;
	}
}

static void Start_Thread(void *arg)
{
	uint8 task_status = 0;
	int32 i = 0;

	Task_Timer_Create();		//创建定时器函数	
	
	//软件看门狗初始化
	if(SoftDog_Init() == 0) 
	{
		debug(Debug_Error, "WatchDog init err\r\n");
		while(1);
	}
	
	//创建其他任务
	for(i = 1; i < sizeof(ThreadArray)/sizeof(ThreadArray[0]); i++) 
	{	
		task_status |= SysTaskCreate(ThreadArray[i].ThreadFun, ThreadArray[i].arg,		
										ThreadArray[i].pstack, ThreadArray[i].prio);

		if(task_status != SYS_NO_ERR) break;
	}
										
	if(task_status != SYS_NO_ERR) 
	{
		debug(Debug_None, "线程创建失败,id:%d\n", i);
		while(1);						//任务创建失败	
	}

	//启动定时器 
	Task_Timer_Start();
	
	debug(Debug_None, "线程开始调度!\r\n");	
	SysTaskDel(StartPrio);
}	

//定义长度22*1024时会死掉;21时能启动
//__align(8) char deadstack[1024 * 21] = {0};
int main(void)
{
	SetAppRunAddr(0x8000000);					//设置跳转地址，需同时修改IROM1地址	
	System_Init();								//系统初始化
	Bsp_Init();									//驱动初始化
	App_Msg_Init();	
	fLED_IO_Init();	
	OverQueue_Init();		
	Debug_Init();
	
	//创建看门狗任务/启动任务
	SysTaskCreate(ThreadArray[0].ThreadFun, ThreadArray[0].arg,	
					ThreadArray[0].pstack,	 ThreadArray[0].prio);	

	printf("线程开始调度!\r\n");	
	SysStart();	//开始任务调度
	return 0;
}		

	
