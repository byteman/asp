#include "TIM6_init.h"	  
#include "static_init.h"						//Get_SyS_CLK

static u32 TIM6_Count = 0;					//计数器
static u8 TIM6_CntFlag = 0;					//计数器开启标志
static TIM6_fun RealTim6fun = 0;		//定时器执行函数指针

/****************************************************************************
* 名	称：void TIM6_TimeOpenOrClose(const u8 Flag)
* 功	能：开关定时器
* 入口参数：const u8 Flag		定时器开关	1开 0关
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_TimeOpenOrClose(const u8 Flag)
{
	Flag > 0 ? TIM_Cmd(TIM6, ENABLE) : TIM_Cmd(TIM6, DISABLE);
}

/****************************************************************************
* 名	称：void TIM6_ITOpenOrClose(const u8 Flag)
* 功	能：开关定时器中断
* 入口参数：const u8 Flag		定时器中断开关	1开 0关
* 出口参数：无
* 说	明：中断开启计时值增加
****************************************************************************/
void TIM6_ITOpenOrClose(const u8 Flag)
{
	Flag > 0 ? TIM_ITConfig(TIM6, TIM_FLAG_Update, ENABLE) : TIM_ITConfig(TIM6, TIM_FLAG_Update, DISABLE);
}

/****************************************************************************
* 名	称：void TIM6_TimeISR_Open(void)
* 功	能：计时中断开启
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_TimeISR_Open(void)
{
	TIM6_ITOpenOrClose(1);
	TIM6_TimeOpenOrClose(1);
	
	if(TIM6_CntFlag == 1)								//如果需要计数器计数
	{
		if(TIM6_Count == 0)								//第一次开始时
			TIM6_Count = 1;									//计时到达获取值时，最后一个中断计数还没计数
	}
}

/****************************************************************************
* 名	称：void TIM6_TimeISR_Close(void)
* 功	能：计时中断关闭
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_TimeISR_Close(void)
{
	TIM6_ITOpenOrClose(0);
	TIM6_TimeOpenOrClose(0);
}

/****************************************************************************
* 名	称：u32 TIM6_GetCount(void)
* 功	能：获得时长计时值
* 入口参数：无
* 出口参数：u32 计时次数 (毫秒 / 秒)
* 说	明：无
****************************************************************************/
u32 TIM6_GetCount(void)
{
	return TIM6_Count;
}

/****************************************************************************
* 名	称：void TIM6_ResetCount(void)
* 功	能：计时值复位从0开始
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_ResetCount(void)
{
	TIM6_Count = 0;		 //计时值复位
}

/****************************************************************************
* 名	称：void TIM6_CntFlagOnOff(const u8 OnOff)
* 功	能：开启关闭计数器
* 入口参数：const u8 OnOff			开启或关闭计数器	1开启 0关闭
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_CntFlagOnOff(const u8 OnOff)
{
	OnOff > 0 ? (TIM6_CntFlag = 1) : (TIM6_CntFlag = 0);		//计时值复位
}

/****************************************************************************
* 名	称：void TIM6_Caller(void(*TIM6_fun)(void))
* 功	能：TIM6回调函数
* 入口参数：回调执行函数
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM6_Caller(void(*TIM6_fun_t)(void))
{
	RealTim6fun = TIM6_fun_t;	//执行函数指针指向执行函数
}

/****************************************************************************
* 名	称：void TIM6_DAC_IRQHandler(void)
* 功	能：定时器中断处理
* 入口参数：无
* 出口参数：无
* 说	明：计时值增加
****************************************************************************/
void TIM6_DAC_IRQHandler(void)
{
	TIM_ClearITPendingBit(TIM6, TIM_FLAG_Update);	//清空溢出中断标志
	if(TIM6_CntFlag == 1)
	{
		if(TIM6_Count == 0xFFFFFFFF)
			TIM6_Count = 0;							//超过最大值重新回到0开始计数
		TIM6_Count++;									//计时值增加
	}
	
	if(RealTim6fun != 0)								//如果执行函数有指向
		RealTim6fun();										//执行执行函数
}

/****************************************************************************
* 名	称：u8 TIM6_Config(const TIM6_fun Cfun,const u16 TimeUS,const u8 mode)
* 功	能：定时器配置
* 入口参数：const TIM6_fun Cfun		回调函数
						const u16 TimeUS	需要定时的单位级别	微秒单位
						const u8 mode			模式 0微秒	1毫秒	2秒
* 出口参数：u8	设置是否成功		1成功 0失败
* 说	明：无
****************************************************************************/
u8 TIM6_Config(const TIM6_fun Cfun,const u16 TimeUS,const u8 mode)				//默认为毫秒中断
{
	TIM_TimeBaseInitTypeDef  T;							//创建定时器变量
	NVIC_InitTypeDef N;   									//创建中断管理变量

	u32 Period_t = 0;												//计算用定时值
	u16 Prescaler_t = 0;										//计算用分频系数
	u16 TimeUS_t = TimeUS;									//计时时间
		
	if(TimeUS_t < 1)
		TimeUS_t = 2;													//防止Period_t计数器为0
	
	switch(mode)														//定时模式区分
	{
		case Mode_1US:
		{
			Period_t = TimeUS_t - 1;																	//计算用定时值
			Prescaler_t = Get_SyS_CLK(SYSCLK) / 2 / 1000000 - 1;			//计算用分频系数
			break;
		}
		case Mode_10US:
		{
			Period_t = TimeUS_t - 1;																	//计算用定时值
			Prescaler_t = Get_SyS_CLK(SYSCLK) / 2 / 100000 - 1;				//计算用分频系数
			break;
		}
		case Mode_100US:
		{
			Period_t = TimeUS_t - 1;																	//计算用定时值
			Prescaler_t = Get_SyS_CLK(SYSCLK) / 2 / 10000 - 1;				//计算用分频系数
			break;
		}
		case Mode_500US:
		{
			Period_t = TimeUS_t - 1;																	//计算用定时值
			Prescaler_t = Get_SyS_CLK(SYSCLK) / 2 / 2000 - 1;					//计算用分频系数
			break;
		}
		default:
			return 0;														//模式错误
	}
	
	//定时器配置
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);	//给定时器分配时钟APB1	//APB1预分频为非1时,定时器时钟频率为2倍频即36*2=72MHZ
  TIM_DeInit(TIM6);																			//复位TIM2定时器到默认值
	
	//定时时间：72000000个频率为1秒，当前设置(TIM_Period+1) * (TIM_Prescaler+1)个频率后产生溢出中断
	T.TIM_Period = Period_t;										//定时值,减10000次发生一次溢出中断 (100微秒*10000=1秒) 定时器为16位的最大值65535
	T.TIM_Prescaler = Prescaler_t; 							//预分频系数	7200个频率后定时值减1	(7200个频率100微秒)
	
  T.TIM_ClockDivision = 0;										//时钟分割 在定时器预分频之前作一段延时，很少用到，设为0即可
  T.TIM_CounterMode = TIM_CounterMode_Up;			//向上计数		UP：10000到0的计数	DOWN:10000到FFFF
  TIM_TimeBaseInit(TIM6, &T);									//根据上面设置初始化定时器TIM2
  TIM_ClearFlag(TIM6, TIM_FLAG_Update);				//清空溢出中断

	//嵌套向量中断控制器设置	  
	N.NVIC_IRQChannel = TIM6_DAC_IRQn;					//配置对TIMx_IRQn的中断监听
  N.NVIC_IRQChannelPreemptionPriority = 0;		//抢先级
  N.NVIC_IRQChannelSubPriority = 0;						//响应级
  N.NVIC_IRQChannelCmd = ENABLE;							//开启中断监听
  NVIC_Init(&N); 															//根据上面设置初始化中断控制器
	
	if(Cfun != 0)																//回调函数不为空
		TIM6_Caller(Cfun);												//设置回调函数
	return 1;
}

/*
if(TIM6_Config(0,10,Mode_100US) == 0)							//默认为毫秒中断
		DebugPf(0,"定时器6设置失败\r\n");
	
TIM6_FlagOnOff(1);					//计时值复位
TIM6_ResetCount();
TIM6_TimeISR_Open();
*/
