#include "IO.h"
#include "dt3102_io.h"
#include "C2.h"
#include "sys_param.h"

static void DT3102_OutputInit(void)
{
	IoSetMode(OUT1, IO_MODE_OUTPUT);		//1
	IoSetSpeed(OUT1, IO_SPEED_50M);
	IoPushPull(OUT1); 	
	IoSetMode(OUT2, IO_MODE_OUTPUT);		//2
	IoSetSpeed(OUT2, IO_SPEED_50M);
	IoPushPull(OUT2); 	
	IoSetMode(OUT3, IO_MODE_OUTPUT);		//3
	IoSetSpeed(OUT3, IO_SPEED_50M);
	IoPushPull(OUT3); 	
	IoSetMode(OUT4, IO_MODE_OUTPUT);		//4
	IoSetSpeed(OUT4, IO_SPEED_50M);
	IoPushPull(OUT4); 	
}
static void DT3102_InputInit(void)
{	//板子上已经上拉，不用MCU内部上拉
	IoSetMode(IN1, IO_MODE_INPUT);			//1
	IoSetSpeed(IN1, IO_SPEED_50M);	
	IoSetMode(IN2, IO_MODE_INPUT);			//2
	IoSetSpeed(IN2, IO_SPEED_50M);	
	IoSetMode(IN3, IO_MODE_INPUT);			//3
	IoSetSpeed(IN3, IO_SPEED_50M);	
	IoSetMode(IN4, IO_MODE_INPUT);			//4
	IoSetSpeed(IN4, IO_SPEED_50M);	
	IoSetMode(IN5, IO_MODE_INPUT);			//5
	IoSetSpeed(IN5, IO_SPEED_50M);	
	IoSetMode(IN6, IO_MODE_INPUT);			//6
	IoSetSpeed(IN6, IO_SPEED_50M);	
	IoSetMode(IN7, IO_MODE_INPUT);			//7
	IoSetSpeed(IN7, IO_SPEED_50M);	
	IoSetMode(IN8, IO_MODE_INPUT);			//8
	IoSetSpeed(IN8, IO_SPEED_50M);	
	IoSetMode(IN9, IO_MODE_INPUT);			//9
	IoSetSpeed(IN9, IO_SPEED_50M);	
	IoSetMode(IN10, IO_MODE_INPUT);			//10
	IoSetSpeed(IN10, IO_SPEED_50M);	
}

//=====================================================================
//外设电平配置
/*
 #define	LIGHT2_NOEVENT 		1			//信号电平		无信号标识
 #define	LIGHT2_ERROR 		0			//故障电平		故障标识
 #define	LAND1_NOEVENT		1			//信号电平		无信号标识
 #define	LAND1_ERROR		0			//故障电平		故障标识
 #define	LAND2_NOEVENT		1			//信号电平		无信号标识
 #define	LAND2_ERROR		0			//故障电平		故障标识
 #define	AXIS_TYPE1_NOEVENT	0			//信号电平		无信号标识
 #define	AXIS_TYPE1_SINGLE	0			//单双轮电平	单轮标识
 #define	AXIS_TYPE1_ERROR	1			//故障电平		故障标识
*/

static DeviceSignal DT3102Signal = {0};

static void DT3102_DeviceSignal_DefaultInit(void)
{
	DT3102Signal.Signal.LC1EventSignal 	= 0;
	DT3102Signal.Signal.LC1ErrSignal		= 0;
	
	DT3102Signal.Signal.LC2EventSignal 	= 0;
	DT3102Signal.Signal.LC2ErrSignal		= 0;
	
	DT3102Signal.Signal.GC1EventSignal		= 0;
	DT3102Signal.Signal.GC1ErrSignal		= 0;
	
	DT3102Signal.Signal.GC2EventSignal		= 0;
	DT3102Signal.Signal.GC2ErrSignal		= 0;

	DT3102Signal.Signal.TyreEventSignal	= 1;
	DT3102Signal.Signal.TyreDoubleSignal	= 1;
	DT3102Signal.Signal.TyreErrSignal		= 1;
}

u16 Get_Device_Signal(void)
{
	return DT3102Signal.value;
}

BOOL Set_Device_Signal(u16 value)
{
	if(Param_Write(GET_U16_OFFSET(ParamDeviceSignal), &value, sizeof(unsigned short)))
	{
		DT3102Signal.value = value;
		return TRUE;
	}

	return FALSE;
}

void fDT3102_IO_Init(void)
{
	uint16 tmp = 0xffff;
	
	DT3102_OutputInit();
	DT3102_InputInit();

	Param_Read(GET_U16_OFFSET(ParamDeviceSignal), &tmp, sizeof(unsigned short));
	if((tmp & 0xf800) != 0)		//电平设置使用两个字节的低11位，高位初始化为0，若高位不等于0时则重新加载默认电平
	{
		//加载默认电平
		DT3102_DeviceSignal_DefaultInit();
	}
	else
	{
		DT3102Signal.value = tmp;
	}
}

//读地感1状态
sLand1_State_TypeDef fRead_Land1State(void)
{
	sLand1_State_TypeDef sTmp;
	if( DT3102Signal.Signal.GC1EventSignal == GetIo(LAND1_SIGN) )
		sTmp.Bit.EventFlag = 1;		//如果触发地感1
	else
		sTmp.Bit.EventFlag = 0;
	if( DT3102Signal.Signal.GC1ErrSignal == GetIo(LAND1_ERR) )
		sTmp.Bit.ErrFlag = 1;		//如果地感1故障
	else
		sTmp.Bit.ErrFlag = 0;
	
	return sTmp;
}
//读地感2状态
sLand2_State_TypeDef fRead_Land2State(void)
{
	sLand2_State_TypeDef sTmp;
	if( DT3102Signal.Signal.GC2EventSignal == GetIo(LAND2_SIGN) )
		sTmp.Bit.EventFlag = 1;		//如果触发地感2
	else
		sTmp.Bit.EventFlag = 0;
	if( DT3102Signal.Signal.GC2ErrSignal == GetIo(LAND2_ERR) )
		sTmp.Bit.ErrFlag = 1;		//如果地感2故障
	else
		sTmp.Bit.ErrFlag = 0;
	
	return sTmp;
}
//读光幕2状态
sLight_State_TypeDef fRead_Light2State(void)
{
	sLight_State_TypeDef sTmp;
	if( DT3102Signal.Signal.LC2EventSignal == GetIo(LIGHT2_SIGN) )
		sTmp.Bit.EventFlag = 1;		//如果触发光幕
	else
		sTmp.Bit.EventFlag = 0;
	if( DT3102Signal.Signal.LC2ErrSignal == GetIo(LIGHT2_ERR) )
		sTmp.Bit.ErrFlag = 1;		//如果光幕故障
	else 
		sTmp.Bit.ErrFlag = 0;
	
	return sTmp;
}
//读轴型识别器1状态
sAxisType1_State_TypeDef fRead_AxisType1State(void)
{
	sAxisType1_State_TypeDef sTmp;
	if( DT3102Signal.Signal.TyreEventSignal == GetIo(AXIS_TYPE1_SIGN) )
		sTmp.Bit.EventFlag = 1;		//如果触发轴型识别器
	else
		sTmp.Bit.EventFlag = 0;
	if( DT3102Signal.Signal.TyreDoubleSignal == GetIo(AXIS_TYPE1_JUDGE) )
		sTmp.Bit.SingleOrBoth = 1;	//如果是双轮
	else
		sTmp.Bit.SingleOrBoth = 0;
	if( DT3102Signal.Signal.TyreErrSignal == GetIo(AXIS_TYPE1_ERR) )
		sTmp.Bit.ErrFlag = 1;		//如果轴型识别器故障
	else
		sTmp.Bit.ErrFlag = 0;
	
	return sTmp;
}

static void fRoadGate_Close(void)
{
	IoHi(ROAD_GATE1);
	IoLo(ROAD_GATE2);
	SysTimeDly(ROAD_GATE_DLY_TIME);
	IoHi(ROAD_GATE2);
}
static void fRoadGate_Open(void)
{
	IoHi(ROAD_GATE2);
	IoLo(ROAD_GATE1);
	SysTimeDly(ROAD_GATE_DLY_TIME);
	IoHi(ROAD_GATE1);
}
/***********************************************************************
 * 外设控制函数
 * 函数名称：fPeripheral_Control
 * 函数功能：控制外设，返回控制结果
 * 参数：	  STATUS status:控制值(0、红绿灯亮绿灯，道闸开启;1、红绿灯亮红灯，道闸关闭) 
 *  		  uint8 type：1、控制红绿灯;2、控制道闸
 * 返回值：  s8 ：控制是否成功(1、成功；-1、失败)
 * 备注:
   1.1电路板  绿灯 高电平点亮 红灯 低电平点亮
   1.0电路板  绿灯 低电平点亮 红灯 高电平点亮
 ***********************************************************************/
s8 fPeripheral_Control(u8 type , u8 status)
{
	if( TrafficSignal == type )				//控制红绿灯
	{
		if( 0 == status )		//绿灯亮
			IoHi(TRAFFIC_LIGHT);
		else					//红灯亮
			IoLo(TRAFFIC_LIGHT);
		return 1;
	}
	else if( BarrierGate == type )		//控制道闸
	{
		if( 0 == status )		//开启道闸
			fRoadGate_Open();
		else if ( 1 == status )	//关闭道闸
			fRoadGate_Close();
		return 1;
	}
	return 0;
}

//ROAD_GATE2_PIN 道闸关
//ROAD_GATE1_PIN 道闸开
void BarrierGate_JDQ_Control(u8 port, u8 status)
{
	if(port == BarrierGateOpenPort)
	{
		if(status == PortNormalOpen) 
			IoHi(ROAD_GATE1);
		else if(status == PortNormalClose) 
			IoLo(ROAD_GATE1);
	}
	else if(port == BarrierGateClosePort)
	{
		if(status == PortNormalOpen) 
			IoHi(ROAD_GATE2);
		else if(status == PortNormalClose) 
			IoLo(ROAD_GATE2);
	}
}


