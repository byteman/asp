/* 
 * File:    sys_param.h
 * Brief:   FRAM 参数地址定义 
 *
 * History:
 * 1. 2012-11-24 创建文件, 从长明云南程序移植 river 
 * 2. 2012-11-28 添加debug打印等级标志
 * 3. 2012-12-26 增加红绿灯重量阈值参数
 */

#ifndef SYSPARAM_H	
#define SYSPARAM_H

#ifdef __cplusplus		   	
extern "C" {
#endif

#include "sys_cpu.h"
#include "stm32f4xx.h"
#include "app.h"
	
	typedef struct  {
    u8 unit;            // 0:kg  1:t
    u8 Beep;            // 0:close 1:open
    u8 password[10];    // 密码
	u8 second;					//秒
} SSYSSET;


//FRAM分区：
//整车式参数区域    0 ~ 1023;
//双秤台参数区域 1024 ~ 3072

/****************************************整车式参数区****************************************/

#define U8_PARAM_BASE 		0		//单字节区域, 定义64字节长度
#define U16_PARAM_BASE		64		//2字节区域, 定义128字节长度
#define F32_PARAM_BASE		192		//浮点参数区域, 定义256字节长度
#define U32_PARAM_BASE		448	    //整型参数区域，定义256字节长度
#define USR_PARAM_BASE		704

#define U8_PARAM_OFFSET         0
#define U16_PARAM_OFFSET        64
#define F32_PARAM_OFFSET        128
#define U32_PARAM_OFFSET        192

enum {
/********单字节区域, 最大存储64个参数********/
    ParamPowerZero = 0,			//开机置零范围,最大20%
    ParamHandlZero,				//手动置零范围,最大4%
    ParamZeroTruckRang,			//零点跟踪范围
    ParamZeroTruckSpeed,			//零点跟踪速度
    ParamDynamicDivValue,			//动态分度值
    ParamStaticDivValue,			//静态分度值
    ParamDynamicFilter,			//动态滤波等级
    ParamStaticFilter,			//静态滤波等级
    ParamSuperSpeedLevel,			//超速阈值
    ParamLongCarMode,				//长车模式选择
    ParamTrafficSignalFlag,		//红绿灯标志
    ParamBGFlag,					//道闸标志
    ParamVoicePowerOn,			//蜂鸣器开关
    ParamPreLCFlag,				//前光幕标志，前光幕与前地感二者有一
    ParamDebugLevel,				//调试信息打印等级
	ParamFollowCarMode,			//跟车等级
	ParamLittleFollowCarMode,		//小车跟车等级
	ParamMaxAxleNum,
    
/********2字节区域, 最大存储64个参数********/
    ParamBdr = 64,				//串口2波特率
    ParamOverWetCarNum,		//dataflash过载车辆数
    ParamNormalCarNum,		//dataflash正常过车车辆数
    ParamUpWet,				//上秤阈值
    ParamDownWet,				//下秤阈值
    ParamPreAxleUpWet,			//轴1识别器上轴阈值
    ParamPreAxleDownWet,
    ParamBackAxleUpWet,			//轴2识别器上轴阈值
    ParamBackAxleDownWet,
    ParamWbAxleUpWet,			//轴3识别器上轴阈值  //弯板-->>PreAxle2
    ParamWbAxleDownWet,
    ParamSpeedLength,			//光幕地感距离，单位cm
    ParamAxleLength,
    ParamZeroTruckTick,		//零位跟踪时间

	ParamSpeed0k,				//速度修正系数
    ParamSpeed5k,				
    ParamSpeed10k,
    ParamSpeed15k,
    ParamSpeed20k,
    ParamSpeed25k,
    ParamSpeed30k,
    ParamResetCnt,				//系统复位次数

    ParamCarNumFront,			//历史车辆数据队头位置
    ParamCarNumRear,			//历史车辆数据队尾指针

	ParamDeviceSignal, 			//外设电平设置
  
	ParamFollow0k,				//轴速度修正系数
    ParamFollow2P5k,				
    ParamFollow5k,
    ParamFollow7P5k,				
    ParamFollow10k,
	ParamFollow12P5k,		
	ParamFollow15k,
    ParamFollow20k,
    ParamFollow25k,
    ParamFollow30k,


/********浮点参数区域, 最大存储64个参数********/
    ParamPreAxle1_K	= 128,	//4路ad K系数
    ParamPreAxle2_K,
    ParamScaler_K,
    ParamBackAxle_K,	
    ParamTotal_K,
    ParamDyncK,				//动态系数
    ParamPreAxle1Zero,		//4路ad零点
    ParamPreAxle2Zero,
    ParamScalerZero,
    ParamBackAxleZero,
    
/********整型参数区域, 最大存储64个参数********/
    ParamAppVer  =  192,		//程序版本号
    ParamParVer,				//整车式参数版本，根据参数版本决定是否加载默认参数
    ParamAppMod,				//流程模式
    ParamMaxWet,				//整秤最大重量
    ParamTrafficWet, 			//红绿灯阈值
    ParamMax2Wet,				//2轴车最大重量
    ParamMax3Wet,
    ParamMax4Wet,
    ParamMax5Wet,
    ParamMax6Wet,
    ParamAxleMaxWet,
    
/********其他参数区域, 起始地址704********/
    ParamNet	 = USR_PARAM_BASE, 	//网络参数
    ParamOther = USR_PARAM_BASE + 40
};

#define GET_U8_OFFSET(PARAMX)  (U8_PARAM_BASE + PARAMX - U8_PARAM_OFFSET)
#define GET_U16_OFFSET(PARAMX) (U16_PARAM_BASE + ((PARAMX - U16_PARAM_OFFSET)<<1))
#define GET_F32_OFFSET(PARAMX) (F32_PARAM_BASE + ((PARAMX - F32_PARAM_OFFSET)<<2))
#define GET_U32_OFFSET(PARAMX) (U32_PARAM_BASE + ((PARAMX - U32_PARAM_OFFSET)<<2))

/********************************************************************************************/

/****************************************双秤台参数区****************************************/

//双秤台参数版本，100为出厂初始版本号； 
#define SCS_SC_VER_INIT 100
#define SCS_SC_VER	 100
// #define WETPARAM_VER    100


//参数信息2012.11.21 lxj
#define  SYSPARAM_VER_ADDR      1024		//0x00    // 版本地址 4
#define  SYSPARAM_SYSSET        1028		//0x10    // 系统设置 12
#define  SYSPARAM_DEVSET      	  1040   	//0x30      // 外设设置 7

#define  WETPARAM_VER_ADDR      1064			//0x1000  // 版本地址 4
#define  WETPARAM_FLASHADDR     1068			//0x1004  // 数据偏移地址 4
#define  WETPARAM_DATAPACKNO    1072			//0x1008  // 数据包编号 4
#define  WETPARAM_MATRIC         1076			//0x1010  // 称重参数 50
#define  WETPARAM_CALIBPARAM_A  1140			//0x1200  // A标定    16
#define  WETPARAM_CALIBPARAM_B  1200			//0x1280  // B标定    16
#define  WETPARAM_OVERLOAD       1300 		//0x1300  // 超载阈值  14
#define  WETPARAM_SPEEDMOD       1320			//0x1310  // 速度修正  13
#define  WETPARAM_CARMOD         1340		//0x1320  // 车型修正  24*20
//#define  WETPARAM_CARMOD        0x1500  // 车型修正  24*20

#define  CALIB_VER_ADDR         1900		//0x1A00   // 标定原始数据版本 4
#define  CALIB_DATA_A           1904		//	0x1C00   // A标定原始数据    121
#define  CALIB_DATA_B           2048		//0x1E00   // B标定原始数据    121

/********************************************************************************************/

void Param_Interface_Init(void);

/*
 * 功能：写入参数
 * 参数：addr 地址
         data 写入参数地址
         size 参数长度
 * 返回：true 成功 FALSE 失败
 */
BOOL   Param_Write(int addr,  void* data, uint32 size);

/*
 * 功能：读取参数
 * 参数：addr 地址
         data 读取参数地址
         size 参数长度
 * 返回：true 成功 FALSE 失败
 */
BOOL   Param_Read(int addr,  void* data, uint32 size);

//保存密码的设置信息
u8 SysSetParamSave(void);
u8 SysInitParam(void);

u8 GetSysStatus(u32 bit);
u8 SetSysStatus(u32 bit,u8 en);

//===============================
//IO
u16 Get_Device_Signal(void);
BOOL Set_Device_Signal(u16 value);


#ifdef __cplusplus		   	
}
#endif

#endif
