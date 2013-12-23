#include "rtc_init.h"
#include "printf_init.h"		//DebugPf

//未实现WakeUP唤醒功能、Tamper功能、Time stamp event功能

typedef void (*Alarm_fun)(void); 			//函数指针
static Alarm_fun A_Alarm_fun = 0;			//警报A响应执行函数指针
static Alarm_fun B_Alarm_fun = 0;			//警报B响应执行函数指针

/****************************************************************************
* 名	称：void RTCInit(void)
* 功	能：初始化RTC时钟
* 入口参数：无
* 出口参数：无
* 说	明：无 
****************************************************************************/
void RTCInit(void)
{
  RCC_BackupResetCmd(ENABLE);					//重置RTC后备寄存器
	RCC_BackupResetCmd(DISABLE);				//重置RTC后备寄存器
	
  RCC_LSEConfig(RCC_LSE_ON);					//使用外部晶振
  while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	//等待外部晶振准备好

	RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);							//选择时钟来源
	RCC_RTCCLKCmd(ENABLE);															//RTC开启	必须用于RCC_RTCCLKConfig之后

  RTC_WaitForSynchro();																//等待RTC寄存器和APB时钟同步
}

/****************************************************************************
* 名	称：ErrorStatus SetRTC_Init(void)
* 功	能：设置日历结构
* 入口参数：无
* 出口参数：ErrorStatus		ERROR or SUCCESS
* 说	明：无
****************************************************************************/
ErrorStatus SetRTC_Init(void)
{
	RTC_InitTypeDef RTC_InitTemp;			//RTC日历结构

	RTC_InitTemp.RTC_AsynchPrediv = 0x7F;		//RTC 7位异步寄存器 <0x7F
	RTC_InitTemp.RTC_SynchPrediv = 0xFF;		//RTC 15位同步寄存器 <0x7FFF
	RTC_InitTemp.RTC_HourFormat = RTC_HourFormat_24;			//12小时制
	
	return RTC_Init(&RTC_InitTemp);
}

/****************************************************************************
* 名	称：ErrorStatus SetRTC_Time(const u8 hour,const u8 min,const u8 sec)
* 功	能：设置时间
* 入口参数：	const u8 hour		时
						const u8 min		分
						const u8 sec		秒
* 出口参数：ErrorStatus		ERROR or SUCCESS
* 说	明：因采用24小时制故无需配RTC_H12参数
					SetRTC_Time(23,59,58);		//23:59:58
****************************************************************************/
ErrorStatus SetRTC_Time(const u8 hour,const u8 min,const u8 sec)
{
	RTC_TimeTypeDef RTC_TimeTemp;			//时分秒
	
	RTC_TimeTemp.RTC_Hours = hour;					//时
	RTC_TimeTemp.RTC_Minutes = min;					//分
	RTC_TimeTemp.RTC_Seconds = sec;					//秒
    
	return RTC_SetTime(RTC_Format_BIN, &RTC_TimeTemp);		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
}

/****************************************************************************
* 名	称：ErrorStatus SetRTC_Date(const u16 year,const u8 month,const u8 day,const u8 week)
* 功	能：设置日期
* 入口参数：	const u16 year	年
						const u8 month	月
						const u8 day		日
						const u8 week		周
* 出口参数：ErrorStatus		ERROR or SUCCESS
* 说	明：SetRTC_Date(2012,10,4,4);			//2012-10-4 周四
****************************************************************************/
ErrorStatus SetRTC_Date(const u16 year,const u8 month,const u8 day,const u8 week)
{
	RTC_DateTypeDef RTC_DateTemp;			//年月日周
	
	RTC_DateTemp.RTC_Year  = year < 2000 ? (u8)year : (u8)(year % 100);	//年
	RTC_DateTemp.RTC_Month = month;			//月
	RTC_DateTemp.RTC_Date  = day;				//日
	RTC_DateTemp.RTC_WeekDay  = week;		//周
	
	return RTC_SetDate(RTC_Format_BIN, &RTC_DateTemp);		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
}

/**************************************************
**函数原型: void RTC_AlarmExtiNivc(void)
**功能:		RTC报警中断配置
**输入参数:	无
**返回参数:	无
**说    明：无
**************************************************/
void RTC_AlarmExtiNivc(void)
{
	EXTI_InitTypeDef E;		//中断线
	NVIC_InitTypeDef N;		//中断
	
	E.EXTI_Line = EXTI_Line17;									//中断线配置
	E.EXTI_Mode = EXTI_Mode_Interrupt;					//中断模式
	E.EXTI_Trigger = EXTI_Trigger_Rising;				//上升沿中断
	E.EXTI_LineCmd = ENABLE;										//存储设置到寄存器
	EXTI_Init(&E);
	
	N.NVIC_IRQChannel = RTC_Alarm_IRQn;					//报警中断
	N.NVIC_IRQChannelPreemptionPriority = 2;		//优先级
	N.NVIC_IRQChannelSubPriority = 0;						//响应级
	N.NVIC_IRQChannelCmd = ENABLE;							//开启中断监听
	NVIC_Init(&N);															//根据上面设置初始化中断控制器	
}

/**************************************************
**函数原型: void RTC_AlarmA(const u8 A_Day,const u8 A_Unit,const u8 A_Hours,const u8 A_Minutes,const u8 A_Seconds,const u8 A_Alarm_CE,void (*Alarm_fun)(void))
**功能:		RTC报警A配置
**输入参数:	const u8 A_Day				日期号或周号								// > 31屏蔽
						const u8 A_Unit				日期 or 周			0日期 1周
						const u8 A_Hours			小时												// > 23屏蔽
						const u8 A_Minutes		分钟												// > 59屏蔽
						const u8 A_Seconds		秒													// > 59屏蔽
						const u8 A_Alarm_CE		报警开关				0关闭报警	1开启报警
						void (*Alarm_fun)(void)					报警执行程序
**返回参数:	无
**说    明：
	RTC_AlarmMask_DateWeekDay		//屏蔽掉周和日		需要时分秒匹配引发报警					每天触发一次
	RTC_AlarmMask_None					//不作任何屏蔽		需要日期、时分秒匹配才引发报警	周当天触发一次 or 号数当天触发一次
	RTC_AlarmMask_Hours					//屏蔽掉小时			需要日期、分秒匹配才引发报警		周当天每小时触发一次 or 号数当天每小时触发一次
	RTC_AlarmMask_Minutes				//屏蔽掉分钟			需要日期、时秒匹配才引发报警		周当天小时当时每分钟触发一次 or 号数当天小时当时每分钟触发一次
	RTC_AlarmMask_Seconds				//屏蔽掉秒				需要日期、时分匹配才引发报警		周当天小时分钟当时每秒触发一次 or 号数当天小时分钟当时每秒触发一次
	RTC_AlarmMask_All						//屏蔽任何				所有日期所有时所有分所有秒均引发报警		每秒报警
**************************************************/
void RTC_AlarmA(const u8 A_Day,const u8 A_Unit,const u8 A_Hours,const u8 A_Minutes,const u8 A_Seconds,const u8 A_Alarm_CE,void (*Alarm_fun)(void))
{
	u8 i = 0;							//2个以上屏蔽项则为报警关闭
	
	RTC_AlarmTypeDef A;		//报警A
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);				//报警A关闭

	//是否关闭报警
	if(A_Alarm_CE == 0)			
	{
		RTC_ITConfig(RTC_IT_ALRA,DISABLE);			//报警A中断关闭
		RTC_AlarmCmd(RTC_Alarm_A,DISABLE);			//报警A关闭;
		return;
	}
	
	if(Alarm_fun != 0)
		A_Alarm_fun = Alarm_fun;								//执行函数指针指向执行函数
	
	//设置报警A
	A.RTC_AlarmDateWeekDay = A_Day;					//报警日子
	
	A.RTC_AlarmDateWeekDaySel = A_Unit == 0 ? RTC_AlarmDateWeekDaySel_Date : RTC_AlarmDateWeekDaySel_WeekDay;
	
	//有两个以上屏蔽项则为屏蔽所有设置
	if(A_Day > 31)			i++;
	if(A_Hours > 23)		i++;
	if(A_Minutes > 59)	i++;
	if(A_Seconds > 59)	i++;
	
	if(i > 1)
		A.RTC_AlarmMask = RTC_AlarmMask_All;			//屏蔽任何设置 				每秒引发报警
	else
	{
		A.RTC_AlarmMask = RTC_AlarmMask_None;			//设置不作任何屏蔽		一月或一周引发报警
		
		if(A_Day > 31)					//屏蔽掉周 or 日期
			A.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;						
		else if(A_Hours > 23)		//屏蔽掉小时
			A.RTC_AlarmMask = RTC_AlarmMask_Hours;									
		else if(A_Minutes > 59)	//屏蔽掉分
			A.RTC_AlarmMask = RTC_AlarmMask_Minutes;									
		else if(A_Seconds > 59)	//屏蔽掉秒
			A.RTC_AlarmMask = RTC_AlarmMask_Seconds;	
	}
	
	//A.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;								//下午	24小时制不需要配
	A.RTC_AlarmTime.RTC_Hours = A_Hours;									//时
	A.RTC_AlarmTime.RTC_Minutes = A_Minutes;							//分
	A.RTC_AlarmTime.RTC_Seconds = A_Seconds;							//秒
	RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_A,&A);					//设置报警
	
	//清除报警标志
	RTC_ClearFlag(RTC_FLAG_ALRAF);				//清除报警A标志
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断17线标志
	
	RTC_ITConfig(RTC_IT_ALRA,ENABLE);			//报警A中断开启
	RTC_AlarmCmd(RTC_Alarm_A,ENABLE);			//报警A开启
}

/**************************************************
**函数原型: void RTC_AlarmB(const u8 B_Day,const u8 B_Unit,const u8 B_Hours,const u8 B_Minutes,const u8 B_Seconds,const u8 B_Alarm_CE,void (*Alarm_fun)(void))
**功能:		RTC报警A配置
**输入参数:	const u8 B_Day				日期号或周号								// > 31屏蔽
						const u8 B_Unit				日期 or 周			0日期 1周
						const u8 B_Hours			小时												// > 23屏蔽
						const u8 B_Minutes		分钟												// > 59屏蔽
						const u8 B_Seconds		秒													// > 59屏蔽
						const u8 B_Alarm_CE			报警开关				0关闭报警	1开启报警
						void (*Alarm_fun)(void)											//报警函数
**返回参数:	无
**说    明：
	RTC_AlarmMask_DateWeekDay		//屏蔽掉周和日		需要时分秒匹配引发报警					每天触发一次
	RTC_AlarmMask_None					//不作任何屏蔽		需要日期、时分秒匹配才引发报警	周当天触发一次 or 号数当天触发一次
	RTC_AlarmMask_Hours					//屏蔽掉小时			需要日期、分秒匹配才引发报警		周当天每小时触发一次 or 号数当天每小时触发一次
	RTC_AlarmMask_Minutes				//屏蔽掉分钟			需要日期、时秒匹配才引发报警		周当天小时当时每分钟触发一次 or 号数当天小时当时每分钟触发一次
	RTC_AlarmMask_Seconds				//屏蔽掉秒				需要日期、时分匹配才引发报警		周当天小时分钟当时每秒触发一次 or 号数当天小时分钟当时每秒触发一次
	RTC_AlarmMask_All						//屏蔽任何				所有日期所有时所有分所有秒均引发报警		每秒报警
**************************************************/
void RTC_AlarmB(const u8 B_Day,const u8 B_Unit,const u8 B_Hours,const u8 B_Minutes,const u8 B_Seconds,const u8 B_Alarm_CE,void (*Alarm_fun)(void))
{
	u8 i = 0;							//2个以上屏蔽项则为报警关闭
	
	RTC_AlarmTypeDef B;		//报警B
	
	RTC_AlarmCmd(RTC_Alarm_B,DISABLE);			//报警B关闭

	//是否关闭报警
	if(B_Alarm_CE == 0)			
	{
		RTC_ITConfig(RTC_IT_ALRB,DISABLE);			//报警B中断关闭
		RTC_AlarmCmd(RTC_Alarm_B,DISABLE);			//报警B关闭;
		return;
	}
	
	if(Alarm_fun != 0)
		B_Alarm_fun = Alarm_fun;								//执行函数指针指向执行函数
	
	//设置报警B
	B.RTC_AlarmDateWeekDay = B_Day;						//报警日子
	
	B.RTC_AlarmDateWeekDaySel = B_Unit == 0 ? RTC_AlarmDateWeekDaySel_Date : RTC_AlarmDateWeekDaySel_WeekDay;
	
	//有两个以上屏蔽项则为屏蔽所有设置
	if(B_Day > 31)			i++;
	if(B_Hours > 23)		i++;
	if(B_Minutes > 59)	i++;
	if(B_Seconds > 59)	i++;
	
	if(i > 1)
		B.RTC_AlarmMask = RTC_AlarmMask_All;			//屏蔽任何设置 				每秒引发报警
	else
	{
		B.RTC_AlarmMask = RTC_AlarmMask_None;			//设置不作任何屏蔽		一月或一周引发报警
		
		if(B_Day > 31)					//屏蔽掉周 or 日期
			B.RTC_AlarmMask = RTC_AlarmMask_DateWeekDay;						
		else if(B_Hours > 23)		//屏蔽掉小时
			B.RTC_AlarmMask = RTC_AlarmMask_Hours;									
		else if(B_Minutes > 59)	//屏蔽掉分
			B.RTC_AlarmMask = RTC_AlarmMask_Minutes;									
		else if(B_Seconds > 59)	//屏蔽掉秒
			B.RTC_AlarmMask = RTC_AlarmMask_Seconds;	
	}
	
	//B.RTC_AlarmTime.RTC_H12 = RTC_H12_PM;								//下午	24小时制不需要配
	B.RTC_AlarmTime.RTC_Hours = B_Hours;									//时
	B.RTC_AlarmTime.RTC_Minutes = B_Minutes;							//分
	B.RTC_AlarmTime.RTC_Seconds = B_Seconds;							//秒
	RTC_SetAlarm(RTC_Format_BIN,RTC_Alarm_B,&B);					//设置报警
	
	//清除报警标志
	RTC_ClearFlag(RTC_FLAG_ALRAF);				//清除报警A标志
	EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断17线标志
	
	RTC_ITConfig(RTC_IT_ALRB,ENABLE);			//报警B中断开启
	RTC_AlarmCmd(RTC_Alarm_B,ENABLE);			//报警B开启
}

/**************************************************
**函数原型: void RTC_TamperExtiNivc(void)
**功能:		RTC干预中断配置
**输入参数:	无
**返回参数:	无
**说    明：暂未实现
**************************************************/
void RTC_TamperExtiNivc(void)
{
	EXTI_InitTypeDef E;		//中断线
	NVIC_InitTypeDef N;		//中断
	
	E.EXTI_Line = EXTI_Line21;									//中断线
	E.EXTI_Mode = EXTI_Mode_Interrupt;					//中断
	E.EXTI_Trigger = EXTI_Trigger_Rising;				//上升沿中断
	E.EXTI_LineCmd = ENABLE;										//存储设置到寄存器
	EXTI_Init(&E);
	
	N.NVIC_IRQChannel = TAMP_STAMP_IRQn;				//干预中断
	N.NVIC_IRQChannelPreemptionPriority = 2;		//优先级
	N.NVIC_IRQChannelSubPriority = 0;						//响应级
	N.NVIC_IRQChannelCmd = ENABLE;							//开启中断监听
	NVIC_Init(&N);															//根据上面设置初始化中断控制器	
}

/**************************************************
**函数原型: u8 RTC_Config(const u16 year,const u8 month,const u8 day,const u8 week,const u8 hour,const u8 min,const u8 sec)
**功能:		RTC配置
**输入参数:	const u16 year	年
						const u8 month	月
						const u8 day		日
						const u8 week		周
						const u8 hour		时
						const u8 min		分
						const u8 sec		秒
**返回参数:	u8 是否成功		1成功 0失败
**说    明：无
**************************************************/
u8 RTC_Config(const u16 year,const u8 month,const u8 day,const u8 week,const u8 hour,const u8 min,const u8 sec)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//打开PWR时钟	
  PWR_BackupAccessCmd(ENABLE);													//打开RTC后备寄存器通道

  if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0xA5A5)			//后备寄存器检查
	{  
    RTCInit();
		if(SetRTC_Init() == ERROR)													//初始化RTC
			return 0;
    if(SetRTC_Time(hour,min,sec) == ERROR)							//设置时间
			return 0; 			
    if(SetRTC_Date(year,month,day,week) == ERROR) 			//设置日期
			return 0;
		
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0xA5A5);				//写后备寄存器检查数据
  }
  else
  {
    if (RCC_GetFlagStatus(RCC_FLAG_PORRST) != RESET)			//上次为 电源关闭复位
			;//  DebugPf("POWER Is Close\r\n");
		
    else if (RCC_GetFlagStatus(RCC_FLAG_PINRST) != RESET)	//上次为 RESET复位
			;//  DebugPf("RESET System\r\n");

    RCC_ClearFlag();								//清除RCC的复位标志，以便下一次对RCC_FLAG_PORRST电源复位RCC_FLAG_PINRST引脚复位做判断
		
		RCC_LSEConfig(RCC_LSE_ON);					//启用LSE低速振荡器
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);	//等待始终稳定
		
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);							//选择时钟来源
		RCC_RTCCLKCmd(ENABLE);					//RTC开启	必须用于RCC_RTCCLKConfig之后
		
    RTC_WaitForSynchro();						//等待APB1高速时钟与RTC低速时钟同步，才能读写寄存器
  }
	
	RTC_AlarmExtiNivc();		//RTC报警中断配置
	//RTC_TamperExtiNivc();		//RTC干预中断配置		暂未实现
	
	return 1;																								//成功
}

/****************************************************************************
* 名	称：void GetTime(Time_Body* pTime)
* 功	能：获取时间
* 入口参数：Time_Body* pTime		时间
* 出口参数：无
* 说	明：24小时制无需判断上下午
****************************************************************************/
void GetTime(Time_Body* pTime)
{
	RTC_TimeTypeDef RTC_Time;			//时分秒
	RTC_DateTypeDef RTC_Date;			//年月日周
	
	RTC_GetTime(RTC_Format_BIN, &RTC_Time);		//获取时间		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
  RTC_GetDate(RTC_Format_BIN, &RTC_Date);		//获取日期		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
	
	pTime->HH = RTC_Time.RTC_Hours;					//小时
	pTime->MM = RTC_Time.RTC_Minutes;				//分
	pTime->SS = RTC_Time.RTC_Seconds;				//秒
	
// 	if(RTC_Time.RTC_H12 == RTC_H12_AM)			//上午还是下午	24小时制无需判断
// 		pTime->AMorPM = 0;
// 	else
// 		pTime->AMorPM = 1;	
	
	pTime->Year = RTC_Date.RTC_Year + 2000;	//年
	pTime->Month = RTC_Date.RTC_Month;			//月
	pTime->Day = RTC_Date.RTC_Date;					//日
	pTime->Week = RTC_Date.RTC_WeekDay;			//周	
}

/****************************************************************************
* 名	称：void ShowTime(void)
* 功	能：显示时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ShowTime(void)
{
	Time_Body thistime = {0,0,0,0,0,0,0};		//时间
	
	GetTime(&thistime);	//获取系统时间		
	printf("%d-%d-%d   %d:%d:%d   星期%d\r\n",thistime.Year,		//年
	thistime.Month,			//月
	thistime.Day,				//日
	thistime.HH,				//时
	thistime.MM,				//分
	thistime.SS,				//秒
	thistime.Week);			//周
}

/****************************************************************************
* 名	称：void RTC_Alarm_IRQHandler(void)
* 功	能：报警中断
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void RTC_Alarm_IRQHandler(void)
{
	//报警A激活
	if(RTC_GetITStatus(RTC_IT_ALRA) != RESET)		
	{
		if(A_Alarm_fun == 0)								//如果执行函数没有指向
			printf("RTX ALRA!!\r\n");					//打印提示信息
		else
			A_Alarm_fun();										//如果执行函数有指向  则执行执行函数
		
		RTC_ClearITPendingBit(RTC_IT_ALRA);		//清除报警标志
		EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线标志
	}
	
	//报警B激活
	if(RTC_GetITStatus(RTC_IT_ALRB) != RESET)		
	{
		if(B_Alarm_fun == 0)								//如果执行函数没有指向
			printf("RTX ALRB!!\r\n");					//打印提示信息
		else
			B_Alarm_fun();										//如果执行函数有指向  则执行执行函数
		
		RTC_ClearITPendingBit(RTC_IT_ALRB);		//清除报警标志
		EXTI_ClearITPendingBit(EXTI_Line17);	//清除中断线标志
	}
}
