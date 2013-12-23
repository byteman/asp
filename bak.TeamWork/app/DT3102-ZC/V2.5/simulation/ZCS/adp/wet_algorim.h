
#ifndef __STATICTESTIFPROC_H_
#define __STATICTESTIFPROC_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

//ad通道
enum {
	ScalerChanle,				//秤台
	BackAxleChanle	,			//后轴
	PreAxle1Chanle,		//前轴
	PreAxle2Chanle,		//小秤台第二路ad
};

enum {
	KERR_NONE = 0,
	KERR_MIN,		//标定k值太小
	KERR_MAX,			//标定k值太大
	KERR_PARM,			//k值未写入flash
	KERR_ADERR,
};

#define DYNAMIC_AD_RATE 800									//动态ad采样率
#define WEIGHT_CYCLE    32     								//抽样周期
#define STATIC_AD_RATE  25    //(DYNAMIC_AD_RATE/WEIGHT_CYCLE)		//静态ad采样率

//算法业务
void Weight_Business(void *buf);

//算法模块初始化
int Weight_Init(void);

//获取秤台动态重量
int32 Get_Dynamic_Weight(void);

//设置秤台重量, 调试模拟使用
void Set_Static_Weight(int32 wet);		

//获取秤台静态重量
int32 Get_Static_Weight(void);	
int32 Get_Static_MaxWeight(void);	
void Clear_ScalerMaxWeight(void);	

//秤台置零
void Scaler_Set_Zero(BOOL bAuto);

//判断K值异常
uint8 Check_K_Value(float k);

//保存AD通道零点,标定零点
BOOL Weight_Calc_Zero(void);

//计算AD通道K系数
uint8 Weight_Cal_K(int inWet, int chanle);

//内蒙标定
BOOL SCSZC1_Cal_K(int inWet, int chanle, int place);


//设置秤台静态分度值
int Set_Scaler_StaticDrv(uint8 drv);

//设置秤台动态分度值
int Set_Scaler_DynamicDrv(uint8 drv);


//返回当前ad通道数
int32 Get_ADChanle_Num(void);

//初始化算法参数
void Init_Wet_Par(void);

int Set_ScalerStatic_K(float k);




#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
