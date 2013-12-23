#ifndef _FLASH_BOOTLOADER_INIT_H_	
#define _FLASH_BOOTLOADER_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"			//u32
	
void Jump_IAP(const u32 MainAppAddr);					//程序跳转
void BOOTLoader(const u32 BaudRate);					//引导入口		

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}						   
#endif

#endif
