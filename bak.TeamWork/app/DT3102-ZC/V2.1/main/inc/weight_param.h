/* 
 * File:    weight_param.h
 * Brief:   global weight parameter  
 *
 * History:
 * 1. 2012-11-24 创建文件, 从杨长明甘肃程序移植 river 
 */

#ifndef WEIGH_PARAM_H	
#define WEIGH_PARAM_H

#ifdef __cplusplus		   	
extern "C" {
#endif

#include "sys_cpu.h"

typedef struct {
	u32 MaxWet;					//整秤满量程
	u8  PowerZero;				//开机置零范围
	u8  HandlZero;				//手动置零范围
	u8  ZeroTruckSpeed;		//零点跟踪速度
	u8	DynamicDivValue;		//动态分度值
	u8  StaticDivValue;		//静态分度值
	u8  ZeroTruckRang;			//零点跟踪范围
	u8	StaticFilterLevel;	//静态滤波等级
	u8  DynamicFilterLevel;	//动态滤波等级
	u8	SuperSpeedLevel;		//超速阈值
	
	u16	ZeroTruckTick;			//零位跟踪时间
	u16  PreAxleUpWet;			//轴1上轴阈值
	u16  PreAxleDownWet;
	u16  BackAxleUpWet;			//轴2上轴阈值
	u16  BackAxleDownWet;
	u16  WbAxleUpWet;			//轴3上轴阈值
	u16  WbAxleDownWet;
	u16  TotalUpWet;		//整秤上秤阈值
	u16  TotalDownWet;
	u16  Length; 			//光幕地感距离
	u16  AxleLength;		//轴识别器宽度

	u16 Speed_0k;		//0km/h~5km/h
	u16 Speed_5k;		//5km/h~10km/h修正系数
	u16 Speed_10k;		//10km/h
	u16 Speed_15k;
	u16 Speed_20k;
	u16 Speed_25k;
	u16 Speed_30k;		//30km/h以上
	
	u16 Follow_0k;		//
	u16 Follow_2P5k;	//
	u16 Follow_5k;		//
	u16 Follow_7P5k;	//
	u16 Follow_10k;		//
	u16 Follow_12P5k;		//
	u16 Follow_15k;		//	
	u16 Follow_20k;		//
	u16 Follow_25k;		//
	u16 Follow_30k;		//

	u32 Max2AxleWet;		//车辆超重重量，按轴分
	u32 Max3AxleWet;
	u32 Max4AxleWet;
	u32 Max5AxleWet;
	u32 Max6AxleWet;
	u32 MaxTrafficWet;		//红绿灯重量阈值

	float DynamicK;	  	//动态系数
	float PreAxle1_K;		//ad通道k系数
	float PreAxle2_K;
	float Scaler_K;
	float BackAxle_K;
	float Total_K;
	float PreAxle1ZeroAD;	//ad通道ad零点
	float PreAxle2ZeroAD;
	float ScalerZeroAD;
	float BackAxleZeroAD;
	
	u8	nMaxAxleNum;			//定义车辆最大轴数
}gWetParam;

/*//称重数据接口
 *ad通道由称重计量模式决定
 *SCS_ZC_1:
	PreAxle1AD ScalerAD有效; PreAxle1AD表示秤台第一路AD, ScalerAD表示秤台第二路AD
 *SCS_ZC_2:
	PreAxle1AD ScalerAD有效; PreAxleAD表示前轴识别器AD, ScalerAD表示秤台AD 
 *SCS_ZC_3:
	PreAxle1AD ScalerAD BackAxle有效; PreAxleAD表示前轴识别器AD, ScalerAD表示秤台AD,BackAxle表示后轴识别器AD
 *SCS_ZC_5,SCS_ZC_6:
	PreAxle1AD PreAxle2AD ScalerAD BackAxle有效; PreAxle1AD,PreAxle2AD表示小秤台AD, ScalerAD表示秤台AD,BackAxle表示后轴识别器AD
*/
typedef struct 
{
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

extern gWetParam gWetPar;			//全局称重参数

int32 Get_Scaler_DrvWeight(void);

//更新称重数据
void Update_Scaler_WetPar(WetStatue *wet);

//获取称重数据
void Get_Scaler_WetPar(WetStatue *gWet);

void Weight_Param_Init(void);

//整车式参数版本，200为出厂初始版本号； 
#define SCS_ZC_VER_INIT 200
#define SCS_ZC_VER	 201		//201版本修正轴上下阈值，增加轴速度修正	

BOOL Set_Max_AxleNum(unsigned char num);

#ifdef __cplusplus		   	
}
#endif

#endif


