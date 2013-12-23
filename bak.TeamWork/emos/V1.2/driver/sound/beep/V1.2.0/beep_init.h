 //硬件接口不同时，可能会更改的配置参数
#ifndef _BEEP_CFG_H	
#define _BEEP_CFG_H

#include "includes.h"

//IOCTL命令
#define 	BEEPCMD_SETONOFF										0		//蜂鸣器开关设置
#define 	BEEPCMD_GETONOFF										1		//蜂鸣器开关获取
#define		BEEPCMD_LookDevVer									2		//查询设备版本号
#define		BEEPCMD_LookDevDisc									3		//查询设备描述
#define		BEEPCMD_LookDevShared								4		//查询设备共享
#define		BEEPCMD_LookDevOpenCount						5		//查询设备打开次数

//IOCTL命令
#define 	BEEPCMD_SETONOFF										0		//蜂鸣器开关设置
#define 	BEEPCMD_GETONOFF										1		//蜂鸣器开关获取
#define 	BEEPCMD_ON											    2   //蜂鸣器叫指定ms
typedef struct
{
	u32 count;							//发声次数
	u16 time;								//每次发声时间，单位毫秒
}BEEP_STRUCTURE;


#endif	//_BEEP_CFG_H

/*
测试：
	#include "beep_init.h"			//蜂鸣器初始化
	#include "beep_usercfg.h"		//BEEP_STRUCTURE

	//蜂鸣器配置
	static BEEPCTL_STRUCT Beep_User =
	{
		GPIOE,					//引脚端口号
		GPIO_Pin_15			//引脚号
	};

	HVL_ERR_CODE err = HVL_NO_ERR;	//设备操作错误
	HANDLE BEEP_HANDLE;							//蜂鸣器设备句柄
	u8* DevPt = (u8*)0;							//设备信息指针		调试时防止被优化掉

	BEEP_STRUCTURE Beep_test = {3,100};			//蜂鸣器鸣叫

	if(Beep_Init(Beep_User) == 0)						//蜂鸣器初始化
		return;
	
	BEEP_HANDLE	= CharDeviceOpen(CHAR_BEEP,0,&err);					//打开蜂鸣器
	if(err != HVL_NO_ERR)
		while(1);
		
	DevPt = (u8*)CharDeviceIoctl(BEEP_HANDLE,BEEPCMD_LookDevDisc,0);		//获取蜂鸣器设备描述
	CharDeviceWrite(BEEP_HANDLE,(INT8S*)&Beep_test,0);									//蜂鸣器测试
*/
