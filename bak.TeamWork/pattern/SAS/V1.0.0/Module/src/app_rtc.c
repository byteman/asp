#include "app_rtc.h"
#include "pcf8563.h"
#include "debug_info.h"

volatile static CSysTime curTime={0};
static HANDLE rtc_handle;


// 说明：由10ms定时器调用，更新RTC时间到curTime中
void Update_System_Time(void)
{
	OutTime_Body systime={0};

	DeviceIoctl(rtc_handle,OUTRTCCMD_GetTime, (u32)&systime);
	curTime.year= systime.Year;
	curTime.mon	= systime.Month;
	curTime.day	= systime.Day;
	curTime.hour= systime.HH;
	curTime.min	= systime.MM;
	curTime.sec	= systime.SS;
	curTime.week= systime.Week;
}

//说明：由10ms定时器调用，初始化rtc模块
void CSysTime_Init(void)
{
	HVL_ERR_CODE err;
	if(OutRTC_init(0)==0)
	{
		debug(Debug_None,"init outer rtc failed!!!\n");
		return;
	}
	
	rtc_handle=DeviceOpen(CHAR_RTC,0,&err);
	if(err != HVL_NO_ERR)
	{
		debug(Debug_Error,"open rtc device failed!!!\n");
		while(1);
	}
}


// 返回值：返回curTime时间结构体地址
// 说明：通过返回的指针，可查询到当前的时间
CSysTime* Get_System_Time(void)
{
	return (CSysTime*)&curTime;
}

// 参数：time为要设置的时间结构体指针
// 返回值：为0时设置成功，为1时设置出错，参数超出范围
// 说明：根据time指向的结构体内容设置系统时间
u8 Set_System_Time(const CSysTime *time)
{
	OutTime_Body systime;
	
	if(time==NULL)
		return 1;
	systime.Year=time->year;
	systime.Month=time->mon;
	systime.Day=time->day;
	systime.HH=time->hour;
	systime.MM=time->min;
	systime.SS=time->sec;
	systime.Week=time->week;
	
	//设置rtc芯片里的时间
	return DeviceIoctl(rtc_handle, OUTRTCCMD_SetTime, (u32)&systime);
}


