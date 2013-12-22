/* 
 * File:    sys_config.h
 * Brief:   system parameter configuration 
 *
 * History:
 * 1. 2012-11-24 创建文件 river
 */

#ifndef SYSCONFIG_H	
#define SYSCONFIG_H

#ifdef __cplusplus		   	
extern "C" {
#endif
#include "sys_cpu.h"

//5种系统模式
typedef enum {
	SCS_ZC_1 = 0,		
	SCS_ZC_2,
	SCS_ZC_3,
	SCS_ZC_4,
	SCS_ZC_5,
	SCS_ZC_6,
	SCS_ZC_7,
	SCS_CNT
}SysMod;

//长车称重模式
typedef enum {
	AutoLongCarMode,
	HandleLongCarMode
}LongCarMod;

//跟车选项
typedef enum {
	FollowAtOnce,		//光幕断尾1.2秒立即获取跟车重量
	FollowNoFarPay, 	//跟车没有离秤缴费
	FollowFarPay		//跟车有离秤缴费
}FollowCarMod;

void System_Config_Init(void);

/*
 * 系统模式选择
 * 参数：model工作模式
 * 说明：从5种工作模式中选择设置当前的工作模式 
 */
BOOL Set_System_Mode(SysMod mode);
SysMod Get_System_Mode(void);

LongCarMod Get_LongCar_Mode(void);
void Set_LongCar_Mode(LongCarMod mode);

BOOL Get_TrafficSignal_Flag(void);		//return ture: 有红绿灯 FALSE: 没有
void Set_TrafficSignal_Flag(BOOL type);

BOOL Get_BGFlag(void);
void Set_BGFlag(BOOL type);

FollowCarMod Get_FollowCarMode(void);
void Set_FollowCarMode(char type);

BOOL Get_Beep_Flag(void);
void Set_Beep_Flag(BOOL type);

unsigned short Get_AppReset_Count(void);


#ifdef __cplusplus		   	
}
#endif

#endif
