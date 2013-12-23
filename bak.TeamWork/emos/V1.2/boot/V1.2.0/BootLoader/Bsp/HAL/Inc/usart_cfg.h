#ifndef _USART_CFG_H_
#define _USART_CFG_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"			//u32		

#define DebugXiang					//是否输出调试信息		开启输出 屏蔽不输出
	
//USART设备控制结构体
typedef struct
{
	u32			BaudRate;							//波特率
	
	//接收缓冲区
	u8			*USARTRxBuffer;				//接收缓冲区
	u32			USARTRxBufferSize;		//接收缓冲区大小

	//发送缓冲区
	u8			*USARTTxBuffer;				//发送缓冲区
	u32			USARTTxBufferSize;		//发送缓冲区大小
} USART_STRUCT;

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif
