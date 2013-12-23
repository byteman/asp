#ifndef _TIM6_INIT_H_	
#define _TIM6_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"

#define Mode_1US				0							//1微秒
#define Mode_10US				1							//10微秒
#define Mode_100US			2							//100微秒
#define Mode_500US			3							//500微秒

typedef void(*TIM6_fun)(void); 			//函数指针
	
u8 TIM6_Config(const TIM6_fun Cfun,const u16 TimeUS,const u8 mode);	 	//定时器初始化
void TIM6_TimeISR_Open(void);												//开启定时器及中断
void TIM6_TimeISR_Close(void);											//关闭定时器及中断
void TIM6_ResetCount(void);													//复位计数值
u32 TIM6_GetCount(void);														//获得计数值
void TIM6_CntFlagOnOff(const u8 OnOff);							//是否启用计数功能

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
