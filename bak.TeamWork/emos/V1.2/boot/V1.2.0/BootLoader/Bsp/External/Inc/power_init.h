#ifndef _POWER_INIT_H_	
#define _POWER_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
	
#define PowerLine_Read 		GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_4)	//电源线检测
#define Power_Close 		GPIO_SetBits(GPIOE, GPIO_Pin_3)				//电源关
#define Power_Open 			GPIO_ResetBits(GPIOE, GPIO_Pin_3)			//电源开

void Power_init(void);

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
