#ifndef _USART6_INIT_H_
#define _USART6_INIT_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif
	
#include "usart_cfg.h"			//串口配置文件

//串口设置u8 USART6_Config(USART_STRUCT* Real);					//按配置初始化串口
void USART6_Init(const u32 BaudRate_t);				//设置波特率
	
//功能函数
void USART6_RX_Buffer_Clear(void);						//清空缓冲区

//状态查询	
u32 LookUSART6_GetBuffCount(void);						//获取缓冲区中接收到的数据个数
u8 USART6CheckISRSendBusy(void);							//中断发送忙状态检测

//接收数据
u8 USART6_GetByte(u8* Data);									//缓冲区中提取单字符数据
u32 USART6_GetString(u8* Data ,const u32 len);			//缓冲区中提取字符串数据
u8 USART6_GetByte_WaitTime(u8* Data,const u32 TimeLater);		//时间内提取字符数据

//发送数据
void USART6_SendByte(const u8 Data);								//单字符发送
void USART6_SendString(u8* Data,const u32 Len);			//多字符输出
void USART6_DMASendString(const u32 Len);						//DMA方式多字符输出

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
串口使用：
【1：包含头文件】
#include "usart1_init.h"		//串口函数
#include "usart_cfg.h"			//串口配置
【2：开辟收发缓冲区】
//发送缓冲区
#define USART1_RX_BUFF_SIZEMAX 128
static u8 USART1RxBuffer[USART1_RX_BUFF_SIZEMAX];	//串口接收缓存区
//发送缓冲区
#define USART1_TX_BUFF_SIZEMAX 128
static u8 USART1TxBuffer[USART1_TX_BUFF_SIZEMAX];	//串口发送缓存区
【3：配置串口】
//串口配置变量
USART_STRUCT USART1_Real = 
{
	57600,										//波特率

	//接收缓冲区
	USART1RxBuffer,						//接收缓冲区
	USART1_RX_BUFF_SIZEMAX,		//接收缓冲区大小

	//发送缓冲区
	USART1TxBuffer,						//发送缓冲区
	USART1_TX_BUFF_SIZEMAX,		//发送缓冲区大小
};
【4：初始化串口】
	if(USART1_Config(&USART1_Real) == 0)		//按配置初始化串口
		return 0;	
【5：串口收发应用】
*/

/*
直接从缓冲区中取数据：
	u8 flag = 0;						//获取是否结束		1结束		0获取
	u32 len = 0;						//接收到的数据个数
	u8 CharVal = 0;					//获取的单字符
	u8 StringVal[5] = {0};	//获取的字符串
	u32 ISRCnt = 0;					//对ISR发送等待进行计数
	u32 DMACnt = 0;					//对DMA发送等待进行计数

	if(USART1_Config(&USART1_Real) == 0)		//按配置初始化串口
		return 0;		

	while(!flag);											//获取数据	修改标志获取结束
	flag = 0;

	len = USART1_GetByte(&CharVal);				//缓冲区中提取单字符数据
	USART1_SendByte(CharVal);							//显示字符
	len = USART1_GetString(StringVal,3);	//缓冲区中提取字符串数据
	USART1_SendString(StringVal,len);			//显示字符串
	while(USART1CheckISRSendBusy()) ISRCnt++; 	//等待ISR发送中断将数据发送完												
	USART1_DMASendString(len);						//DMA显示字符串	
	while(USART1CheckDMASendBusy()) DMACnt++; 	//等待DMA发送中断将数据发送完
*/
