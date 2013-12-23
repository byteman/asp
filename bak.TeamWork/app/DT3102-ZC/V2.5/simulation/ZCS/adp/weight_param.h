
#ifndef _WET_PARAM
#define _WET_PARAM
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
#endif
