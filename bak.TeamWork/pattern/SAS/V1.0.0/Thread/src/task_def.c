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
#include "dt3102_io.h"
#include "sys_param.h"
#include "softdog.h"
#include "ccm_mem.h"

//串口1
#include "usart1.h"
#include "usart_cfg.h"

#include "net_rmcpupdate_business.h"
#include "net_business_thread.h"


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
// __align(8) static SYS_STK WatchDogThreadStack[WatchDogStackSize];
// __align(8) static SYS_STK KeyThreadStack[KeyStackSize];
// __align(8) static SYS_STK BusinessThreadStack[BusinessStackSize];
// __align(8) static SYS_STK AlgorimThreadStack[AlgorimStackSize];
// __align(8) static SYS_STK SerialRecvThreadStack[SerialRecvStackSize];
// __align(8) static SYS_STK DataManagerThreadStack[DataManagerStackSize];
// __align(8) static SYS_STK USBThreadStack[USBStackSize];
// __align(8) static SYS_STK GUIhreadStack[GUIStackSize];
// __align(8) static SYS_STK NetThreadStack[NetStackSize];
// __align(8) static SYS_STK NetSendStack[NetSendStackSize];
// __align(8) static SYS_STK DebugThreadStack[DebugStackSize];
// __align(8) static SYS_STK StartThreadStack[StartStackSize];

static SYS_STK *WatchDogStackBos	= NULL;
static SYS_STK *KeyStackBos			= NULL;
static SYS_STK *BusinessStackBos	= NULL;
static SYS_STK *AlgorimStackBos		= NULL;
static SYS_STK *SerialRecvStackBos	= NULL;
static SYS_STK *DataManagerStackBos	= NULL;
static SYS_STK *USBStackBos			= NULL;
static SYS_STK *GUIStackBos		    = NULL;
static SYS_STK *NetStackBos			= NULL;
static SYS_STK *NetSendStackBos		= NULL;
static SYS_STK *DebugStackBos  		= NULL;
static SYS_STK *StartStackBos  		= NULL;

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

static void WatchDog_Thread(void *arg)
{
	static BOOL led_flag = FALSE;
	static int W_Handle;
	uint8 err;
	TaskMsg msg = {0};	

	W_Handle = Register_SoftDog(400, SOFTDOG_RUN, "线程调度");

	//网络升级
	while(1) 
	{
		err = Task_QPend(&NetUpdateMsgQ, &msg, sizeof(TaskMsg), 50);		
		if(err == SYS_ERR_NONE) 
		{
			Net_Rmcp_Update(msg.msg_id, (unsigned char *)msg.msg_ctx, (int)msg.msg_len);
		}
        else if(err == SYS_ERR_TIMEOUT)
        {
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
        
        //软件看门狗喂狗
  		if(W_Handle >= 0) SoftDog_Feed(W_Handle);
	}
}

static void Start_Thread(void *arg)
{
	uint8 task_status = 0;
	
	printf(" >>线程开始调度!\r\n");	

	Task_Timer_Create();		//创建定时器函数	
	
	//软件看门狗初始化
	if(SoftDog_Init() == 0) 
	{
		debug(Debug_Error, "WatchDog init err\r\n");
		while(1);
	}
	
	task_status |= SysTaskCreate(Key_Thread,		  NULL, KeyStackBos + KeyStackSize - 1,                 KeyPrio);
	task_status |= SysTaskCreate(Business_Thread, 	  NULL, BusinessStackBos + BusinessStackSize - 1,	    BusinessPrio);
	task_status |= SysTaskCreate(Algorim_Thread, 	  NULL, AlgorimStackBos + AlgorimStackSize - 1,	        AlgorimPrio);
	task_status |= SysTaskCreate(SerialRecv_Thread,   NULL, SerialRecvStackBos + SerialRecvStackSize - 1,	SerialRecvPrio);
	task_status |= SysTaskCreate(DataManager_Thread,  NULL, DataManagerStackBos + DataManagerStackSize - 1, DataManagerPrio);
	task_status |= SysTaskCreate(USB_Thread, 		  NULL, USBStackBos + USBStackSize - 1,			        USBPrio);
	task_status |= SysTaskCreate(GUI_Thread, 		  NULL, GUIStackBos + GUIStackSize - 1,			        GUIPrio);
	task_status |= SysTaskCreate(Net_Business_Thread, NULL, NetStackBos + NetStackSize - 1,			        NetPrio);
	task_status |= SysTaskCreate(Net_Send_Thread, 	  NULL, NetSendStackBos + NetSendStackSize - 1,	        NetSendPrio);
	task_status |= SysTaskCreate(Debug_Thread,		  NULL, DebugStackBos + DebugStackSize - 1,  	        DebugPrio);
	task_status |= SysTaskCreate(WatchDog_Thread, 	  NULL, WatchDogStackBos + WatchDogStackSize - 1,       WatchDogPrio);

	if(task_status != SYS_NO_ERR) 
	{
		debug(Debug_None, "线程创建失败");
		while(1);						//任务创建失败	
	}

	//启动定时器 
	Task_Timer_Start();
	
	debug(Debug_None, "线程开始调度!\r\n");	
	SysTaskDel(StartPrio);
}	

/////////////////////////////////////////////////////////////////////////////////
//从CCM memory处分配内存
//各任务堆栈栈底指针指向地址0x10000000开始处的CCM MEMORY内存区域
static void Assign_TaskStack_Buffer(void)
{
	WatchDogStackBos	= (SYS_STK *)Ccm_Malloc(WatchDogStackSize * sizeof(SYS_STK));
	KeyStackBos			= (SYS_STK *)Ccm_Malloc(KeyStackSize * sizeof(SYS_STK));
	BusinessStackBos	= (SYS_STK *)Ccm_Malloc(BusinessStackSize * sizeof(SYS_STK));
	AlgorimStackBos		= (SYS_STK *)Ccm_Malloc(AlgorimStackSize * sizeof(SYS_STK));
	SerialRecvStackBos	= (SYS_STK *)Ccm_Malloc(SerialRecvStackSize * sizeof(SYS_STK));
	DataManagerStackBos	= (SYS_STK *)Ccm_Malloc(DataManagerStackSize * sizeof(SYS_STK));
	USBStackBos			= (SYS_STK *)Ccm_Malloc(USBStackSize * sizeof(SYS_STK));
	GUIStackBos			= (SYS_STK *)Ccm_Malloc(GUIStackSize * sizeof(SYS_STK));
	NetStackBos			= (SYS_STK *)Ccm_Malloc(NetStackSize * sizeof(SYS_STK));
	NetSendStackBos		= (SYS_STK *)Ccm_Malloc(NetSendStackSize * sizeof(SYS_STK));
	DebugStackBos		= (SYS_STK *)Ccm_Malloc(DebugStackSize * sizeof(SYS_STK));
	StartStackBos		= (SYS_STK *)Ccm_Malloc(StartStackSize * sizeof(SYS_STK));
}

//定义长度22*1024时会死掉;21时能启动
//__align(8) char deadstack[1024 * 21] = {0};
int main(void)
{
	SetAppRunAddr(0x8020000);					//设置跳转地址，需同时修改IROM1地址	
	System_Init();								//系统初始化
	Bsp_Init();									//驱动初始化
	App_Msg_Init();	
	fLED_IO_Init();		
	Debug_Init();
	Assign_TaskStack_Buffer();
	
	//创建看门狗任务/启动任务
	SysTaskCreate(Start_Thread, NULL, StartStackBos+StartStackSize-1, StartPrio);	

	SysStart();	//开始任务调度
	return 0;
}		

	
