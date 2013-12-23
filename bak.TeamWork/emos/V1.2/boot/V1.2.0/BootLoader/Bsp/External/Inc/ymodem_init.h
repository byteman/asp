#ifndef _YMODEM_INIT_H_
#define _YMODEM_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"

u8 Ymodem_START(const u8 Option);			//ymodem开始						 	extern				flash_bootloader_init.c
u32 GetFlieSize(void);						//获取文件大小						extern				flash_bootloader_init.c
u8 GetFlieNameLen(void);					//获取文件名长度					extern				flash_bootloader_init.c
u8* GetFlieNameDataAddr(void);		//获取文件名存放地址			extern				flash_bootloader_init.c

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}						   
#endif

#endif
