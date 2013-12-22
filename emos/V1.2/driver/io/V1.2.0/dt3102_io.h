#ifndef		_DT3102_IO_H_
#define		_DT3102_IO_H_

#include "includes.h"
#include "IO.h"
/*
		IN1				地感1事件状态			PF0
		IN2				地感1故障状态			PF1
		IN3				光幕2事件状态			PF2
		IN4				光幕2故障状态			PF3
		IN5				地感2事件状态			PF4
		IN6				地感2故障状态			PF5
		
		IN8				轴型识别器1单双轮引脚	PF7
		IN9				轴型识别器1事件状态		PF8
		IN10			轴型识别器1故障状态		PF9
		
		OUT1			道闸通道1				PE0
		OUT2			道闸通道2				PE1
		OUT3			红绿灯					PE2
*/
//IO-PORT
#define	IN1_PORT	GPIOF
#define	IN2_PORT	GPIOF
#define	IN3_PORT	GPIOF
#define	IN4_PORT	GPIOF
#define	IN5_PORT	GPIOF
#define	IN6_PORT	GPIOF
#define	IN7_PORT	GPIOF
#define	IN8_PORT	GPIOF
#define	IN9_PORT	GPIOF
#define	IN10_PORT	GPIOF
#define	OUT1_PORT	GPIOE
#define	OUT2_PORT	GPIOE
#define	OUT3_PORT	GPIOE
#define	OUT4_PORT	GPIOE
//IO-PIN
#define	IN1_PIN		GPIO_Pin_0
#define	IN2_PIN		GPIO_Pin_1
#define	IN3_PIN		GPIO_Pin_2
#define	IN4_PIN		GPIO_Pin_3
#define	IN5_PIN		GPIO_Pin_4
#define	IN6_PIN		GPIO_Pin_5
#define	IN7_PIN		GPIO_Pin_6
#define	IN8_PIN		GPIO_Pin_7
#define	IN9_PIN		GPIO_Pin_8
#define	IN10_PIN	GPIO_Pin_9
#define	OUT1_PIN	GPIO_Pin_0
#define	OUT2_PIN	GPIO_Pin_1
#define	OUT3_PIN	GPIO_Pin_2
#define	OUT4_PIN	GPIO_Pin_3
//光幕2
#define	LIGHT2_SIGN_PORT	IN3_PORT
#define	LIGHT2_SIGN_PIN	IN3_PIN
#define	LIGHT2_ERR_PORT	IN4_PORT
#define	LIGHT2_ERR_PIN		IN4_PIN
//地感1
#define	LAND1_SIGN_PORT	IN1_PORT
#define	LAND1_SIGN_PIN		IN1_PIN
#define	LAND1_ERR_PORT		IN2_PORT
#define	LAND1_ERR_PIN		IN2_PIN
//地感2
#define	LAND2_SIGN_PORT	IN5_PORT
#define	LAND2_SIGN_PIN		IN5_PIN
#define	LAND2_ERR_PORT		IN6_PORT
#define	LAND2_ERR_PIN		IN6_PIN
//轴型识别器1
#define	AXIS_TYPE1_JUDGE_PORT	IN8_PORT	
#define	AXIS_TYPE1_JUDGE_PIN	IN8_PIN
#define	AXIS_TYPE1_SIGN_PORT	IN9_PORT
#define	AXIS_TYPE1_SIGN_PIN		IN9_PIN
#define	AXIS_TYPE1_ERR_PORT		IN10_PORT
#define	AXIS_TYPE1_ERR_PIN		IN10_PIN
//道闸、红绿灯
#define	ROAD_GATE_DLY_TIME		60			//道闸控制延时时间
#define	ROAD_GATE1_PORT			OUT1_PORT
#define	ROAD_GATE1_PIN			OUT1_PIN
#define	ROAD_GATE2_PORT			OUT2_PORT
#define	ROAD_GATE2_PIN			OUT2_PIN
#define	TRAFFIC_LIGHT_PORT		OUT3_PORT
#define	TRAFFIC_LIGHT_PIN		OUT3_PIN

//设备电平结构
typedef union _DeviceSignal
{
	struct
	{
		u8	LC1EventSignal 	:	1;	//光幕1信号电平
		u8	LC1ErrSignal		:	1;	//光幕1故障电平
		u8	LC2EventSignal 	:	1;	//光幕2信号电平
		u8	LC2ErrSignal		:	1;	//光幕2故障电平
		u8	GC1EventSignal 	:	1;	//地感1信号电平
		u8	GC1ErrSignal 		:	1;	//地感1故障电平
		u8	GC2EventSignal 	:	1;	//地感2信号电平
		u8	GC2ErrSignal 		:	1;	//地感2故障电平
		u8	TyreEventSignal 	:	1;	//轮轴信号电平
		u8	TyreErrSignal 		:	1;	//轮轴故障电平
		u8	TyreDoubleSignal	:	1;	//双轮电平
	}Signal;
	u16 value;
}DeviceSignal;

u16 Get_Device_Signal(void);
BOOL Set_Device_Signal(u16 value);


//光幕状态结构体
typedef	union sLight_State_TypeDef
{
	u8	u8Data;
	struct
	{
		u8	EventFlag : 1;		//事件位	0-无事件	1-光幕触发
		u8	ErrFlag : 	1;		//故障位	0-光幕正常	1-光幕故障
	}Bit;
}sLight_State_TypeDef;			
//地感状态结构体
typedef	union sLand1_State_TypeDef
{
	u8	u8Data;
	struct
	{
		u8	EventFlag : 1;		//事件位	0-无事件	1-地感触发
		u8	ErrFlag : 	1;		//故障位	0-地感正常	1-地感故障
	}Bit;
}sLand1_State_TypeDef;
typedef	union sLand2_State_TypeDef
{
	u8	u8Data;
	struct
	{
		u8	EventFlag : 1;		//事件位	0-无事件	1-地感触发
		u8	ErrFlag : 	1;		//故障位	0-地感正常	1-地感故障
	}Bit;
}sLand2_State_TypeDef;
//轴型识别器状态结构体
typedef union sAxisType1_State_TypeDef
{
	u8 u8Data;
	struct
	{
		u8	EventFlag :	1;	//事件位			0-无事件	1-轴型识别器触发
		u8	ErrFlag	:	1;		//故障位			0-地感正常	1-轴型识别器故障
		u8	SingleOrBoth :	1;	//单双轮标志位		0-单轮		1-双轮
	}Bit;
}sAxisType1_State_TypeDef;

void fDT3102_IO_Init(void);							//包括10路输入4路输出的初始化
sLand1_State_TypeDef fRead_Land1State(void);		//获取地感1状态
sLand2_State_TypeDef fRead_Land2State(void);		//获取地感2状态
sLight_State_TypeDef fRead_Light2State(void);		//获取光幕2状态
sAxisType1_State_TypeDef fRead_AxisType1State(void);//获取轴型识别器1状态

enum 
{
	TrafficSignal = 1,		//红绿灯
	BarrierGate				//道闸
};

//道闸引脚
enum
{
	BarrierGateOpenPort = 1,
	BarrierGateClosePort,
};

//道闸电平定义
#define	PortNormalOpen	1
#define PortNormalClose 0

s8 fPeripheral_Control(u8 type , u8 status);		//道闸、红绿灯控制
void BarrierGate_JDQ_Control(u8 port, u8 status);	//道闸引脚控制

#endif
