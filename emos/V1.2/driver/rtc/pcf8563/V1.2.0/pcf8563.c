#include "pcf8563.h"
#include "I2Cx.h"
#include "driver.h"
#include "system_init.h"
#include "stdio.h"

#define ColReg1		0x00					//控制状态寄存器1
#define ColReg2		0x01					//控制状态寄存器2
#define SSReg			0x02					//秒寄存器
#define MMReg			0x03					//分寄存器
#define HHReg			0x04					//小时寄存器
#define DayReg		0x05					//日期寄存器
#define WeekReg		0x06					//星期寄存器
#define MCReg			0x07					//月和世纪寄存器
#define YearReg		0x08					//年寄存器

#define ValidL7		0x7F					//低7位有效
#define ValidL6		0x3F					//低6位有效
#define ValidL3		0x07					//低3位有效
#define ValidL5		0x1F					//低5位有效

static u8 OutRtc_pDisc[] = "LuoHuaiXiang_OutRtc\r\n";				//设备描述字符					必须static

typedef struct
{
	u8				DeviceAddr;						//器件地址
	I2C_Cfg		I2Cx;									//I2C配置
}OutRtc_Cfg;											//OutRTC设置

static OutRtc_Cfg OutRtcReal = 			//外部RTC设备
{
	0xA2,				//外部RTC地址
	{
		0,
		GPIO_Pin_6,		//SCK
		GPIOB,
		GPIO_Pin_7,		//SDA
		GPIOB
	}
};

/****************************************************************************
* 名	称：u8 OutRTC_Set(OutTime_Body* OutTime_t)
* 功	能：外部RTC初始化
* 入口参数：OutTime_Body* OutTime_t			时间结构体
* 出口参数：u8													是否成功		1成功	0失败
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
} OutTime_Body;
****************************************************************************/
u8 OutRTC_Set(OutTime_Body* OutTime_t)
{
	u8 Temp = 0;
	
	Temp = BINToBCD(OutTime_t->SS);			//设置秒
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,SSReg,&Temp,1) != 1) 
		return 0;			
	Temp = BINToBCD(OutTime_t->MM);			//设置分
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,MMReg,&Temp,1) != 1) 
		return 0; 	
	Temp = BINToBCD(OutTime_t->HH);			//设置小时
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,HHReg,&Temp,1) != 1) 
		return 0;
	Temp = BINToBCD(OutTime_t->Day);		//设置日
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,DayReg,&Temp,1) != 1)	
		return 0;
	Temp = BINToBCD(OutTime_t->Week);		//设置周
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,WeekReg,&Temp,1) != 1) 
		return 0;
	if((OutTime_t->Year) < 2000)
		Temp = BINToBCD(1);								//设置世纪	19xx
	else
		Temp = BINToBCD(0);								//设置世纪	20xx
	Temp = Temp << 7;
	Temp += BINToBCD(OutTime_t->Month);	//设置月
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,MCReg,&Temp,1) != 1) 
		return 0; 
	Temp = BINToBCD(OutTime_t->Year % 100);				//设置年
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,YearReg,&Temp,1) != 1) 
		return 0; 
	
	return 1;
}

/****************************************************************************
* 名	称：u8 OutRTC_Get(OutTime_Body* OutTime_t)
* 功	能：外部RTC初始化
* 入口参数：OutTime_Body* OutTime_t			时间结构体
* 出口参数：u8													是否成功		1成功	0失败
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
} OutTime_Body;
****************************************************************************/
u8 OutRTC_Get(OutTime_Body* OutTime_t)
{
	u8 Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,SSReg,&Temp,1) != 1)	
		return 0;	
	OutTime_t->SS = BCDToBIN(Temp& ValidL7);			//获得秒
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,MMReg,&Temp,1) != 1)	
		return 0;	
	OutTime_t->MM = BCDToBIN(Temp & ValidL7);			//获得分
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,HHReg,&Temp,1) != 1)	
		return 0;	
	OutTime_t->HH = BCDToBIN(Temp & ValidL6);			//获得小时
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,DayReg,&Temp,1) != 1)	
		return 0;	
	OutTime_t->Day = BCDToBIN(Temp & ValidL6);		//获得日
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,WeekReg,&Temp,1) != 1)	
		return 0;	
	OutTime_t->Week = BCDToBIN(Temp & ValidL3);		//获得周		周7 = 0
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,MCReg,&Temp,1) != 1)	
		return 0;
	OutTime_t->Month = BCDToBIN(Temp & ValidL5);		//获得月
	
	if((Temp >> 7) > 0)									//获得世纪
		OutTime_t->Year = 1900;		
	else 
		OutTime_t->Year = 2000;
	Temp = 0;
	
	if(I2CReceiveByteADDRData(OutRtcReal.DeviceAddr,YearReg,&Temp,1) != 1)	
		return 0;													//获得年
	OutTime_t->Year += BCDToBIN(Temp);	//年赋值
	
	return 1;														//获取成功
}


/****************************************************************************
* 名	称：u8 OutRTC_Show(void)
* 功	能：显示时间
* 入口参数：无
* 出口参数：u8 获取显示时间是否成功		0失败	1成功
* 说	明：获取并显示 调试用
****************************************************************************/
u8 OutRTC_Show(void)
{
	OutTime_Body TimeTemp = {0};			//获取用时间
	
	if(OutRTC_Get(&TimeTemp) == 0)		//获取时间
		return 0;		//失败
	
	printf("%d-%d-%d %d:%d:%d %d\r\n",TimeTemp.Year,		//年
																	TimeTemp.Month,			//月
																	TimeTemp.Day,				//日
																	TimeTemp.HH,				//时
																	TimeTemp.MM,				//分
																	TimeTemp.SS,				//秒
																	TimeTemp.Week);			//周 0-6
	
	return 1;			//成功
}

/****************************************************************************
* 名	称：static s8 OUTRTC_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 OUTRTC_Open(u32 lParam)
{
	return 0;	
}

/****************************************************************************
* 名	称：static s8 OUTRTC_Close(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 OUTRTC_Close(u32 lParam)
{
	return 0;
}

/****************************************************************************
* 名	称：static s32 OUTRTC_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 OUTRTC_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
{
	s32 i = 0;
	
	SysSchedLock();											//锁调度
	i = OutRTC_Get((OutTime_Body*)lParam);		
	SysSchedUnlock();										//解锁调度	
	
	return i;
}

/****************************************************************************
* 名	称：static s32 OUTRTC_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：立即发音
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：
****************************************************************************/
static s32 OUTRTC_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
{
	s32 i = 0;
	
	SysSchedLock();											//锁调度
	i = OutRTC_Set((OutTime_Body*)lParam);		
	SysSchedUnlock();										//解锁调度	
	
	return i;
}

/****************************************************************************
* 名	称：static s32 OUTRTC_Ioctl(u32 cmd, u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 OUTRTC_Ioctl(u32 cmd, u32 lParam)
{
	s32 i = 0;
	
	switch(cmd)
	{
		case (u32)OUTRTCCMD_SetTime:									//设置时间
		{
			SysSchedLock();															//锁调度
			i = OutRTC_Set((OutTime_Body*)lParam);		
			SysSchedUnlock();														//解锁调度	
			return i;
		}
		case (u32)OUTRTCCMD_GetTime:									//获取时间
		{
			SysSchedLock();															//锁调度
			i = OutRTC_Get((OutTime_Body*)lParam);		
			SysSchedUnlock();														//解锁调度	
			return i;
		}
		case (u32)OUTRTCCMD_ShowTime:									//显示时间
		{
			SysSchedLock();															//锁调度
			OutRTC_Show();		
			SysSchedUnlock();														//解锁调度	
			return 1;
		}
	}
	return 0;	//错误命令	
}
/****************************************************************************
* 名	称：u8 OutRTC_init(OutTime_Body* Time_t)
* 功	能：外部RTC初始化
* 入口参数：OutTime_Body* Time_t			设置的时间	0表示不需要时间设置
* 出口参数：u8												是否成功		1成功	0失败
* 说	明：无
****************************************************************************/
u8 OutRTC_init(OutTime_Body* Time_t)
{
	u8 Val = 0;
	
	//注册用变量
	DEV_REG outrtc = 			//设备注册信息表						不必static		
	{
		CHAR_RTC,						//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		OutRtc_pDisc,				//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)OUTRTC_Open,		//设备打开函数指针
		(u32*)OUTRTC_Close, 	//设备关闭函数指针
		(u32*)OUTRTC_Read,		//字符读函数
		(u32*)OUTRTC_Write,		//字符写函数
		(u32*)OUTRTC_Ioctl		//控制函数
	};	
	
	if(I2Cx_Init() == 0)		//I2C初始化
		return 0;
	
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,ColReg1,&Val,1) != 1)	//控制状态寄存器1		普通模式、时钟运行、电源复位失效
		return 0;	
	
	if(I2CSendByteADDRData(&OutRtcReal.I2Cx,OutRtcReal.DeviceAddr,ColReg2,&Val,1) != 1) //控制状态寄存器2		报警无效、定时无效
		return 0;	
	
	if(DeviceInstall(&outrtc) != HVL_NO_ERR)		//设备注册
		return 0;
	
	if(Time_t != (OutTime_Body*)0)							//需要进行时间设置
	{	
		if(OutRTC_Set(Time_t) == 0) 							//设置时间
			return 0;					
	}
	
	return 1;
}


