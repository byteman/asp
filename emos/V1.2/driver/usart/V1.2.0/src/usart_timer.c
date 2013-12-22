#include "usart_timer.h"
#include "stm32f4xx.h"			//stm32
#include "C2.h"							//SYS_EVENT
#include "usart1.h"
#include "usart6.h"
#define usart1_Activate			1				//1激活		0冻结
#define usart2_Activate			0				//1激活		0冻结
#define usart3_Activate			0				//1激活		0冻结
#define uart4_Activate			0				//1激活		0冻结

#if(usart1_Activate)
extern void USART1_SetStatus(const u8 Status);											//设置串口触发状态
extern SYS_EVENT* GetUSART1_ReceiveFinishEvent(void);								//获取接收完成信号量
extern u8 LookUSART1_SrcStatus(void);																//获取串口触发源状态
extern u8 LookUSART1_WorkStatusVal(void);														//查看状态信号量触发后串口的工作状态值
#endif

#if(usart2_Activate)
extern void USART2_SetStatus(const u8 Status);											//设置串口触发状态
extern SYS_EVENT* GetUSART2_ReceiveFinishEvent(void);								//获取接收完成信号量
extern u8 LookUSART2_SrcStatus(void);																//获取串口触发源状态
extern u8 LookUSART2_WorkStatusVal(void);														//查看状态信号量触发后串口的工作状态值
#endif

#if(usart3_Activate)
extern void USART3_SetStatus(const u8 Status);											//设置串口触发状态
extern SYS_EVENT* GetUSART3_ReceiveFinishEvent(void);								//获取接收完成信号量
extern u8 LookUSART3_SrcStatus(void);																//获取串口触发源状态
extern u8 LookUSART3_WorkStatusVal(void);														//查看状态信号量触发后串口的工作状态值
#endif

#if(uart4_Activate)
extern void UART4_SetStatus(const u8 Status);												//设置串口触发状态
extern SYS_EVENT* GetUART4_ReceiveFinishEvent(void);								//获取接收完成信号量
extern u8 LookUART4_SrcStatus(void);																//获取串口触发源状态
extern u8 LookUART4_WorkStatusVal(void);														//查看状态信号量触发后串口的工作状态值
#endif

#define TIM3_FullOut	0			//定时器溢出检测开关
#define USARTSTATUS_General_Bak								0x00		//通常状态		//已经在usart_cfg.h中定义了
#define USARTSTATUS_Bag_Bak										0x08		//包接收			//已经在usart_cfg.h中定义了

/****************************************************************************
* 名	称：void TIM3_Init(void)
* 功	能：定时器初始化
* 入口参数：无
* 出口参数：无
* 说	明：Get_SyS_CLK自行检测系统主频

3字节时间表	  参考设置USART1_Real.TimeVal延时值
115200	= 1000000	/	11520 = 86us*3 		= 258us		/	20us	=	计数12.9次	
57600		= 1000000	/	5760 	= 173us*3 	= 519us		/	20us	=	计数25次
38400		= 1000000	/	3840 	= 260us*3 	= 781us		/	20us	=	计数39次
19200		= 1000000	/	1920 	= 520us*3 	= 1560us	/	20us	=	计数78次
9600		= 1000000	/	960 	= 1041us*3 	= 3123us	/	20us	=	计数156次
****************************************************************************/
void TIM3_Init(void)
{
	TIM_TimeBaseInitTypeDef  T;	//创建定时器变量

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	  	//TIM3使用APB1时钟总线
	TIM_DeInit(TIM3);																					//复位TIM3定时器到默认值

	T.TIM_Period = 65535;											//计数器溢出值，最大65535，此值不能更改，与输出比较的设定值一致，输出比较计数器溢出也为65535
	T.TIM_Prescaler = (SYS_CPU_SysTickClkFreq() / 2) / 50000 - 1; 	//预分频系数，系统每秒168,000,000次，计数器2分频，即每秒84,000,000次，以系统频率来记此处为2秒，168,000,000/100,000=1680(20us)，即每次计数时间为20us，每计满TIM_Period多次时变0重计
	T.TIM_ClockDivision = 0;									//时钟分割 在定时器预分频之前作一段延时，很少用到，设为0即可
  T.TIM_CounterMode = TIM_CounterMode_Up;		//向上计数		UP：0到10000的计数	DOWN:FFFF到10000 
  TIM_TimeBaseInit(TIM3, &T);								//根据上面设置初始化定时器TIM3

  TIM_ClearFlag(TIM3, TIM_FLAG_Update);			//清空溢出中断
  TIM_SetCounter(TIM3,0);										//设置计数器初值为0	//目前尚未启用该定时器
}

/****************************************************************************
* 名	称：void TIM3_OC(void)
* 功	能：定时器输出比较功能初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void TIM3_OC(void)
{
	TIM_OCInitTypeDef O;  	//创建输出比较设置变量
	
  O.TIM_OCMode = TIM_OCMode_Timing; 			  		//配置成输出比较计时模式
  O.TIM_OutputState = TIM_OutputState_Disable;  //禁止OC输出比较
  O.TIM_OCPolarity = TIM_OCPolarity_Low; 		  	//输出比较极为低

	O.TIM_Pulse = 0;				//初始化输出比较通道值	最大65535溢出，和TIM_Period一致

	TIM_OC1Init(TIM3, &O); 	//初始化输出比较通道1
	TIM_OC2Init(TIM3, &O); 	//初始化输出比较通道2
	TIM_OC3Init(TIM3, &O); 	//初始化输出比较通道3
	TIM_OC4Init(TIM3, &O); 	//初始化输出比较通道4	
}

/****************************************************************************
* 名	称：void TIM3_IC(void)
* 功	能：定时器输入捕获功能初始化
* 入口参数：无
* 出口参数：无
* 说	明：暂未实现
****************************************************************************/ 
void TIM3_IC(void)
{
	TIM_ICInitTypeDef I;  													//创建输出比较设置变量
	
	//输入捕获配置	TIMx_CCRx
	I.TIM_Channel = TIM_Channel_1; 				  				//使用输入捕获通道
	I.TIM_ICPolarity = TIM_ICPolarity_Rising; 	  	//输入捕获上升沿有效
	I.TIM_ICSelection = TIM_ICSelection_DirectTI; 	//IC*输入引脚选择，针对IC1/IC2有不同的定义
	I.TIM_ICPrescaler = TIM_ICPSC_DIV1; 		  			//输入模式下，捕获端口上的每一次边沿都触发一次捕获
	I.TIM_ICFilter = 0; 						  							//捕获采样频率，详见TIM*_CCMR->IC*F说明
	TIM_ICInit(TIM3, &I); 						  						//初始化输入
}

/****************************************************************************
* 名	称：void TIM3_NVIC_Init(void)
* 功	能：定时器中断向量表初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM3_NVIC_Init(void)
{
	NVIC_InitTypeDef N;
		  
	N.NVIC_IRQChannel = TIM3_IRQn;						//配置对TIM3_IRQn的中断监听
  N.NVIC_IRQChannelPreemptionPriority = 0;	//抢先级
  N.NVIC_IRQChannelSubPriority = 0;					//响应级
  N.NVIC_IRQChannelCmd = ENABLE;						//开启中断监听
  NVIC_Init(&N); 														//根据上面设置初始化中断控制器
}

/****************************************************************************
* 名	称：void TIM3_Config(void)
* 功	能：配置定时器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/	 
void usart_timer_init(void)
{
	TIM3_Init();			   	//定时器初始化
	TIM3_OC();						//输出比较初始化
//	TIM3_IC();					//输入捕获初始化	暂未实现
	TIM3_NVIC_Init();			//中断向量表初始化

	//清空所有中断	溢出.输出比较通道1.输出比较通道2.输出比较通道3.输出比较通道4
	TIM_ClearITPendingBit(TIM3 , TIM_FLAG_Update | TIM_FLAG_CC1 | TIM_FLAG_CC2 | TIM_FLAG_CC3 | TIM_FLAG_CC4 | TIM_FLAG_Trigger);
	
	if(TIM3_FullOut)
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);	  //开启溢出中断

	TIM_Cmd(TIM3, ENABLE);	  //开启TIM3并计数	(仿真器断点暂停时计数器依然会计数)
}

/****************************************************************************
* 名	称：void TIM3_IRQHandler(void)
* 功	能：定时器3中断
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/	 
void TIM3_IRQHandler(void)		//TIM3中断处理 计数器不会因仿真器暂停而暂停
{
	SysIntEnter();																			//进入中断进行任务优先级调度保护

	if(TIM_GetITStatus(TIM3, TIM_FLAG_Update) != RESET) //溢出中断
 	{
		TIM_ITConfig(TIM3, TIM_IT_Update, DISABLE);	 			//关闭中断
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_Update);			//清除中断标志
	}

	if(TIM_GetITStatus(TIM3, TIM_FLAG_CC1) != RESET)	 	//输出比较通道1中断
 	{	
		TIM_ITConfig(TIM3, TIM_IT_CC1, DISABLE);					//关闭中断
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC1);				//清除中断标志
		usart1_timer_isr();
	}
	
	if(TIM_GetITStatus(TIM3, TIM_FLAG_CC2) != RESET)	 								//输出比较通道1中断
	{
		TIM_ITConfig(TIM3, TIM_IT_CC2, DISABLE);												//关闭中断
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC2);											//清除中断标志
		usart6_timer_isr();
 	}

	if(TIM_GetITStatus(TIM3, TIM_FLAG_CC3) != RESET)	 //输出比较通道3中断
 	{
		TIM_ITConfig(TIM3, TIM_IT_CC3, DISABLE);	//关闭中断
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC3);	//清除中断标志
 	}
	
	if(TIM_GetITStatus(TIM3, TIM_FLAG_CC4) != RESET)	 //输出比较通道4中断
 	{
		TIM_ITConfig(TIM3, TIM_IT_CC4, DISABLE);	//关闭中断
		TIM_ClearITPendingBit(TIM3, TIM_FLAG_CC4);	//清除中断标志
 	}

	SysIntExit();	//退出中断进行任务优先级重新调度
}
