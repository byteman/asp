#include "includes.h"
#include "sys_config.h"
#include "sys_param.h"
#include "debug_info.h"
#include "app_beep.h"

static SysMod AppMode = SCS_ZC_7;									//应用模式模式

static volatile LongCarMod LongCarMode = HandleLongCarMode;			//长车流程模式
static volatile BOOL TrafficSignalFlag = FALSE;	//红绿灯标志
static volatile BOOL BGFlag = FALSE; 				//道闸标志
static volatile BOOL BeepFlag = FALSE;				//蜂鸣器开关
static volatile FollowCarMod FollowMode = FollowAtOnce;
static u16 APPResetCount = 0;						//系统复位次数

BOOL Set_System_Mode(SysMod mode)
{
	if(mode >= SCS_CNT || mode == SCS_ZC_4)
		mode = SCS_ZC_7;
	AppMode = mode;
	return Param_Write(GET_U32_OFFSET(ParamAppMod), &mode, sizeof(uint32));
}

//获取系统模式
SysMod Get_System_Mode(void)
{
	return AppMode;
}

LongCarMod Get_LongCar_Mode(void)
{
	return LongCarMode;
}

void Set_LongCar_Mode(LongCarMod mode)
{
	LongCarMode = mode;
	Param_Write(GET_U8_OFFSET(ParamLongCarMode), &mode, sizeof(uint8));
}


BOOL Get_TrafficSignal_Flag(void)		//return ture: 有红绿灯 FALSE: 没有
{
	return TrafficSignalFlag;
}

void Set_TrafficSignal_Flag(BOOL type)		//return ture: 有红绿灯 FALSE: 没有
{
	TrafficSignalFlag = type;
	Param_Write(GET_U8_OFFSET(ParamTrafficSignalFlag), &type, sizeof(uint8));
}


BOOL Get_BGFlag(void)
{
	return BGFlag;
}

void Set_BGFlag(BOOL type)
{
	BGFlag = type;
	Param_Write(GET_U8_OFFSET(ParamBGFlag), &type, sizeof(uint8));
}

FollowCarMod Get_FollowCarMode(void)
{
	return FollowMode;
}

void Set_FollowCarMode(char type)
{	
	if((type < 0) || (type > FollowFarPay)) return;

	FollowMode = (FollowCarMod)type;
	Param_Write(GET_U8_OFFSET(ParamFollowCarMode), &type, sizeof(uint8));
}

BOOL Get_Beep_Flag(void)
{
	return BeepFlag;
}

void Set_Beep_Flag(BOOL type)
{
	BeepFlag = type;
	Param_Write(GET_U8_OFFSET(ParamVoicePowerOn), &type, sizeof(uint8));
	Beep_Power(type);
}

unsigned short Get_AppReset_Count(void)
{
	return APPResetCount;
}

void System_Config_Init(void)
{
//	BOOL err = FALSE;

	Param_Interface_Init();
	RDelay(50);

	//增加系统复位次数
	Param_Read(GET_U16_OFFSET(ParamResetCnt), &APPResetCount, sizeof(u16));
	if(APPResetCount == 0xffff) 
		APPResetCount = 0;

	APPResetCount++;
	Param_Write(GET_U16_OFFSET(ParamResetCnt), &APPResetCount, sizeof(u16));
	printf("APPResetCnt = %d\r\n", APPResetCount);
	
	//计量模式
	Param_Read(GET_U32_OFFSET(ParamAppMod), &AppMode, sizeof(uint32));
	if(((unsigned int)AppMode > SCS_CNT) || ((unsigned int)AppMode == SCS_ZC_4))
		AppMode = SCS_ZC_7;

	//长车称重模式
	Param_Read(GET_U8_OFFSET(ParamLongCarMode), (void *)&LongCarMode, sizeof(uint8));
	if((unsigned int)LongCarMode > HandleLongCarMode) LongCarMode = AutoLongCarMode;

	//红绿灯配置
	Param_Read(GET_U8_OFFSET(ParamTrafficSignalFlag), (void *)&TrafficSignalFlag, sizeof(uint8));
	if((unsigned int)TrafficSignalFlag > 1) TrafficSignalFlag = TRUE;
	

	//道闸配置
	Param_Read(GET_U8_OFFSET(ParamBGFlag), (void *)&BGFlag, sizeof(uint8));
	if((unsigned int)BGFlag > 1) BGFlag = FALSE;

	//蜂鸣器开关
	Param_Read(GET_U8_OFFSET(ParamVoicePowerOn), (void *)&BeepFlag, sizeof(uint8));
	if((unsigned int)BeepFlag > 1) BeepFlag = TRUE;

	//跟车等级
	Param_Read(GET_U8_OFFSET(ParamFollowCarMode), (void *)&FollowMode, sizeof(uint8));
	if((unsigned char)FollowMode > FollowFarPay) FollowMode = FollowAtOnce;
	
	//调试信息打印等级
	Print_Debug_Init();
}

