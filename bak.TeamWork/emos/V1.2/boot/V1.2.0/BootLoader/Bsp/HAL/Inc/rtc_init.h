#ifndef _RTC_INIT_H_
#define _RTC_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"

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

u8 RTC_Config(const u16 year,const u8 month,const u8 day,const u8 week,const u8 hour,const u8 min,const u8 sec);	 	//RTC配置
ErrorStatus SetRTC_Time(const u8 hour,const u8 min,const u8 sec);										//设置时间
ErrorStatus SetRTC_Date(const u16 year,const u8 month,const u8 day,const u8 week);	//设置日期
void GetTime(Time_Body* pTime);															//获取时间
void ShowTime(void);																				//获取并显示时间

void RTC_AlarmA(const u8 A_Day,const u8 A_Unit,const u8 A_Hours,const u8 A_Minutes,const u8 A_Seconds,const u8 A_Alarm_CE,void (*Alarm_fun)(void));		//配置闹钟A		D>31 H>23 M>59 S>59则屏蔽该项 屏蔽两项以上为任一报警 即每秒报警
void RTC_AlarmB(const u8 B_Day,const u8 B_Unit,const u8 B_Hours,const u8 B_Minutes,const u8 B_Seconds,const u8 B_Alarm_CE,void (*Alarm_fun)(void));		//配置闹钟B		D>31 H>23 M>59 S>59则屏蔽该项 屏蔽两项以上为任一报警 即每秒报警

void Alarm_A_Caller(void(*Alarm_fun)(void));								//报警A响应执行函数
void Alarm_B_Caller(void(*Alarm_fun)(void));								//报警B响应执行函数

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	#include "printf_init.h"		//DebugPf

	void Alarm_A_Respond(void)						//定义报警A执行函数
	{
		DebugPf("AAAAAA!!\r\n");						//报警
	}
	void Alarm_B_Respond(void)						//定义报警B执行函数
	{
		DebugPf("BBBBBBB!!\r\n");						//报警
	}

	if(RTC_Config(2012,10,4,4,15,45,30))		//实时时钟初始化		年月日周时分秒
	{
		RTC_AlarmA(4,0,15,45,35,1,Alarm_A_Respond);		//配置闹钟A		每月4号15:45:30引发报警A
		RTC_AlarmB(4,1,15,45,40,1,Alarm_B_Respond);		//配置闹钟B		每周4  15:45:30引发报警B
	}

	ShowTime();										//获取并显示时间
*/
