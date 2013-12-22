#include "rtc_init.h"
#include "printf_init.h"		//DebugPf

//未实现WakeUP唤醒功能、Tamper功能、Time stamp event功能

typedef void(*Alarm_fun)(void); //函数指针
static Alarm_fun A_Alarm_fun = 0;			//警报A响应执行函数指针			必须static
static Alarm_fun B_Alarm_fun = 0;			//警报B响应执行函数指针			必须static

static u8 RTC_pDisc[] = "LuoHuaiXiang_Rtc\r\n";				//设备描述字符		必须static

/****************************************************************************
* 名	称：void RTC_Config(void)
* 功	能：配置RTC时钟
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void RTC_Config(void)
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
* 名	称：ErrorStatus SetRTC_Time(u8 hour,u8 min,u8 sec)
* 功	能：设置时间
* 入口参数：u8 hour		时
						u8 min		分
						u8 sec		秒
* 出口参数：ErrorStatus		ERROR or SUCCESS
* 说	明：因采用24小时制故无需配RTC_H12参数
					SetRTC_Time(23,59,58);		//23:59:58
****************************************************************************/
ErrorStatus SetRTC_Time(u8 hour,u8 min,u8 sec)
{
	ErrorStatus err = ERROR;
	RTC_TimeTypeDef RTC_TimeTemp;			//时分秒
	
	SysIntEnter();
  RTC_TimeTemp.RTC_Hours = hour;					//时
  RTC_TimeTemp.RTC_Minutes = min;					//分
  RTC_TimeTemp.RTC_Seconds = sec;					//秒
  err = RTC_SetTime(RTC_Format_BIN, &RTC_TimeTemp);		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
	SysIntExit();
	
  return err;
}

/****************************************************************************
* 名	称：ErrorStatus SetRTC_Date(u16 year,u8 month,u8 day,u8 week)
* 功	能：设置日期
* 入口参数：u16 year	年
						u8 month	月
						u8 day		日
						u8 week		周
* 出口参数：ErrorStatus		ERROR or SUCCESS
* 说	明：SetRTC_Date(2012,10,4,4);			//2012-10-4 周四
****************************************************************************/
ErrorStatus SetRTC_Date(u16 year,u8 month,u8 day,u8 week)
{
	RTC_DateTypeDef RTC_DateTemp;			//年月日周
	u8 YearChar = 0;
	ErrorStatus err = ERROR;
	
	SysIntEnter();
	if(year >= 2000)	
		year -= 2000;
	YearChar = (u8)year;								//16位年转换为8位年
	
	RTC_DateTemp.RTC_Year  = YearChar;	//年
	RTC_DateTemp.RTC_Month = month;			//月
	RTC_DateTemp.RTC_Date  = day;				//日
	RTC_DateTemp.RTC_WeekDay  = week;		//周
	
	err = RTC_SetDate(RTC_Format_BIN, &RTC_DateTemp);		//BIN 设置的时间为十进制	BCD 设置的时间为十六进制
	SysIntExit();
	return err;
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
**函数原型: void RTC_AlarmA(u8 A_Day,u8 A_Unit,u8 A_Hours,u8 A_Minutes,u8 A_Seconds,u8 A_Alarm_CE)
**功能:		RTC报警A配置
**输入参数:	u8 A_Day				日期号或周号								// > 31屏蔽
						u8 A_Unit				日期 or 周			0日期 1周
						u8 A_Hours			小时												// > 23屏蔽
						u8 A_Minutes		分钟												// > 59屏蔽
						u8 A_Seconds		秒													// > 59屏蔽
						u8 A_Alarm_CE		报警开关				0关闭报警	1开启报警
**返回参数:	无
**说    明：
	RTC_AlarmMask_DateWeekDay		//屏蔽掉周和日		需要时分秒匹配引发报警					每天触发一次
	RTC_AlarmMask_None					//不作任何屏蔽		需要日期、时分秒匹配才引发报警	周当天触发一次 or 号数当天触发一次
	RTC_AlarmMask_Hours					//屏蔽掉小时			需要日期、分秒匹配才引发报警		周当天每小时触发一次 or 号数当天每小时触发一次
	RTC_AlarmMask_Minutes				//屏蔽掉分钟			需要日期、时秒匹配才引发报警		周当天小时当时每分钟触发一次 or 号数当天小时当时每分钟触发一次
	RTC_AlarmMask_Seconds				//屏蔽掉秒				需要日期、时分匹配才引发报警		周当天小时分钟当时每秒触发一次 or 号数当天小时分钟当时每秒触发一次
	RTC_AlarmMask_All						//屏蔽任何				所有日期所有时所有分所有秒均引发报警		每秒报警
**************************************************/
void RTC_AlarmA(u8 A_Day,u8 A_Unit,u8 A_Hours,u8 A_Minutes,u8 A_Seconds,u8 A_Alarm_CE)
{
	u8 i = 0;							//2个以上屏蔽项则为报警关闭
	
	RTC_AlarmTypeDef A;		//报警A
	
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);			//报警A关闭

	//是否关闭报警
	if(A_Alarm_CE == 0)			
	{
		RTC_ITConfig(RTC_IT_ALRA,DISABLE);			//报警A中断关闭
		RTC_AlarmCmd(RTC_Alarm_A,DISABLE);			//报警A关闭;
		return;
	}
	
	//设置报警A
	A.RTC_AlarmDateWeekDay = A_Day;					//报警日子
	if(A_Unit == 0)													//日子为日期还是星期
		A.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;				//选择日期
	else
		A.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay;		//选择周
	
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
**函数原型: void RTC_AlarmB(u8 B_Day,u8 B_Unit,u8 B_Hours,u8 B_Minutes,u8 B_Seconds,u8 B_Alarm_CE)
**功能:		RTC报警A配置
**输入参数:	u8 B_Day				日期号或周号								// > 31屏蔽
						u8 B_Unit				日期 or 周			0日期 1周
						u8 B_Hours			小时												// > 23屏蔽
						u8 B_Minutes		分钟												// > 59屏蔽
						u8 B_Seconds		秒													// > 59屏蔽
						u8 B_Alarm_CE			报警开关				0关闭报警	1开启报警
**返回参数:	无
**说    明：
	RTC_AlarmMask_DateWeekDay		//屏蔽掉周和日		需要时分秒匹配引发报警					每天触发一次
	RTC_AlarmMask_None					//不作任何屏蔽		需要日期、时分秒匹配才引发报警	周当天触发一次 or 号数当天触发一次
	RTC_AlarmMask_Hours					//屏蔽掉小时			需要日期、分秒匹配才引发报警		周当天每小时触发一次 or 号数当天每小时触发一次
	RTC_AlarmMask_Minutes				//屏蔽掉分钟			需要日期、时秒匹配才引发报警		周当天小时当时每分钟触发一次 or 号数当天小时当时每分钟触发一次
	RTC_AlarmMask_Seconds				//屏蔽掉秒				需要日期、时分匹配才引发报警		周当天小时分钟当时每秒触发一次 or 号数当天小时分钟当时每秒触发一次
	RTC_AlarmMask_All						//屏蔽任何				所有日期所有时所有分所有秒均引发报警		每秒报警
**************************************************/
void RTC_AlarmB(u8 B_Day,u8 B_Unit,u8 B_Hours,u8 B_Minutes,u8 B_Seconds,u8 B_Alarm_CE)
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
	
	//设置报警B
	B.RTC_AlarmDateWeekDay = B_Day;					//报警日子
	if(B_Unit == 0)													//日子为日期还是星期
		B.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_Date;				//选择日期
	else
		B.RTC_AlarmDateWeekDaySel = RTC_AlarmDateWeekDaySel_WeekDay;		//选择周
	
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
	
	SysIntEnter();
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
	SysIntExit();
}

/****************************************************************************
* 名	称：void ShowTime(void)
* 功	能：显示时间
* 入口参数：无
* 出口参数：无
* 说	明：周日为7
****************************************************************************/
void ShowTime(void)
{
	Time_Body thistime = {0,0,0,0,0,0,0};		//时间
	
	SysIntEnter();
	GetTime(&thistime);	//获取系统时间		
	printf("%d-%d-%d   %d:%d:%d   星期%d\r\n",thistime.Year,		//年
	thistime.Month,			//月
	thistime.Day,				//日
	thistime.HH,				//时
	thistime.MM,				//分
	thistime.SS,				//秒
	thistime.Week);			//周
	SysIntExit();
}

/****************************************************************************
* 名	称：void Alarm_A_Caller(void(*Alarm_fun)(void))
* 功	能：报警A响应回调函数
* 入口参数：回应函数
* 出口参数：无
* 说	明：将执行函数指针指向执行函数
****************************************************************************/
void Alarm_A_Caller(void(*Alarm_fun)(void))
{
	A_Alarm_fun = Alarm_fun;	//执行函数指针指向执行函数
}

/****************************************************************************
* 名	称：void Alarm_B_Caller(void(*Alarm_fun)(void))
* 功	能：报警B响应回调函数
* 入口参数：回应函数
* 出口参数：无
* 说	明：将执行函数指针指向执行函数
****************************************************************************/
void Alarm_B_Caller(void(*Alarm_fun)(void))
{
	B_Alarm_fun = Alarm_fun;	//执行函数指针指向执行函数
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
	SysIntEnter();
	
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
	
	SysIntExit();
}

/****************************************************************************
* 名	称：static s8 RTC_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 RTC_Open(u32 lParam)
{
	return 0;					
}

/****************************************************************************
* 名	称：static s8 RTC_Close(void)
* 功	能：统一接口
* 入口参数：无
* 出口参数：s8						关闭操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 RTC_Close(void)
{
	return 0;		
}

/****************************************************************************
* 名	称：static s32 RTC_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：设备读
* 入口参数：u8* buffer			时间值		Time_Body结构体
						u32 len				希望接收的长度
						u32* Reallen	实际接收的长度
						u32 lParam		参数
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
	u8 AMorPM;		//上午or下午	0上午	1下午			//24小时制不使用
} Time_Body;
****************************************************************************/
static s32 RTC_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam)
{
	SysSchedLock();															//锁调度
	GetTime((Time_Body*)buffer);	//获取时间
	SysSchedUnlock();														//解锁调度	
	return 1;
}

/****************************************************************************
* 名	称：static s32 RTC_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：串口读
* 入口参数：u8* buffer			时间值		Time_Body结构体
						u32 len				希望写入的长度
						u32* Reallen	实际写入的长度
						u32 lParam		参数
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：
typedef struct
{
	u16 Year;	   //年
	u8 Month;	   //月
	u8 Day;		   //日
	u8 HH;		   //时
	u8 MM;		   //分
	u8 SS;		   //秒
	u8 Week;	   //周
	u8 AMorPM;		//上午or下午	0上午	1下午			//24小时制不使用
} Time_Body;
****************************************************************************/
static s32 RTC_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
{
	SysSchedLock();															//锁调度
	
	if(SetRTC_Time(((Time_Body*)buffer)->HH,		//小时
		((Time_Body*)buffer)->MM,									//分钟
		((Time_Body*)buffer)->SS) == ERROR)				//秒
	{
		SysSchedUnlock();													//解锁调度
		return 0;
	}
	
	if(SetRTC_Date(((Time_Body*)buffer)->Year,	//年
		((Time_Body*)buffer)->Month,							//月
		((Time_Body*)buffer)->Day,								//日
		((Time_Body*)buffer)->Week) == ERROR)			//周
	{
		SysSchedUnlock();													//解锁调度
		return 0;
	}	
	
	SysSchedUnlock();														//解锁调度
	return 1;	
}

/****************************************************************************
* 名	称：static s32 RTC_Ioctl(u32 cmd,u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 RTC_Ioctl(u32 cmd,u32 lParam)
{
	switch(cmd)
	{
		case (u32)RTCCMD_SetTime:											//设置时间
		{
			SysSchedLock();															//锁调度
			if(SetRTC_Time(((Time_Body*)lParam)->HH,		//小时
				((Time_Body*)lParam)->MM,									//分钟
				((Time_Body*)lParam)->SS) == ERROR)				//秒
			{
				SysSchedUnlock();														//解锁调度
				return 0;
			}
			
			SysSchedUnlock();														//解锁调度
			return 1;
		}
		case (u32)RTCCMD_SetDate:											//设置日期
		{
			SysSchedLock();															//锁调度
			if(SetRTC_Date(((Time_Body*)lParam)->Year,	//年
				((Time_Body*)lParam)->Month,							//月
				((Time_Body*)lParam)->Day,								//日
				((Time_Body*)lParam)->Week) == ERROR)			//周
			{
				SysSchedUnlock();													//解锁调度
				return 0;
			}
			
			SysSchedUnlock();														//解锁调度
			return 1;
		}
		case (u32)RTCCMD_LookTimeDate:								//查询时间日期
		{
			SysSchedLock();															//锁调度
			GetTime((Time_Body*)lParam);	
			SysSchedUnlock();														//解锁调度
			return 1;
		}
		case (u32)RTCCMD_ShowTimeDate:								//显示日期	串口打印显示
		{
			SysSchedLock();															//锁调度
			ShowTime();																	//获取并显示时间
			SysSchedUnlock();														//解锁调度
			return 1;
		}
		case (u32)RTCCMD_SetAlarm_A:							//设置闹钟A
		{
			SysSchedLock();																		//锁调度
			RTC_AlarmA(((ALARM_Cfg*)lParam)->ALARM_Cfg_Day,		//周or日
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Unit,							//单位 0日 1周
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Hours,						//小时
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Minutes,					//分钟
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Seconds,					//秒
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Alarm_CE);				//开关 1开0关
			SysSchedUnlock();																	//解锁调度
			return 1;
		}
		case (u32)RTCCMD_SetAlarm_B:							//设置闹钟B
		{
			SysSchedLock();																		//锁调度
			RTC_AlarmB(((ALARM_Cfg*)lParam)->ALARM_Cfg_Day,		//周or日
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Unit,							//单位 0日 1周
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Hours,						//小时
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Minutes,					//分钟
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Seconds,					//秒
			((ALARM_Cfg*)lParam)->ALARM_Cfg_Alarm_CE);				//开关 1开0关
			SysSchedUnlock();																	//解锁调度
			return 1;
		}
		case (u32)RTCCMD_SetAlarm_A_FUN:					//设置闹钟A执行函数
		{
			Alarm_A_Caller((Alarm_fun)lParam);			//执行函数
			return 1;
		}
		case (u32)RTCCMD_SetAlarm_B_FUN:					//设置闹钟B执行函数
		{
			Alarm_B_Caller((Alarm_fun)lParam);			//执行函数
			return 1;
		}
	}
	return 0;		//命令错误
}

/**************************************************
**函数原型: u8 RTC_init(void)
**功能:		RTC初始化
**输入参数:	无
**返回参数:	u8	初始化是否成功	1成功 0失败
**说    明：无
**************************************************/
u8 RTCInit(void)
{
	//注册用变量
	DEV_REG rtc = 			//设备注册信息表						不必static		
	{
		CHAR_UNDEF1,						//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		RTC_pDisc,					//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)RTC_Open,			//设备打开函数指针
		(u32*)RTC_Close, 		//设备关闭函数指针
		(u32*)RTC_Read,			//字符读函数
		(u32*)RTC_Write,		//字符写函数
		(u32*)RTC_Ioctl			//控制函数
	};

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);		//打开PWR时钟	
  PWR_BackupAccessCmd(ENABLE);													//打开RTC后备寄存器通道

  if(RTC_ReadBackupRegister(RTC_BKP_DR0) != 0x1004)			//后备寄存器检查
	{  
    RTC_Config();
		if(SetRTC_Init() == ERROR)							//初始化RTC
			return 0;
    if(SetRTC_Time(15,45,30) == ERROR)			//设置时间
			return 0; 			
    if(SetRTC_Date(2012,10,4,4) == ERROR) 	//设置日期
			return 0;
		
		RTC_WriteBackupRegister(RTC_BKP_DR0, 0x1004);				//写后备寄存器检查数据
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
	
	if(DeviceInstall(&rtc) != HVL_NO_ERR)			//设备注册
		return 0;
	
	return 1;
}
