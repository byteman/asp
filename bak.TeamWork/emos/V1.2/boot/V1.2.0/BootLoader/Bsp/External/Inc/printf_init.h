#ifndef _PRINTF_INIT_H_	
#define _PRINTF_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdio.h>		   	//printf
	
//#include "usart1_init.h"
// #include "usart2_init.h"
 #include "usart3_init.h"
// #include "uart4_init.h"
// #include "uart5_init.h"
// #include "usart6_init.h"

#define PRINTFPRIO		10																						//调试用打印等级

#define DebugString 	USART3_SendString															//调试用多字符串输出
#define DebugByte 		USART3_SendByte																//调试用单字符输出
	
//不用信号量来打印显示用，用信号来打印显示的请参考CommandSendJob.c
#define DebugPf(x,y) 	if(x < PRINTFPRIO)	printf(y);								//调试用打印输出
#define DebugPfEx(x,y,z) 	if(x < PRINTFPRIO)	printf(y,z);					//调试用打印输出带参数

u8 Get_PrintfPort(void);   																					//获取打印输出端口			(1-6)
void Set_PrintfPort(const u8 Port);																	//设置printf输出端口		(1-6)

#ifdef _COMMANDSENDJOB_H_				//用信号量来打印显示用，具体请参考CommandSendJob.c
u8 SendToPrintf(u8* ShowChar,const u32 Len);				//发送打印数据至打印线程
#endif

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
用例：
	#include "printf_init.h"		//printf函数

	//串口初始化后

	Set_PrintfPort(1);								//设置printf端口			
	printf("你好\r\n");								//测试printf
	printf("1sssss2\r\n");
*/
