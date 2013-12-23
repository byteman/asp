/*
 * History: 
 * 1. 2012-12-25 修改参数版本更新后加载默认加载参数后不再读取传感器K值的问题, 将end提前;
 */

#include "includes.h"
#include "weight_param.h"
#include "sys_param.h"
#include "wet_algorim.h"
#include "Speed_check.h"
#include "data_store_module.h"
#include "car_queue.h"

gWetParam gWetPar = {0};						 
static WetStatue Scaler_Weight = {0};

static gWetParam DefaultWetParm = {
	150000,			//整秤满量程
	0,				//开机置零范围
	4,				//手动置零范围
	4,				//零点跟踪速度		//1e
	50,				//动态分度值
	50,				//静态分度值
	6,				//零点跟踪范围		//5e
	1,				//静态滤波等级
	1,	  			//动态滤波等级
	30,				//超速阈值
	DYNAMIC_AD_RATE,		//零位跟踪时间		//1s
	
	150,				//轴1上轴阈值
	80,					//轴1下轴阈值
	200,				//轴2上轴阈值
	150,				//轴2下轴阈值
	150,				//轴3上轴阈值
	100, 				//轴3下轴阈值
	500,				//整秤上秤阈值
	500,				//整秤下秤阈值	
	150,	 			//光幕地感距离
	900,

	1000,				//0~5    km/h修正系数  
	1000,				//5~10  km/h
	1000,				//10~15 km/h	
	1000,				//15~20 km/h
	1000,				//20km/h
	1000,				//25km/h
	1000,				//30km/h
	
	1000,				//0~2.5km/h axle
	1000,				//2.5~5.0
	1000,				//5.0~7.5
	1000,				//7.5~10
	1000,				//
	1000,				//
	1000,				//
	1000,				//
	1000,				//
	1000,				//	

	17000,				//2轴车最大重量
	28000,				//3轴
	35000,				//4轴
	42000,				//5轴
	51000,				//6轴
	500,				//红绿灯重量阈值	

	6					//车辆最大轴数
};


BOOL Set_Max_AxleNum(unsigned char num)
{
	if((num < 2) || (num > MAX_AXLE_NUM)) return FALSE;

	
	if(Param_Write(GET_U8_OFFSET(ParamMaxAxleNum), &num,  sizeof(u8)))
	{
		gWetPar.nMaxAxleNum = num;
		return TRUE;
	}

	return FALSE;
}

static void Load_Default_Parm(uint32 ver)
{
	uint16 num = 0;
	
	Param_Write(GET_U32_OFFSET(ParamParVer),  &ver,    sizeof(u32));
	memcpy(&gWetPar, &DefaultWetParm, sizeof(gWetParam));
	
	Param_Write(GET_U32_OFFSET(ParamMaxWet),   &gWetPar.MaxWet,    sizeof(u32)); 
	Param_Write(GET_U8_OFFSET(ParamPowerZero), &gWetPar.PowerZero,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamHandlZero), &gWetPar.HandlZero,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamZeroTruckSpeed), &gWetPar.ZeroTruckSpeed,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamDynamicDivValue),&gWetPar.DynamicDivValue,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamStaticDivValue), &gWetPar.StaticDivValue,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamZeroTruckRang),  &gWetPar.ZeroTruckRang,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamStaticFilter),   &gWetPar.StaticFilterLevel,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamDynamicFilter),  &gWetPar.DynamicFilterLevel,  sizeof(u8));
	Param_Write(GET_U8_OFFSET(ParamSuperSpeedLevel),&gWetPar.SuperSpeedLevel, sizeof(u8)); 
	Param_Write(GET_U16_OFFSET(ParamZeroTruckTick),  &gWetPar.ZeroTruckTick, sizeof(u16));

	Param_Write(GET_U16_OFFSET(ParamUpWet),		 &gWetPar.TotalUpWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamDownWet),   	 &gWetPar.TotalDownWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamPreAxleUpWet),	 &gWetPar.PreAxleUpWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamPreAxleDownWet),&gWetPar.PreAxleDownWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamBackAxleUpWet),  &gWetPar.BackAxleUpWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamBackAxleDownWet),&gWetPar.BackAxleDownWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamWbAxleUpWet),  &gWetPar.WbAxleUpWet, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamWbAxleDownWet),&gWetPar.WbAxleDownWet, sizeof(u16));
	
	Param_Write(GET_U16_OFFSET(ParamSpeedLength), &gWetPar.Length, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamAxleLength), &gWetPar.AxleLength, sizeof(u16));
	Param_Write(GET_U16_OFFSET(ParamSpeed0k),  &gWetPar.Speed_0k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed5k),  &gWetPar.Speed_5k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed10k), &gWetPar.Speed_10k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed15k), &gWetPar.Speed_15k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed20k), &gWetPar.Speed_20k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed25k), &gWetPar.Speed_25k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamSpeed30k), &gWetPar.Speed_30k, sizeof(u16));

	Param_Write(GET_U16_OFFSET(ParamFollow0k),  &gWetPar.Follow_0k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow2P5k),  &gWetPar.Follow_2P5k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow5k),  &gWetPar.Follow_5k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow7P5k),  &gWetPar.Follow_7P5k, sizeof(u16)); 	
	Param_Write(GET_U16_OFFSET(ParamFollow10k), &gWetPar.Follow_10k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow12P5k), &gWetPar.Follow_12P5k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow15k), &gWetPar.Follow_15k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow20k), &gWetPar.Follow_20k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow25k), &gWetPar.Follow_25k, sizeof(u16)); 
	Param_Write(GET_U16_OFFSET(ParamFollow30k), &gWetPar.Follow_30k, sizeof(u16));

	Param_Write(GET_U32_OFFSET(ParamMax2Wet), &gWetPar.Max2AxleWet, sizeof(u32));
	Param_Write(GET_U32_OFFSET(ParamMax3Wet), &gWetPar.Max3AxleWet, sizeof(u32));
	Param_Write(GET_U32_OFFSET(ParamMax4Wet), &gWetPar.Max4AxleWet, sizeof(u32));
	Param_Write(GET_U32_OFFSET(ParamMax5Wet), &gWetPar.Max5AxleWet, sizeof(u32));
	Param_Write(GET_U32_OFFSET(ParamMax6Wet), &gWetPar.Max6AxleWet, sizeof(u32));
	Param_Write(GET_U32_OFFSET(ParamTrafficWet), &gWetPar.MaxTrafficWet,sizeof(u32));

	if(ver == SCS_ZC_VER_INIT) 
	{		
		//清空历史车辆数与过载车辆数
		Param_Write(GET_U16_OFFSET(ParamOverWetCarNum), &num, sizeof(uint16));
		Param_Write(GET_U16_OFFSET(ParamNormalCarNum), &num, sizeof(uint16));
		//初始化历史车辆信息队列指针
		Param_Write(GET_U16_OFFSET(ParamCarNumFront), &num, sizeof(uint16));
		Param_Write(GET_U16_OFFSET(ParamCarNumRear), &num, sizeof(uint16));
		//擦除整块flash,需要14秒才能擦完
		//Erase_Flash_Chip();
	}
}



//获取当前整秤经分度值处理后的静态重量
s32 Get_Scaler_DrvWeight(void)
{
	return Scaler_Weight.Wet;
}

//更新称重数据
void Update_Scaler_WetPar(WetStatue *wet)
{
	Scaler_Weight.Wet        = wet->Wet;			
	Scaler_Weight.PreAxle1AD = wet->PreAxle1AD;
	Scaler_Weight.PreAxle2AD = wet->PreAxle2AD;
	Scaler_Weight.ScalerAD   = wet->ScalerAD;
	Scaler_Weight.BackAxleAD = wet->BackAxleAD;
	Scaler_Weight.SmallWet  = wet->SmallWet;
	Scaler_Weight.BigWet  = wet->BigWet;
	Scaler_Weight.ScalerStableFlag = wet->ScalerStableFlag;
	Scaler_Weight.ScalerZeroFlag   = wet->ScalerZeroFlag;
}

//获取称重数据
void Get_Scaler_WetPar(WetStatue *gWet)
{
#if SYS_CRITICAL_METHOD == 3
	SYS_CPU_SR cpu_sr;
#endif

	SYS_ENTER_CRITICAL();
	memcpy(gWet, &Scaler_Weight, sizeof(WetStatue));
	SYS_EXIT_CRITICAL();
}

void Weight_Param_Init(void)
{
	uint32 var = 0;

	//判断参数版本号，不等则恢复默认参数
	if(Param_Read(GET_U32_OFFSET(ParamParVer),  &var,    sizeof(u32))){
		if(var != SCS_ZC_VER) {
			Load_Default_Parm(SCS_ZC_VER);
			goto end;
		}
	}
			
	Param_Read(GET_U32_OFFSET(ParamMaxWet),   &gWetPar.MaxWet,    sizeof(u32)); 
	Param_Read(GET_U8_OFFSET(ParamPowerZero), &gWetPar.PowerZero,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamHandlZero), &gWetPar.HandlZero,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamZeroTruckSpeed), &gWetPar.ZeroTruckSpeed,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamDynamicDivValue),&gWetPar.DynamicDivValue,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamStaticDivValue), &gWetPar.StaticDivValue,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamZeroTruckRang),  &gWetPar.ZeroTruckRang,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamStaticFilter),   &gWetPar.StaticFilterLevel,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamDynamicFilter),  &gWetPar.DynamicFilterLevel,  sizeof(u8));
	Param_Read(GET_U8_OFFSET(ParamSuperSpeedLevel),&gWetPar.SuperSpeedLevel, sizeof(u8)); 
	Param_Read(GET_U16_OFFSET(ParamZeroTruckTick),  &gWetPar.ZeroTruckTick, sizeof(u16));
	
	Param_Read(GET_U16_OFFSET(ParamUpWet),		 &gWetPar.TotalUpWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamDownWet),   	 &gWetPar.TotalDownWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamPreAxleUpWet),	 &gWetPar.PreAxleUpWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamPreAxleDownWet),&gWetPar.PreAxleDownWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamBackAxleUpWet),  &gWetPar.BackAxleUpWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamBackAxleDownWet),&gWetPar.BackAxleDownWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamWbAxleUpWet),  &gWetPar.WbAxleUpWet, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamWbAxleDownWet),&gWetPar.WbAxleDownWet, sizeof(u16));
	
	Param_Read(GET_U16_OFFSET(ParamSpeedLength), &gWetPar.Length, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamAxleLength), &gWetPar.AxleLength, sizeof(u16));
	Param_Read(GET_U16_OFFSET(ParamSpeed0k),  &gWetPar.Speed_0k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed5k),  &gWetPar.Speed_5k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed10k), &gWetPar.Speed_10k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed15k), &gWetPar.Speed_15k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed20k), &gWetPar.Speed_20k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed25k), &gWetPar.Speed_25k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamSpeed30k), &gWetPar.Speed_30k, sizeof(u16));

	Param_Read(GET_U16_OFFSET(ParamFollow0k),  &gWetPar.Follow_0k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow2P5k),  &gWetPar.Follow_2P5k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow5k),  &gWetPar.Follow_5k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow7P5k),  &gWetPar.Follow_7P5k, sizeof(u16)); 	
	Param_Read(GET_U16_OFFSET(ParamFollow10k), &gWetPar.Follow_10k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow12P5k), &gWetPar.Follow_12P5k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow15k), &gWetPar.Follow_15k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow20k), &gWetPar.Follow_20k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow25k), &gWetPar.Follow_25k, sizeof(u16)); 
	Param_Read(GET_U16_OFFSET(ParamFollow30k), &gWetPar.Follow_30k, sizeof(u16));

	Param_Read(GET_U32_OFFSET(ParamMax2Wet), &gWetPar.Max2AxleWet, sizeof(u32));
	Param_Read(GET_U32_OFFSET(ParamMax3Wet), &gWetPar.Max3AxleWet, sizeof(u32));
	Param_Read(GET_U32_OFFSET(ParamMax4Wet), &gWetPar.Max4AxleWet, sizeof(u32));
	Param_Read(GET_U32_OFFSET(ParamMax5Wet), &gWetPar.Max5AxleWet, sizeof(u32));
	Param_Read(GET_U32_OFFSET(ParamMax6Wet), &gWetPar.Max6AxleWet, sizeof(u32));
	Param_Read(GET_U32_OFFSET(ParamTrafficWet), &gWetPar.MaxTrafficWet,sizeof(u32));

end:
	Param_Read(GET_F32_OFFSET(ParamDyncK),      	&gWetPar.DynamicK, sizeof(float)); 
	Param_Read(GET_F32_OFFSET(ParamPreAxle1Zero), &gWetPar.PreAxle1ZeroAD, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamPreAxle2Zero), &gWetPar.PreAxle2ZeroAD, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamScalerZero), 	 &gWetPar.ScalerZeroAD, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamBackAxleZero), &gWetPar.BackAxleZeroAD, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamPreAxle1_K), &gWetPar.PreAxle1_K, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamPreAxle2_K), &gWetPar.PreAxle2_K, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamScaler_K),   &gWetPar.Scaler_K, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamBackAxle_K), &gWetPar.BackAxle_K, sizeof(float));
	Param_Read(GET_F32_OFFSET(ParamTotal_K),    &gWetPar.Total_K, sizeof(float));
	
	if (Check_K_Value(gWetPar.Scaler_K) != KERR_NONE)
		gWetPar.Scaler_K = 1.0f; 

	if(Check_K_Value(gWetPar.PreAxle1_K) != KERR_NONE)
	    gWetPar.PreAxle1_K = 1.0f;

	if(Check_K_Value(gWetPar.PreAxle2_K) != KERR_NONE)
		gWetPar.PreAxle2_K = 1.0f;

	if(Check_K_Value(gWetPar.BackAxle_K) != KERR_NONE)
		gWetPar.BackAxle_K = 1.0f;

	//初始化车辆最大轴数
	Param_Read(GET_U8_OFFSET(ParamMaxAxleNum), (void *)&gWetPar.nMaxAxleNum, sizeof(u8));
	if((gWetPar.nMaxAxleNum > MAX_AXLE_NUM) || (gWetPar.nMaxAxleNum < 6))
	{
		gWetPar.nMaxAxleNum = 6;	
	}

	Set_Speed_Length(gWetPar.Length*10);
	SysInitParam();
//	Init_Wet_Par();
}
 
