#include "usart3_init.h"
#include "stm32f4xx.h"			//u8
#include "String.h"					//memcpy

//接收控制队列
static	u8*			USART3QStart = (u8*)0;		      //开始位置
static	u8*			USART3QEnd = (u8*)0;		        //结束位置
static  u8*			USART3QIn = (u8*)0;				    	//输入位置
static	u8*			USART3QOut = (u8*)0;				    //输出位置
static  u32			USART3QDataCount = 0;           //空间里数据个数 

//串口配置
static  USART_STRUCT*		USART3Real = (USART_STRUCT*)0;				//串口结构体指针
static 	u8 			USART3_GetChar = 0;					//接收到的单个字符				中断中使用	
static  u32			ISRSendLen = 0;							//中断需要发送的数据个数
static  u32			ISRSendCnt = 0;							//中断发送个数
static  u8			ISRSendBusyFlag = 0;				//中断发送忙状态	1忙	0不忙

/****************************************************************************
* 名	称：void USART3_GPIO_Init(void)
* 功	能：串口引脚初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void USART3_GPIO_Init(void)			//串口引脚初始化
{
	GPIO_InitTypeDef GPIO_InitStructure;		//串口引脚结构
	
	//串口引脚分配时钟
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_USART3);		//Connect PXx to USARTx_Tx  
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_USART3);	//Connect PXx to USARTx_Rx

	//配置串口 Tx (PC.10) 为复用推挽输出
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				//串口发送引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;			//轮流模式
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//转换频率
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;		//推挽输出
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	//不作上下拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);						//初始化引脚
    
	// 配置串口 Rx (PC.11) 为浮空输入
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;				//串口接收引脚
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;			//轮流模式
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//转换频率
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;	//不作上下拉
  GPIO_Init(GPIOC, &GPIO_InitStructure);						//初始化引脚
}

/****************************************************************************
* 名	称：void USART3_Init(const u32 BaudRate_t)
* 功	能：串口初始化
* 入口参数：const u32 BaudRate_t		波特率
* 出口参数：无
* 说	明：无
****************************************************************************/
void USART3_Init(const u32 BaudRate_t)
{		
	USART_InitTypeDef UART_InitStructure;		//串口结构

	//串口分配时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, ENABLE);

	//串口初始化
	UART_InitStructure.USART_BaudRate            = BaudRate_t;						//波特率
	UART_InitStructure.USART_WordLength          = USART_WordLength_8b;		//数据位8bit
	UART_InitStructure.USART_StopBits            = USART_StopBits_1;		//停止位个数
	UART_InitStructure.USART_Parity              = USART_Parity_No ;		//不进行奇偶效验
	UART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;	//RTS和CTS使能(None不使用)
	UART_InitStructure.USART_Mode                = USART_Mode_Rx | USART_Mode_Tx;	//发送和接收使能
	USART_Init(USART3, &UART_InitStructure);	//初始化串口
}

/****************************************************************************
* 名	称：void USART3_NVIC_Init(void)
* 功	能：串口中断向量表初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void USART3_NVIC_Init(void)
{
	NVIC_InitTypeDef NVIC_InitStructure; 		//中断控制器变量

	NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;			//设置中断通道
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;	//主优先级设置
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;			//设置优先级
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//打开串口中断
	NVIC_Init(&NVIC_InitStructure);								//初始化中断向量表
}

/****************************************************************************
* 名	称：void USART3_RX_Buffer_init(void)
* 功	能：串口接收初始化
* 入口参数：无
* 出口参数：无
* 说	明：接收中断与接收缓冲区绑定
****************************************************************************/
void USART3_RX_Buffer_init(void)
{
	USART3QStart = USART3Real->USARTRxBuffer;						//开始位置
	USART3QEnd = &USART3Real->USARTRxBuffer[USART3Real->USARTRxBufferSize - 1];	//结束位置
	USART3QIn = USART3QStart;														//输入位置=开始位置
	USART3QOut = USART3QStart;													//输出位置=开始位置
	USART3QDataCount = 0;																//空间里数据个数
}

/****************************************************************************
* 名	称：u8 USART3_Config(USART_STRUCT* Real)
* 功	能：串口设置
* 入口参数：USART_STRUCT* Real		结构体配置
* 出口参数：u8 是否成功	1成功	0失败
* 说	明：默认为包数据接收					 
****************************************************************************/
u8 USART3_Config(USART_STRUCT* Real)
{
	if(Real == 0)
		return 0;
	
	USART3Real = Real;						//结构体获取
	
	USART3_Init(USART3Real->BaudRate);	//串口初始化
	USART3_GPIO_Init();						//串口引脚初始化
	USART3_NVIC_Init();						//中断初始化
	USART3_RX_Buffer_init();			//接收中断与接收缓冲区绑定

	USART_ClearITPendingBit(USART3, USART_IT_RXNE);				//清接收标志
	USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);				//开启接收中断
	
	USART_Cmd(USART3, ENABLE);  	//使能失能串口外设
	
	return 1;
}

/****************************************************************************
* 名	称：void USART3_RX_Buffer_Clear(void)	
* 功	能：清空接收缓冲区
* 入口参数：无
* 出口参数：无
* 说	明：无						
****************************************************************************/
void USART3_RX_Buffer_Clear(void)
{
	USART3QIn = USART3QStart;							//输入位置=开始位置
	USART3QOut = USART3QStart;						//输出位置=开始位置
	USART3QDataCount = 0;									//空间里数据个数
	memset(USART3Real->USARTRxBuffer,0,USART3Real->USARTRxBufferSize);
}

/****************************************************************************
* 名	称：u32 LookUSART3_GetBuffCount(void)
* 功	能：获取缓冲区中接收到的数据个数
* 入口参数：无
* 出口参数：无
* 说	明：无					  
****************************************************************************/
u32 LookUSART3_GetBuffCount(void)
{
	return USART3QDataCount;
}

/****************************************************************************
* 名	称：u8 USART3_GetByte(u8* Data)	
* 功	能：缓冲区中提取单字符数据
* 入口参数：u8* Data 接收到的数据
* 出口参数：u8	接收成功标志
			0 	接收成功
			1 	接收失败
* 说	明：从接收缓冲区中获取数据，不会持续等待
***************************************************************************/
u8 USART3_GetByte(u8* Data)
{
	if(USART3QDataCount != 0)				//有未处理数据存在
	{
		*Data = *USART3QOut;					//从队列中取数据
		USART3QOut++;									//指向下一个未处理数据点
		if(USART3QOut > USART3QEnd)		//到了最后一个判断点则跳回开始点
			USART3QOut = USART3QStart;	
		USART3QDataCount--;						//未处理数据个数递减

		return 1;
	}
	return 0;
}

/****************************************************************************
* 名	称：u8 USART3_GetByte_WaitTime(u8* Data,const u32 TimeLater)	
* 功	能：u8* Data 接收到的数据
			const u32 TimeLater	等待时间 7=1us
* 入口参数：u8	是否有接收到数据
				0 没有接收到数据
				1 接收到数据
* 出口参数：接收到的单字符数据
* 说	明：从接收缓冲区中获取数据，时长等待
***************************************************************************/
u8 USART3_GetByte_WaitTime(u8* Data,const u32 TimeLater)
{
	u8 flag = 0;					//获取标志
	u32 err = 0;					//错误标志
	u8 val = 0;					//获取的数据存放点
	
	while(1)
	{
		if(LookUSART3_GetBuffCount() == 0)	//缓冲区中没有数据
		{	
			err++;
			if(err >= TimeLater)	//10毫秒都没获取到数据，错误退出
				return 0;
		}
		else
		{
			flag = USART3_GetByte(&val);
			if (flag == 1)
			{
				*Data = val;
				return 1;
			}
			else
				return 0;	
		}
	}
}

/****************************************************************************
* 名	称：u32 USART3_GetString(u8* Data ,const u32 len)	
* 功	能：缓冲区中提取字符串数据
* 入口参数：u8* Data 接收到的数据
			const u32 len	 需要接收的长度
* 出口参数：u32	获取的数据个数		0获取失败
* 说	明：从接收缓冲区中获取数据，不会持续等待 
  
***************************************************************************/
u32 USART3_GetString(u8* Data ,const u32 len)
{
	u32 i = 0;
	u32 cnt = len;
	
	if(cnt == 0)													//准备接收0字节数据
		return 0;														//接收失败

	if(USART3QDataCount == 0)					   	//如果缓冲区没有数据
		return 0;									   				//接收失败

	if(cnt > USART3QDataCount)				   	//要取的个数比目前接收的个数还多
		cnt = USART3QDataCount;				   		//取所接收到的所有数据

	if((USART3QOut + cnt - 1) > (USART3QEnd + 1))		//有部分数据在缓冲区的前端		-1从0开始存储的
	{
		i = USART3QEnd - USART3QOut + 1;							//后半部分的数据个数
		memcpy(Data , USART3QOut , i);								//取后N个数据
		memcpy(Data + i , USART3QStart , cnt - i);	 	//取前N个数据
		USART3QOut = USART3QStart + cnt - i;	 				//更新取数位置
	}
	else
	{
		memcpy(Data,USART3QOut,cnt);	 		//取N个数据
		USART3QOut += cnt;								//指向下一个未处理数据点
	}

	if(USART3QOut > USART3QEnd)	 				//输出位置超出最后的一个地址
		USART3QOut = USART3QOut - USART3QEnd + USART3QStart - 1;	   //-1从0开始	//计算输出地址
	USART3QDataCount -= cnt;			 			//减掉接收了的数据个数
	
	return cnt;								 //返回获取的个数
}

/****************************************************************************
* 名	称：void USART3_SendByte(const u8 Data)
* 功	能：单字符发送
* 入口参数：const u8 Data 	发送单字符数据
* 出口参数：无
* 说	明：无				   
****************************************************************************/
void USART3_SendByte(const u8 Data)		   //单字符数据输出
{
	USART_SendData(USART3, Data);
	while(USART_GetFlagStatus(USART3, USART_FLAG_TXE) == RESET);
}

/****************************************************************************
* 名	称：void USART3_SendString(u8* Data,const u32 Len)
* 功	能：多字符输出
* 入口参数：u8* Data 	发送的数据
						const u32 Len				字符个数
* 出口参数：无
* 说	明：无					 
****************************************************************************/
void USART3_SendString(u8* Data,const u32 Len)		//多字符输出
{
	if(Len == 0)										//长度为0
		return;
	
	if(ISRSendBusyFlag == 1)				//发送忙状态
		return;
	
	if(Len > USART3Real->USARTTxBufferSize)			//缓冲区长度是否足够
		return;
	
	memcpy(USART3Real->USARTTxBuffer,Data,Len);	//拷贝数据到缓冲区
	
	ISRSendLen = 0;									//清除需要发送的数据个数
	ISRSendLen = Len;								//记录需要发送的数据个数
	ISRSendBusyFlag = 1;						//开启忙状态
	
	USART_SendData(USART3, USART3Real->USARTTxBuffer[0]);	//发送第一个字节			
	ISRSendCnt = 1;									//发送计数
	USART_ITConfig(USART3, USART_IT_TXE, ENABLE);		//开启发送中断	
}

/****************************************************************************
* 名	称：u8 USART3CheckISRSendBusy(void)	
* 功	能：中断发送忙状态检测
* 入口参数：无
* 出口参数：u8		1忙	0不忙
* 说	明：上一次的发送是否完成，忙则等待
****************************************************************************/
u8 USART3CheckISRSendBusy(void)	
{
		return ISRSendBusyFlag;
}

/****************************************************************************
* 名	称：void USART3_IRQHandler(void)	
* 功	能：中断机制
* 入口参数：无
* 出口参数：无
* 说	明：接收到的数据存入接收缓冲区
	USART_GetITStatus		检查指定的USART中断发生与否
	USART_GetFlagStatus		检查指定的USART标志位设置与否
****************************************************************************/
void USART3_IRQHandler(void)
{	
	//接收中断触发
	if(USART_GetITStatus(USART3, USART_IT_RXNE) == SET)		
	{
		//数据接收动作
		USART_ClearITPendingBit(USART3, USART_IT_RXNE);				//清空接收中断标志

		USART3_GetChar = USART_ReceiveData(USART3);								//接收到的字符数据
		
		if(USART3QDataCount < USART3Real->USARTRxBufferSize)			//如果空间未满,保存数据 
		{
			if(USART3QIn > USART3QEnd)			//如果有空间可存数据，但输入位置在末尾位置
				USART3QIn = USART3QStart;			//将输入位置跳到开始位置
			*USART3QIn = USART3_GetChar;		//接收到的字符数据存入缓冲区输入位置
			USART3QIn++;										//输入位置增加
			USART3QDataCount++;							//空间里数据个数增加	
		}
	}
	
	else if(USART_GetFlagStatus(USART3, USART_IT_ORE) == SET)		//检测是否有接收溢出
		USART_ReceiveData(USART3);									//清接收溢出标志，只能用读数据的方式来清溢出标志

	//发送中断触发
	if(USART_GetITStatus(USART3, USART_IT_TXE) == SET)		//发送中断
	{		
		if(ISRSendCnt >= ISRSendLen)												//数据发送完
		{
			USART_ITConfig(USART3, USART_IT_TXE, DISABLE);		//禁止发送移位缓冲区空中断
			ISRSendBusyFlag = 0;					//发送空闲
		}
		else																								//数据未发送完
		{
			USART_SendData(USART3, USART3Real->USARTTxBuffer[ISRSendCnt]);
			ISRSendCnt++;
		}		
	}
}
