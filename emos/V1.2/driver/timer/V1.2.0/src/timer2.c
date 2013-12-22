#include "timer2.h"	   
#include "includes.h"
#include "driver.h"

static u32 TIM2_Count = 0;			//计数器
static u8 TIM2_CntFlag = 0;			//计数器开启标志

static TIM2_fun RealTim2fun = 0;		//定时器5执行函数指针			必须static
static u8 TIM2_pDisc[] = "LuoHuaiXiang_Tim2\r\n";		//设备描述字符

/****************************************************************************
* 名	称：void TIM2_TimeOpen(void)
* 功	能：开启定时器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_TimeOpen(void)
{
	TIM_Cmd(TIM2, ENABLE);		//调用固件库函数
}

/****************************************************************************
* 名	称：void TIM2_TimeClose(void)
* 功	能：关闭定时器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_TimeClose(void)
{
	TIM_Cmd(TIM2, DISABLE);		//调用固件库函数
}

/****************************************************************************
* 名	称：void TIM2_ITOpen(void)
* 功	能：开启定时器中断
* 入口参数：无
* 出口参数：无
* 说	明：中断开启计时值增加
****************************************************************************/
void TIM2_ITOpen(void)
{
	TIM_ITConfig(TIM2, TIM_FLAG_Update, ENABLE);	   //打开TIM5的更新标记TIM_FLAG_Update中断
}

/****************************************************************************
* 名	称：void TIM2_ITClose(void)
* 功	能：关闭定时器中断
* 入口参数：无
* 出口参数：无
* 说	明：中断关闭计时值不会增加
****************************************************************************/
void TIM2_ITClose(void)
{
	TIM_ITConfig(TIM2, TIM_FLAG_Update, DISABLE);	   //关闭TIM2的更新标记TIM_FLAG_Update中断
}

/****************************************************************************
* 名	称：void TIM2_TimeISR_Open(void)
* 功	能：计时中断开启
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_TimeISR_Open(void)
{
	TIM2_ITOpen();
	TIM2_TimeOpen();
	
	if(TIM2_CntFlag == 1)								//如果需要计数器计数
	{
		if(TIM2_Count == 0)								//第一次开始时
			TIM2_Count = 1;									//计时到达获取值时，最后一个中断计数还没计数
	}
}

/****************************************************************************
* 名	称：void TIM2_TimeISR_Close(void)
* 功	能：计时中断关闭
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_TimeISR_Close(void)
{
	TIM2_ITClose();
	TIM2_TimeClose();
}

/****************************************************************************
* 名	称：u32 TIM2_GetCount(void)
* 功	能：获得时长计时值
* 入口参数：无
* 出口参数：u32 计时次数 (毫秒 / 秒)
* 说	明：无
****************************************************************************/
u32 TIM2_GetCount(void)
{
	return TIM2_Count;
}

/****************************************************************************
* 名	称：void TIM2_ResetCount(void)
* 功	能：计时值复位从0开始
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_ResetCount(void)
{
	TIM2_Count = 0;		 //计时值复位
}

/****************************************************************************
* 名	称：void TIM2_FlagOnOff(u8 OnOff)
* 功	能：开启关闭计数器
* 入口参数：u8 OnOff			开启或关闭计数器	1开启 0关闭
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_FlagOnOff(u8 OnOff)
{
	if(OnOff > 0)
		TIM2_CntFlag = 1;		 //计时值复位
	else
		TIM2_CntFlag = 0;		 //计时值复位
}

/****************************************************************************
* 名	称：void TIM2_Caller(void(*TIM2_fun)(void))
* 功	能：TIM7回调函数
* 入口参数：回调执行函数
* 出口参数：无
* 说	明：无
****************************************************************************/
void TIM2_Caller(void(*TIM2_fun_t)(void))
{
	RealTim2fun = TIM2_fun_t;	//执行函数指针指向执行函数
}

/****************************************************************************
* 名	称：void TIM2_DAC_IRQHandler(void)
* 功	能：定时器中断处理
* 入口参数：无
* 出口参数：无
* 说	明：计时值增加
****************************************************************************/
void TIM2_IRQHandler(void)
{
	SysIntEnter();	//进入中断进行任务优先级调度保护

	TIM_ClearITPendingBit(TIM2, TIM_FLAG_Update);	//清空溢出中断标志
	if(TIM2_CntFlag == 1)
	{
		if(TIM2_Count == 0xFFFFFFFF)
			TIM2_Count = 0;							//超过最大值重新回到0开始计数
		TIM2_Count++;									//计时值增加
	}
	
	if(RealTim2fun != 0)								//如果执行函数有指向
		RealTim2fun();										//执行执行函数
		
	SysIntExit();   //退出中断进行任务优先级重新调度
}

/****************************************************************************
* 名	称：static s8 TIM2_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 TIM2_Open(u32 lParam)
{
	return 0;					
}

/****************************************************************************
* 名	称：static s8 TIM2_Close(void)
* 功	能：统一接口
* 入口参数：无
* 出口参数：s8						关闭操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 TIM2_Close(void)
{
	return 0;		
}

/****************************************************************************
* 名	称：static s32 TIM2_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：串口读
* 入口参数：u8* buffer		接收缓冲区
						u32 len				希望接收的长度
						u32* Reallen	实际接收的长度
						u32 lParam		参数
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 TIM2_Read(u8* buffer,u32 len) 
{
	return TIM2_GetCount();					
}

/****************************************************************************
* 名	称：static s32 TIM2_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：串口读
* 入口参数：u8* buffer		接收缓冲区
						u32 len				希望发送的长度
						u32* Reallen	实际发送的长度
						u32 lParam		参数
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 TIM2_Write(u8* buffer,u32 len) 
{
	return 0;						
}

/****************************************************************************
* 名	称：static s32 TIM2_Ioctl(u32 cmd,u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 TIM2_Ioctl(u32 cmd,u32 lParam)
{
	switch(cmd)
	{
		case TIM2_CMDClearCnt:	//清空计数器
		{
			TIM2_ResetCount();
			return 1;							//成功
		}
		case TIM2_CMDTimOpen:		//打开定时器并响应中断
		{
			TIM2_TimeISR_Open();
			return 1;							//成功
		}
		case TIM2_CMDTimClose:	//关闭定时器并关闭中断
		{
			TIM2_TimeISR_Close();
			return 1;							//成功
		}
		case TIM2_CMDSetCntFlag:	//关闭定时器并关闭中断
		{
			TIM2_FlagOnOff((u8)lParam);
			return 1;							//成功
		}
		case TIM2_CMDSetCaller:	//设置回调函数
		{
			TIM2_Caller((TIM2_fun)((u32*)lParam));
			return 1;							//成功
		}
	}
	return 0;	//错误命令	
}	

/****************************************************************************
* 名	称：u8 TIM2_Config(TIM5_fun Cfun,u16 TimeUS,u8 mode)
* 功	能：定时器配置
* 入口参数：TIM2_fun Cfun		回调函数
						u16 TimeUS	需要定时的单位级别	微秒单位
						u8 mode			模式 0微秒	1毫秒	2秒
* 出口参数：u8	设置是否成功		1成功 0失败
* 说	明：无
****************************************************************************/
u8 TIM2_Config(TIM2_fun Cfun,u16 TimeUS,u8 mode)				//默认为毫秒中断
{
	//注册用变量
	DEV_REG tim2 = 			//设备注册信息表						不必static		
	{
		CHAR_TIM2,					//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		TIM2_pDisc,					//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)TIM2_Open,		//设备打开函数指针
		(u32*)TIM2_Close, 	//设备关闭函数指针
		(u32*)TIM2_Read,		//字符读函数
		(u32*)TIM2_Write,		//字符写函数
		(u32*)TIM2_Ioctl		//控制函数
	};
	
	TIM_TimeBaseInitTypeDef  T;							//创建定时器变量
	NVIC_InitTypeDef N;   									//创建中断管理变量

	u32 Period_t = 0;												//计算用定时值
	u16 Prescaler_t = 0;										//计算用分频系数
	
	if(TimeUS < 1)
		TimeUS = 2;														//防止Period_t计数器为0
	
	switch(mode)														//定时模式区分
	{
		case Mode_1US:
		{
			Period_t = TimeUS - 1;																				//计算用定时值
			Prescaler_t = SYS_CPU_SysTickClkFreq() / 2 / 1000000 - 1;			//计算用分频系数
			break;
		}
		case Mode_10US:
		{
			Period_t = TimeUS - 1;																				//计算用定时值
			Prescaler_t = SYS_CPU_SysTickClkFreq() / 2 / 100000 - 1;			//计算用分频系数
			break;
		}
		case Mode_100US:
		{
			Period_t = TimeUS - 1;																				//计算用定时值
			Prescaler_t = SYS_CPU_SysTickClkFreq() / 2 / 10000 - 1;				//计算用分频系数
			break;
		}
		case Mode_500US:
		{
			Period_t = TimeUS - 1;																				//计算用定时值
			Prescaler_t = SYS_CPU_SysTickClkFreq() / 2 / 2000 - 1;				//计算用分频系数
			break;
		}
		default:
			return 0;														//模式错误
	}
	
	//定时器配置
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);	//给定时器分配时钟APB1	//APB1预分频为非1时,定时器时钟频率为2倍频即36*2=72MHZ
  TIM_DeInit(TIM2);																			//复位TIM2定时器到默认值
	
	//定时时间：72000000个频率为1秒，当前设置(TIM_Period+1) * (TIM_Prescaler+1)个频率后产生溢出中断
	T.TIM_Period = Period_t;										//定时值,减10000次发生一次溢出中断 (100微秒*10000=1秒) 定时器为16位的最大值65535
	T.TIM_Prescaler = Prescaler_t; 							//预分频系数	7200个频率后定时值减1	(7200个频率100微秒)
	
  T.TIM_ClockDivision = 0;										//时钟分割 在定时器预分频之前作一段延时，很少用到，设为0即可
  T.TIM_CounterMode = TIM_CounterMode_Up;			//向上计数		UP：10000到0的计数	DOWN:10000到FFFF
  TIM_TimeBaseInit(TIM2, &T);									//根据上面设置初始化定时器TIM2
  TIM_ClearFlag(TIM2, TIM_FLAG_Update);				//清空溢出中断

	//嵌套向量中断控制器设置	  
	N.NVIC_IRQChannel = TIM2_IRQn;							//配置对TIMx_IRQn的中断监听
  N.NVIC_IRQChannelPreemptionPriority = 0;		//抢先级
  N.NVIC_IRQChannelSubPriority = 0;						//响应级
  N.NVIC_IRQChannelCmd = ENABLE;							//开启中断监听
  NVIC_Init(&N); 															//根据上面设置初始化中断控制器
	
	if(Cfun != 0)																//回调函数不为空
		TIM2_Caller(Cfun);												//设置回调函数
	
	if(DeviceInstall(&tim2) != HVL_NO_ERR)			//设备注册
		return 0;
	
	return 1;
}

/*
	u32 LaterTime = 0;

	TIM7_Config(1);							//TIM7	定时器	1=秒 	0=毫秒
	TIM7_ResetCount();					//计时值复位
	TIM7_TimeLater_Open();			//开始测量
	OSTimeDly(100);							//1S	1个系统时钟为10MS
	TIM7_TimeLater_Close();			//结束测量
	LaterTime = TIM7_GetCount();	//获取计时值	9S
*/
