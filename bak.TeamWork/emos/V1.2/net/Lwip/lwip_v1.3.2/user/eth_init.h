#ifndef _ETH_INIT_H_	
#define _ETH_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"
	
u8 ETH_Config(void);		//以太网初始化

void ETH_NVIC_Enable(void);

void ETH_NVIC_Disable(void);

	
#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
