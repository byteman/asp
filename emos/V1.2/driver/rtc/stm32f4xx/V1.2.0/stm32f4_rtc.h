#ifndef _RTC_INIT_H_
#define _RTC_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif
	
#include "includes.h"
	
#define 		RTCCMD_SetTime					0				//设置时间	
#define 		RTCCMD_SetDate					1				//设置日期
#define 		RTCCMD_LookTimeDate			2				//查询日期
#define 		RTCCMD_ShowTimeDate			3				//显示日期	串口打印显示
#define 		RTCCMD_SetAlarm_A				4				//设置闹钟A时间
#define 		RTCCMD_SetAlarm_B				5				//设置闹钟B时间
#define 		RTCCMD_SetAlarm_A_FUN		6				//设置闹钟A执行函数
#define 		RTCCMD_SetAlarm_B_FUN		7				//设置闹钟B执行函数

//时间结构体
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
	u8 AMorPM;		//上午or下午	0上午	1下午			//24小时制不使用
} Time_Body;

typedef struct
{
	u8 	ALARM_Cfg_Day;						//日期	or 周
	u8 	ALARM_Cfg_Unit;						//单位	 0日期 1周
	u8 	ALARM_Cfg_Hours;					//小时
	u8 	ALARM_Cfg_Minutes;				//分钟
	u8 	ALARM_Cfg_Seconds;				//秒
	u8 	ALARM_Cfg_Alarm_CE;				//开关		1开0关
} ALARM_Cfg;										//闹钟设置

/****************************************************************************
* 名	称：u8 RTCInit(void);
* 功	能：硬件RTC初始化
* 入口参数：无
* 出口参数：u8	设置是否成功		1成功 0失败
* 说	明：无
****************************************************************************/
u8 RTCInit(void);	 //RTC初始化

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				无用
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备读操作
* 入口参数：HANDLE IDH		设备句柄
						u8* Buffer		时间值								Time_Body结构体	
						u32 len				希望读取的数据个数		无用
						u32* Reallen	实际读取的数据个数		无用
						u32 lParam		参数									无用
* 出口参数：s32						读取操作是否成功			无用
* 说	明：
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
	u8 AMorPM;	//上午or下午	0上午	1下午			//24小时制不使用
} Time_Body;
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备写操作
* 入口参数：HANDLE IDH		设备句柄							无用
						u8* Buffer		时间值								Time_Body结构体	
						u32 len				希望写入的数据个数		无用
						u32* Reallen	实际写入的数据个数		无用
						u32 lParam		参数									无用
* 出口参数：s32						写入操作是否成功			无用
* 说	明：
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
	u8 AMorPM;	//上午or下午	0上午	1下午			//24小时制不使用
} Time_Body;
****************************************************************************/	

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备控制
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数							返回值
RTCCMD_SetTime						Time_Body					是否成功		1成功 0失败
RTCCMD_SetDate						Time_Body					是否成功		1成功 0失败
RTCCMD_LookTimeDate				Time_Body					无用
RTCCMD_ShowTimeDate				无用							无用
RTCCMD_SetAlarm_A					ALARM_Cfg					无用
RTCCMD_SetAlarm_B					ALARM_Cfg					无用
RTCCMD_SetAlarm_A_FUN			Alarm_fun					无用
RTCCMD_SetAlarm_B_FUN			Alarm_fun					无用
****************************************************************************/	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	void ALarmA(void)					//闹钟A
	{
		printf("\r\nALarmA\r\n");
	}		
	void ALarmB(void)					//闹钟B
	{
		printf("\r\nBLarmB\r\n");
	}

	u32 Reallen = 0;
	HVL_ERR_CODE err = HVL_NO_ERR;	//设备操作错误
	
	HANDLE RTC_Dev = 0;												//设备句柄
	
	Time_Body SetTime = 
	{
		2012,11,11,			//年月日
		23,59,58,				//时分秒
		7,0							//周 AM/PM
	};
	
	ALARM_Cfg RTCA = 		//闹钟时间				//每秒
	{
		99,				//日期	or 周
		0,				//单位	 	0日期 1周
		99,				//小时
		99,				//分钟
		99,				//秒
		1,				//开关		1开0关
	};
	
	ALARM_Cfg RTCB = 		//闹钟时间				//4日16:xx:10秒的时候触发闹钟	每分钟
	{
		4,				//日期	or 周
		0,				//单位	 	0日期 1周
		16,				//小时
		99,				//分钟
		10,				//秒
		1,				//开关		1开0关
	};

	USART_STRUCT usart1cfg = 
	{
		57600,																	//波特率
		USART_WordLength_8b,										//数据位
		USART_StopBits_1,												//停止位
		USART_Parity_No													//效验位
	};

	USART1_Config(&usart1cfg);								//串口初始化
	Set_PrintfPort(1);												//设置打印串口
	printf("测试开始\r\n");
	
	RTCInit();
	
	RTC_Dev = DeviceOpen(CHAR_RTC,0,&err);		//设备打开
	if(err != HVL_NO_ERR)
		while(1);
	
	if(CharDeviceWrite(RTC_Dev,(u8*)&SetTime,0,&Reallen,0) == 0)					//设置时间
		while(1);
	if(DeviceIoctl(RTC_Dev,RTCCMD_SetAlarm_A,(u32)&RTCA) == 0)						//设置闹钟A
		while(1);
	if(DeviceIoctl(RTC_Dev,RTCCMD_SetAlarm_B,(u32)&RTCB) == 0)						//设置闹钟B
		while(1);
	if(DeviceIoctl(RTC_Dev,RTCCMD_SetAlarm_A_FUN,(u32)ALarmA) == 0)				//设置闹钟A执行函数
		while(1);
	if(DeviceIoctl(RTC_Dev,RTCCMD_SetAlarm_B_FUN,(u32)ALarmB) == 0)				//设置闹钟B执行函数
		while(1);
	
	while(1)
	{
		if(DeviceIoctl(RTC_Dev,RTCCMD_ShowTimeDate,0) == 0)									//显示时间
			while(1);
		SysTimeDly(100);			//1S
	}
*/
