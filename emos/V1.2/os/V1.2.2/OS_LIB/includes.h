#ifndef _INCLUDES_H_	
#define _INCLUDES_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stm32f4xx.h"			//STM32F4xx固件库
#include "C2.h"	
#include "sys_cpu.h"				

#include "debug_info.h"
#include "driver.h"

//应用层自定义短延时函数
static __inline RDelay(int32 cnt)
{
	while(cnt--);
}
	
#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif
