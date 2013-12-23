#ifndef _PCF8563T_INIT_H_
#define _PCF8563T_INIT_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"

#define 		OUTRTCCMD_SetTime					0				//设置时间
#define 		OUTRTCCMD_GetTime					1				//获取时间
#define 		OUTRTCCMD_ShowTime				2				//显示时间	需要事先配置好printf 用于测试用
	
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
} OutTime_Body;

/****************************************************************************
* 名	称：u8 OutRTC_init(OutTime_Body* Time_t)
* 功	能：外部RTC初始化
* 入口参数：OutTime_Body* Time_t			设置的时间	0表示不需要时间设置
* 出口参数：u8												是否成功		1成功	0失败
* 说	明：无
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
} OutTime_Body;
****************************************************************************/
u8 OutRTC_init(OutTime_Body* Time_t);

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
						u8* Buffer		时间值								OutTime_Body结构体	
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
} OutTime_Body;
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备写操作
* 入口参数：HANDLE IDH		设备句柄							无用
						u8* Buffer		时间值								OutTime_Body结构体	
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
} OutTime_Body;
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
OUTRTCCMD_SetTime					OutTime_Body			是否成功		1成功 0失败
OUTRTCCMD_GetTime					OutTime_Body			是否成功		1成功 0失败
OUTRTCCMD_ShowTime				无用							获取显示时间是否成功		0失败	1成功	(测试使用必须先实现Printf)
****************************************************************************/	

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
#include "pcf8563t_init.h"

	u8 err = 0;
	u8 i = 0;
	
	HANDLE RTC_Dev = 0;												//设备句柄
	
	USART_STRUCT usart1cfg = 
	{
		57600,																	//波特率
		USART_WordLength_8b,										//数据位
		USART_StopBits_1,												//停止位
		USART_Parity_No													//效验位
	};

	OutTime_Body UserSetOutTime = 						//设置用时间
	{
		2012,12,18,11,42,5,1
	};
	
	OutTime_Body Timex = {0};									//设置用时间
	
	USART1_Config(&usart1cfg);								//串口初始化
	Set_PrintfPort(1);												//设置打印串口
	printf("测试开始\r\n");
	

	if(OutRTC_init(&UserSetOutTime) == 0)		//外部RTC初始化
		while(1);
	
	RTC_Dev = DeviceOpen(CHAR_RTC,0,&err);	//设备打开
	if(err != HVL_NO_ERR)
		while(1);
	
	while(1)
	{
		if(i == 20)			//20秒之后重新设置时间
		{
			UserSetOutTime.Year = 2013;
			UserSetOutTime.Month = 11;
			UserSetOutTime.Day = 11;
			UserSetOutTime.HH = 23;
			UserSetOutTime.MM = 14;
			UserSetOutTime.SS = 5;
			UserSetOutTime.Week = 3;
			if(DeviceIoctl(RTC_Dev,OUTRTCCMD_SetTime,(u32)&UserSetOutTime) == 0)									//显示时间
				while(1);
		}
		
		if(i == 40)			//40秒之后重新设置时间
		{
			UserSetOutTime.Year = 2045;
			UserSetOutTime.Month = 12;
			UserSetOutTime.Day = 15;
			UserSetOutTime.HH = 12;
			UserSetOutTime.MM = 4;
			UserSetOutTime.SS = 2;
			UserSetOutTime.Week = 6;
			if(CharDeviceWrite(RTC_Dev,0,0,0,(u32)&UserSetOutTime) == 0)			//修改时间
				while(1);
		}
		
		if(i < 40)			//40秒之前用Ioctl命令进行时间显示
		{
			if(DeviceIoctl(RTC_Dev,OUTRTCCMD_ShowTime,0) == 0)							//显示时间
				while(1);
		}
		else						//40秒之前用Read命令进行时间显示
		{
			if(CharDeviceRead(RTC_Dev,0,0,0,(u32)&Timex) == 0)							//获取时间
				while(1);
			printf("%d+%d+%d %d:%d:%d %d\r\n",Timex.Year,		//年						//显示时间
																	Timex.Month,				//月
																	Timex.Day,					//日
																	Timex.HH,						//时
																	Timex.MM,						//分
																	Timex.SS,						//秒
																	Timex.Week);				//周 0-6
		}
		SysTimeDly(100);			//1S
		i++;
	}
*/
