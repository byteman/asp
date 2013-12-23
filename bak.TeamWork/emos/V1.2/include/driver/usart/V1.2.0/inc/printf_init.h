#ifndef _PRINTF_INIT_H_	
#define _PRINTF_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include <stdio.h>		   	//printf FILE
#include "includes.h"			
#include "driver.h"				//HANDLE
	
#ifdef _USART1_INIT_H_
#include "usart1_init.h"
#endif

#ifdef _USART2_INIT_H_
#include "usart2_init.h"
#endif

#ifdef _USART3_INIT_H_
#include "usart3_init.h"
#endif

#ifdef _UART4_INIT_H_
#include "uart4_init.h"
#endif

#ifdef _UART5_INIT_H_
#include "uart5_init.h"
#endif

#ifdef _USART6_INIT_H_
#include "usart6_init.h"
#endif

//不用信号量来打印显示用，用信号来打印显示的请参考CommandSendJob.c
#define DebugPf 			printf								//调试用打印输出
#define DebugString 	USART1_SendString			//调试用多字符串输出
#define DebugByte 		USART1_SendByte				//调试用单字符输出

typedef struct
{
	u8* pData;						//打印数据
	u32 pDataLen;					//打印数据长度
}PrintfData;						//打印数据结构体

/****************************************************************************
* 名	称：u8 Get_PrintfPort(void)
* 功	能：获取打印输出端口
* 入口参数：无
* 出口参数：u8 当前打印输出端口号	0获取失败
* 说	明：无
****************************************************************************/
u8 Get_PrintfPort(void);   									//获取打印输出端口			(1-6)

/****************************************************************************
* 名	称：void Set_PrintfPort(u8 Port)
* 功	能：设置打印输出端口
* 入口参数：u8	打印显示端口号 串口1 = 1	串口2 = 2 .....
* 出口参数：无
* 说	明：无
****************************************************************************/
void Set_PrintfPort(u8 Port);								//设置printf输出端口		(1-6)

/****************************************************************************
* 名	称：void SetPrintfEvent(SYS_EVENT* JOBEvent)
* 功	能：设置打印信号量
* 入口参数：SYS_EVENT*	打印信号量地址
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetPrintfEvent(SYS_EVENT* JOBEvent);		//设置打印信号量	

/****************************************************************************
* 名	称：u8 WaitPrintfEventAndPrintf(HANDLE USARTx,u32 Times)
* 功	能：等待打印信号量并打印数据
* 入口参数：HANDLE USARTx				设备句柄
						u32 Times						等待时间10MS
* 出口参数：u8	是否成功	0失败	1成功
* 说	明：
typedef struct
{
	u8* pData;						//打印数据
	u32 pDataLen;					//打印数据长度
}PrintfData;						//打印数据结构体
****************************************************************************/
u8 WaitPrintfEventAndPrintf(HANDLE USARTx,u32 Times);	//等待打印信号并打印数据

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
用例：
#include "usart1_init.h"
#include "usart_cfg.h"
#include "printf_init.h"

USART_STRUCT usart1cfg = 
{
	57600,																	//波特率
	USART_WordLength_8b,										//数据位
	USART_StopBits_1,												//停止位
	USART_Parity_No													//效验位
};

USART1_Config(&usart1cfg);												//串口初始化

Set_PrintfPort(1);						//设置打印串口
printf("测试开始\r\n");
*/

/*
JOB1
SYS_EVENT* DefaultJob1Event = (SYS_EVENT*)0;					//定义邮箱
if(DefaultJob1Event == (SYS_EVENT*)0)									
		DefaultJob1Event = SysMboxCreate((void *)0);			//创建邮箱信号量
SetPrintfEvent(DefaultJob1Event);											//设置打印信号量	
if(WaitPrintfEventAndPrintf(USART1_Dev,0) == 0)				//等待打印信号并打印数据
		while(1);																					//等待打印出错
		
JOB2
PrintfData senddata = 																//打印信息
	{
		"test\r\n",				//发送的数据
		6									//发送数据的个数
	};
if(DefaultJob1Event != (SYS_EVENT*)0)
	SysMboxPost(DefaultJob1Event, (void*)&senddata);		//发送激活等待的邮箱
*/
