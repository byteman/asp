#ifndef _SYS_CONFIG_H_ 
#define _SYS_CONFIG_H_ 

#include "includes.h"
#include "stdlib.h"
#include "string.h"
#include "weight_param.h"

#define DYNAMIC_AD_RATE 800									//动态ad采样率
#define WEIGHT_CYCLE    32     								//抽样周期
#define STATIC_AD_RATE  25    //(DYNAMIC_AD_RATE/WEIGHT_CYCLE)		//静态ad采样率

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif


#define SYS_ERR_NONE                   0u



 //5种系统模式
 typedef enum {
 	SCS_ZC_1 = 0,		
 	SCS_ZC_2,
 	SCS_ZC_3,
 	SCS_ZC_4,
 	SCS_ZC_5,
 	SCS_ZC_6,
	SCS_ZC_7
 }SysMod;

 //长车称重模式
typedef enum {
	AutoLongCarMode,
	HandleLongCarMode
}LongCarMod;


typedef struct {
	u8 ad1_err:1;		//第1路AD故障
	u8 ad2_err:1;
	u8 ad3_err:1;
	u8 ad4_err:1;
}ADStatus;
//其中1、2为上称方向的顺序 
typedef union {
	struct {
		u8 LC1_err:1;		//光幕1  0-normal status,1-err status
		u8 LC1_status:1;	//	 0-release,1-trigger
		u8 LC2_err:1;		//光幕2 分车光幕故障
		u8 LC2_status:1;	//分车光幕状态	
		u8 GC1_err:1;		//前地感		
		u8 GC1_status:1;	
		u8 GC2_err:1;		//后地感
		u8 GC2_status:1;
		
		u8 tireSen_err:1;		//轮胎识别器故障
		u8 single_double:1;		//单双轮   0-signal,1-double
		u8 ud1:6;			//备用 6位
		
		u8 ad1_err:1;		//第1路AD故障
		u8 ad2_err:1;
		u8 ad3_err:1;
		u8 ad4_err:1;
		u8 axle11_valid:1;		//前轴第一路有效标志
		u8 axle12_valid:1;
		u8 axle2_valid:1;		//后轴
		u16 ud2:9;			//备用 4位		 
	}bitValue;

	u32 value;
}DeviceStatus;	//外设状态信息 


typedef struct {
	s32 Wet;				//整秤分度重量
	s32 Axle1_Wet;			//前轴轴重
    s32 Axle2_Wet;			//后轴轴重
	float PreAxle1AD;		//ad值
	float PreAxle2AD;
    float ScalerAD;
	float BackAxleAD;
	u16  ScalerStableFlag;				//秤台稳定标志
	u16	ScalerZeroFlag;				//秤台零位标志
}WetStatue; 

#define GET_U8_OFFSET(PARAMX)  (U8_PARAM_BASE + PARAMX - U8_PARAM_OFFSET)
#define GET_U16_OFFSET(PARAMX) (U16_PARAM_BASE + ((PARAMX - U16_PARAM_OFFSET)<<1))
#define GET_F32_OFFSET(PARAMX) (F32_PARAM_BASE + ((PARAMX - F32_PARAM_OFFSET)<<2))
#define GET_U32_OFFSET(PARAMX) (U32_PARAM_BASE + ((PARAMX - U32_PARAM_OFFSET)<<2))

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
    
/********2字节区域, 最大存储64个参数********/
    ParamBdr = 64,				//串口2波特率
    ParamOverWetCarNum,		//dataflash过载车辆数
    ParamNormalCarNum,		//dataflash正常过车车辆数
    ParamUpWet,				//上秤阈值
    ParamDownWet,				//下秤阈值
    ParamAxle1UpWet,			//前轴识别器上轴阈值
    ParamAxle1DownWet,
    ParamAxle2UpWet,			//后轴识别器上轴阈值
    ParamAxle2DownWet,
    ParamSpeedLength,			//光幕地感距离，单位cm
    ParamAxleLength,
    ParamZeroTruckTick,		//零位跟踪时间

    ParamSpeed5k,				//速度修正系数
    ParamSpeed10k,
    ParamSpeed15k,
    ParamSpeed20k,
    ParamSpeed25k,
    ParamSpeed30k,


    
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

//分车选项
typedef enum {
	DevideAtOnce,		//光幕断尾1.2秒立即获取跟车重量
	DevideAtDown		//跟车有离秤缴费
}FollowCarMod;


extern gWetParam gWetPar;

SysMod Get_System_Mode();
int Updata_AD_Status(u8 chanel,u8 status);
void Set_CarSpeed(int32 speed);
void Update_AxleDevice_Status(DeviceStatus status);
int Param_Write(uint32 nAddr, char* pBuff, int nLen);
void Scaler_Set_Zero(BOOL bAuto);
void Update_Scaler_WetPar(WetStatue *wet);
unsigned char Get_NetStatus(void);
LongCarMod Get_LongCar_Mode(void);
BOOL Get_LC_Status(u8 no);
uint16 Get_CarSpeed (void);
uint16 Get_Speed_Length(void);
BOOL Get_TrafficSignal_Flag(void);
void CarQueue_Print_Info(void);
void Clear_ScalerEmpty_Count(void);
BOOL Get_BGFlag(void);
FollowCarMod Get_FollowCarMode(void);
FollowCarMod Get_LittleFollowCarMode(void);


//消息队列
uint8 Task_QPost(MsgQ *pMsgQ, void *pMsg);
uint8 Task_QPend(MsgQ *pMsgQ, void *pMsg, int len, uint16 timeout);

void barriergate_open(void);
#endif

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif


