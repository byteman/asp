/*
 * File:   task_timer.c
 * Author: river
 * Brief:  系统定时器与硬件定时器回调函数实现

 * History:
 * 2012-11-19 created
 * 2012-12-24 给业务任务发送的10ms定时消息开机延迟2s;
 */

#include "includes.h"
#include "task_msg.h"
#include "app_msg.h"
#include "driver.h"
#include "timer7.h"
#include "timer5.h"
#include "device_info.h"
#include "overtime_queue.h"
#include "app_rtc.h"
#include "sys_config.h"
#include "guiwindows.h"

#include "sdk_serial.h"
#include "softdog.h"
#include "DT3102_IO.h"
#include "app_timer.h"

#include "net_rmcpupdate_business.h"
#include "serial_protocol.h"

static SYS_TMR *onesecond_tmr;
static HANDLE tim7_dev; 
static HANDLE tim5_dev;

static volatile	unsigned long sys_tick = 0;


//1s sys timer:每隔1秒查询设备故障状态信息是否有变化，如果有变化则给串口2发送线程发送设备状态信息变化通知；
//同时每隔10秒给串口2发送线程发送心跳通知
static void TaskTimer_1s(void *ptmr, void *parg)
{
	static DeviceStatus lastDev={0};	//上一次的外设状态
	static u8 timeCnt=0;	//10s 计数
	TaskMsg msg = {0};
	u32 curDev=Get_Device_Status();
	//查询设备故障状态信息是否有变化
	if((curDev & 0x154) !=(lastDev.value & 0x154))	//1 0101 0100=0x154
	{
		//有故障状态变化
		msg.msg_id=Msg_Device_Fault;
		Task_QPost(&DataManagerMsgQ, &msg);
		debug(Debug_Notify,"device fault change!!!\r\n");
	}
	//保存这次状态
	lastDev.value=curDev;
	
	//每隔10秒给串口2发送线程发送心跳通知
	if(OverQueue_Get_Size() == 0) 
	{
		if(timeCnt++ > 9) 
		{
			timeCnt=0;
			msg.msg_id=Msg_Device_Heart;
			Task_QPost(&DataManagerMsgQ, &msg);
		} 
	} 
	else 
	{
		timeCnt = 0;
	}
}

static u8 OverTick=0;	//毫秒 计数(0-50)

void Reset_OvertimeTick(void)
{
	OverTick = 0;
}

//10ms timer task routine,给业务处理线程发送10ms定时信息，
//同时查询待发送队列队头元素是否已超时或者未发送，然后给串口2发送线程发送数据超时信息；
//该模块每隔1秒读取并更新RTC时钟，给GUI线程发送1秒窗体刷新信息
static void TaskTimer_1ms(void)
{
	static u8 SencdTick=0;	//秒 计数(0-100)		
	static u8 cnt = 0;
	TaskMsg msg = {0};
	OvertimeData *pCar = NULL;

	//应用层时间tick计数
	sys_tick++;

	cnt++;
	//10ms执行一次的ISR
	if(cnt >= 10)
	{
		cnt = 0;

		//软件看门狗中断处理
		SoftWatchDog_ISR();
		
		//判读发送队列队头元素是否已超时
		pCar = OverQueue_Get_Head();
		if(pCar != NULL)
		{
			//主动发送机制的信息需要超时重发
			if(pCar->SendMod == SendActiveMod)
			{
				if(pCar->bSend == 1) 
				{
					OverTick++;
					//超时重发
					if(OverTick >= 50)
					{
						OverTick = 0;
						msg.msg_id = Msg_Overtime;
						Task_QPost(&DataManagerMsgQ, &msg);			
					} 
				}
				else
				{
					msg.msg_id = Msg_Overtime;
					Task_QPost(&DataManagerMsgQ, &msg);
				}
			}
		}
		
		//1s后读取rtc时间
		if(++SencdTick >= 100)
		{
			SencdTick = 0;	
			//网络升级超时机制
			NetUpdate_Overtime_Business();
			//PostWindowMsg(g_pCurWindow,WM_TIMEUPDATE, 0, 0);
		}
	}
}

unsigned long Get_Sys_Tick(void)
{
	unsigned long tmp = 0;
#if	SYS_CRITICAL_METHOD == 3
	SYS_CPU_SR cpu_sr = 0;
#endif

	SYS_ENTER_CRITICAL();
	tmp = sys_tick;
	SYS_EXIT_CRITICAL();
	
	return tmp;
}

void Task_Timer_Create(void)
{
	HVL_ERR_CODE err;

	//配置1s定时器
	onesecond_tmr = SysTmrCreate(10,10,SYS_TMR_OPT_PERIODIC,TaskTimer_1s,NULL,"one second",&err);
	
	if(err != SYS_ERR_NONE) {
		debug(Debug_Error,"Create 1s sys timer task failed !\r\n");
	}
	//timer7配置2ms定时器
	if(TIM7_Config(TaskTimer_2ms, 20, Mode_100US) == 0) {
		debug(Debug_None, "Timer2 init failed\r\n");	 
		while(1);
	}
	//timer5配置10ms定时器
	if(TIM5_Config(TaskTimer_1ms,10, Mode_100US) == 0) {
		debug(Debug_None, "Timer2 init failed\r\n");	 
		while(1);
	}
	
	tim7_dev = DeviceOpen(CHAR_TIM7,0,&err);
	if(err!=HVL_NO_ERR)
	{
		debug(Debug_Error,"open timer 7 error\r\n");
		while(1);
	}	
	tim5_dev = DeviceOpen(CHAR_TIM5,0,&err);
	if(err!=HVL_NO_ERR)
	{
		debug(Debug_Error,"open timer 5 error\r\n");
		while(1);
	}
}


void Task_Timer_Start(void)
{
	u8 err;

	SysTmrStart(onesecond_tmr, &err);				//开启1s定时器
	if(err!=SYS_ERR_NONE)
	{
		debug(Debug_Error,"start system timer error!!!\r\n");
	}
	if(DeviceIoctl(tim7_dev,TIM7_CMDTimOpen,0)==0)	//开启2ms定时器
	{
		debug(Debug_None,"start timer 7 error!!!\r\n");
		//while(1);
	}
	if(DeviceIoctl(tim5_dev,TIM5_CMDTimOpen,0)==0)	//开启10ms定时器
	{
		debug(Debug_None,"start timer 5 error!!!\r\n");
		//while(1);
	}
	CSysTime_Init();		//app rtc模块初始化
}	

