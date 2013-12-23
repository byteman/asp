#ifndef _APP_RTC_H	
#define _APP_RTC_H
	#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
	extern "C" {
	#endif

#include "includes.h"
		
//时间结构体
typedef  struct CSysTime { 
	u16 	year;		//年  2000+，最大可到65535
	u8  	mon;	//月  1-12
	u8 		day;		//日  1-31
	u8  	week;	//周  0-6，0为周日
	u8		hour;	//时  0-13
	u8		min;		//分  0-59
	u8		sec;		//秒  0-59
}CSysTime;

CSysTime* Get_System_Time(void);
// 返回值：返回curTime时间结构体地址
// 说明：通过返回的指针，可查询到当前的时间

u8 Set_System_Time(const CSysTime *time);
// 参数：time为要设置的时间结构体指针
// 返回值：为0时设置成功，为1时设置出错，参数超出范围
// 说明：根据time指向的结构体内容设置系统时间


void CSysTime_Init(void);//说明：由10ms定时器调用，初始化rtc模块

void Update_System_Time(void);
// 说明：由10ms定时器调用，更新RTC时间到curTime中

		
	#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
	}
	#endif
#endif	//_APP_RTC_H
