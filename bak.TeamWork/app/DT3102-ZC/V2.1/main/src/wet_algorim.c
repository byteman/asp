/*
 * History:
 * 2012-12-27 修改标定k系数为先判断再修改
 */
#include "includes.h"
#include "task_msg.h"
#include "app_msg.h"
#include "weight_param.h"
#include "wet_algorim.h"
#include "axle_business.h"
#include "sys_config.h"
#include "device_info.h"
#include "sys_param.h"
#include "Speed_check.h"
#include "net_business_thread.h"
#include "debug_info.h"

#include "weight.h"
#include "FirFilter.h"
#include "ADCValueCheck.h"
#include "LineFit.h"
#include "Standlize.h"
#include "ZeroTrace.h"
#include "DyncProc.h"
#include "StaticProc.h"
#include "DigSample.h"
#include "StateCheck.h"
#include "AlexParser.h"
#include "Mode7AlexParser.h"
#include "Mode8AlexParser.h"
#include "DyncSensorGetAxisAlgo.h"
#include "DyncAlexParserThree.h"
#include "axletype_business.h"
#include "rmcp.h"


static const float znumF25L1[25] = 
{
    3.315243e-04,1.857341e-03,4.945056e-03,1.000574e-02,1.726106e-02,2.665421e-02,3.77984e-02,4.997899e-02,6.221557e-02,7.337847e-02,
    8.234226e-02,8.815048e-02,9.016179e-02,8.815048e-02,8.234226e-02,7.337847e-02,6.221557e-02,4.997899e-02,3.77984e-02,2.665421e-02,
    1.726106e-02,1.000574e-02,4.945056e-03,1.857341e-03,3.315243e-04
};

static float const znumF800L12[100] = 
{
    -4.073e-05,-6.4e-05,-9.222e-05,-1.252e-04,-1.624e-04,-2.029e-04,-2.451e-04,-2.871e-04,-3.264e-04,-3.598e-04,
    -3.833e-04,-3.927e-04,-3.827e-04,-3.478e-04,-2.818e-04,-1.78e-04,-2.957e-05,1.706e-04,4.296e-04,7.546e-04,
    1.152e-03,1.629e-03,2.189e-03,2.839e-03,3.581e-03,4.417e-03,5.349e-03,6.375e-03,7.494e-03,8.7e-03,
    9.987e-03,1.135e-02,1.277e-02,1.425e-02,1.576e-02,1.73e-02,1.885e-02,2.039e-02,2.191e-02,2.338e-02,
    2.479e-02,2.611e-02,2.734e-02,2.846e-02,2.945e-02,3.029e-02,3.099e-02,3.151e-02,3.187e-02,3.205e-02,
    3.205e-02,3.187e-02,3.151e-02,3.099e-02,3.029e-02,2.945e-02,2.846e-02,2.734e-02,2.611e-02,2.479e-02,
    2.338e-02,2.191e-02,2.039e-02,1.885e-02,1.73e-02,1.576e-02,1.425e-02,1.277e-02,1.135e-02,9.987e-03,
    8.7e-03,7.494e-03,6.375e-03,5.349e-03,4.417e-03,3.581e-03,2.839e-03,2.189e-03,1.629e-03,1.152e-03,
    7.546e-04,4.296e-04,1.706e-04,-2.957e-05,-1.78e-04,-2.818e-04,-3.478e-04,-3.827e-04,-3.927e-04,-3.833e-04,
    -3.598e-04,-3.264e-04,-2.871e-04,-2.451e-04,-2.029e-04,-1.624e-04,-1.252e-04,-9.222e-05,-6.4e-05,-4.073e-05
};

/* 80 Tap Low Pass Kaiser                                                     */
/* Finite Impulse Response                                                    */
/* Sample Frequency = 800.0 Hz                                                */
/* Standard Form                                                              */
/* Arithmetic Precision = 4 Digits                                            */
/*                                                                            */
/* Pass Band Frequency = 15.00 Hz                                             */
/*                                                                            */
/* Kaiser Constant = 4.500                                                    */
/*                                                                            */
static float const znumF800L15[80] = 
{
        -4.621e-04,-5.935e-04,-7.305e-04,-8.666e-04,-9.942e-04,-1.104e-03,-1.185e-03,-1.227e-03,-1.217e-03,-1.14e-03,
        -9.851e-04,-7.373e-04,-3.84e-04,8.72e-05,6.873e-04,1.426e-03,2.309e-03,3.343e-03,4.53e-03,5.867e-03,
        7.352e-03,8.975e-03,1.073e-02,1.259e-02,1.455e-02,1.658e-02,1.867e-02,2.078e-02,2.288e-02,2.495e-02,
        2.695e-02,2.887e-02,3.065e-02,3.229e-02,3.375e-02,3.5e-02,3.603e-02,3.682e-02,3.735e-02,3.762e-02,
        3.762e-02,3.735e-02,3.682e-02,3.603e-02,3.5e-02,3.375e-02,3.229e-02,3.065e-02,2.887e-02,2.695e-02,
        2.495e-02,2.288e-02,2.078e-02,1.867e-02,1.658e-02,1.455e-02,1.259e-02,1.073e-02,8.975e-03,7.352e-03,
        5.867e-03,4.53e-03,3.343e-03,2.309e-03,1.426e-03,6.873e-04,8.72e-05,-3.84e-04,-7.373e-04,-9.851e-04,
        -1.14e-03,-1.217e-03,-1.227e-03,-1.185e-03,-1.104e-03,-9.942e-04,-8.666e-04,-7.305e-04,-5.935e-04,-4.621e-04
    };


/* 80 Tap Low Pass Kaiser                                                     */
/* Finite Impulse Response                                                    */
/* Sample Frequency = 800.0 Hz                                                */
/* Standard Form                                                              */
/* Arithmetic Precision = 7 Digits                                            */
/*                                                                            */
/* Pass Band Frequency = 15.00 Hz                                             */
/*                                                                            */
/* Kaiser Constant = 7.000                                                    */
/*                                                                            */
static float const znumF800L15K7[80] = {
        -5.105312e-05,-8.847085e-05,-1.354511e-04,-1.911499e-04,-2.535255e-04,-3.190955e-04,-3.827238e-04,-4.374525e-04,-4.743977e-04,-4.827249e-04,
        -4.497168e-04,-3.609445e-04,-2.005482e-04,4.837149e-05,4.032563e-04,8.814979e-04,1.499765e-03,2.273276e-03,3.215035e-03,4.335076e-03,
        5.639735e-03,7.130993e-03,8.805917e-03,1.065624e-02,1.266812e-02,1.482203e-02,1.709293e-02,1.945057e-02,2.186007e-02,2.428263e-02,
        2.667651e-02,2.899803e-02,3.120286e-02,3.324719e-02,3.50891e-02,3.668981e-02,3.801491e-02,3.90354e-02,3.972868e-02,4.007926e-02,
        4.007926e-02,3.972868e-02,3.90354e-02,3.801491e-02,3.668981e-02,3.50891e-02,3.324719e-02,3.120286e-02,2.899803e-02,2.667651e-02,
        2.428263e-02,2.186007e-02,1.945057e-02,1.709293e-02,1.482203e-02,1.266812e-02,1.065624e-02,8.805917e-03,7.130993e-03,5.639735e-03,
        4.335076e-03,3.215035e-03,2.273276e-03,1.499765e-03,8.814979e-04,4.032563e-04,4.837149e-05,-2.005482e-04,-3.609445e-04,-4.497168e-04,
        -4.827249e-04,-4.743977e-04,-4.374525e-04,-3.827238e-04,-3.190955e-04,-2.535255e-04,-1.911499e-04,-1.354511e-04,-8.847085e-05,-5.105312e-05
    };


/****************************************************************************************************
 *  通道参数
 ***************************************************************************************************/
/*
 *  滤波参数
 */
static float FastFirBuffScaler[100]		= {0};
static float FastFirBuffPreAxle1[80]		= {0};
static float FastFirBuffPreAxle2[80]		= {0};
static float FastFirBuffBackAxle[80]		= {0};

static sFirFilter FirScaler;
static sFirFilter FirPreAxle1;
static sFirFilter FirPreAxle2;
static sFirFilter FirBackAxle;
/*
 *  标准化参数
 */
#define     STANDPARAMVALUE     ((float) 0.046566)
//#define     STANDPARAMVALUE     ((float) 1.0f)
static sStandlize StandParam;
/*
 *  归一化参数
 */
FitPoint FitPointScaler[3];
FitPoint FitPointPreAxle1[3];
FitPoint FitPointPreAxle2[3];
FitPoint FitPointBackAxle[3];

sLineFit  LineFitScaler;
sLineFit  LineFitPreAxle1;
sLineFit  LineFitPreAxle2;
sLineFit  LineFitBackAxle;

/*
 *  动态零位参数
 */
static sZeroTrace  DyncZeroScaler;
static sZeroTrace  DyncZeroPreAxle1;
static sZeroTrace  DyncZeroPreAxle2;
static sZeroTrace  DyncZeroBackAxle;


/****************************************************************************************************
 *  秤台静态参数
 ***************************************************************************************************/
/*
 *  静态处理
 */
static sDigSample   SampleA;
/*
 *  静态滤波
 */
static float FastFirBuffA[25] = {0};
static sFirFilter FirA;

/*
 *  稳定检测
 */
static sStateCheck StateCheckA;
/*
 *  零位跟踪
 */
static sZeroTrace   ZeroTraceA;


/****************************************************************************************************
 *  秤台动态参数
 ***************************************************************************************************/
/*
 *  动态处理
 */
sSoftAlexDyncProc	SCS_ZC1_DyncProc;
sAlexDyncProc		SCS_ZC23_DyncProc;
sAlexThreeDyncProc	SCS_ZC56_DyncProc;
sMode8AlexDyncProc	SCS_ZC7_DyncProc;		//重庆试点新方案

//秤台句柄
HScaler hs_Main = HSCALER_INVALID;

//4路AD值
static long adbuf[4] = {0};

//整秤静态重量
static volatile int32 Scaler_Weight = 0;
static volatile int32 Scaler_MaxWeight = 0;

struct _ADChanle 
{
	u8 scaler;		//秤台对应的ad通道
	u8 backaxle;
	u8 preaxle1;
	u8 preaxle2;
};

static int32 ADChanleNum = 4;
static struct _ADChanle ADChanle;
static float ZeroTrace_Range_Speed[] = {0, 0.1, 0.2, 0.5, 1.0, 2.0, 5.0};

#define ZEROTRACE_ARRAY_LEN (sizeof(ZeroTrace_Range_Speed)/sizeof(ZeroTrace_Range_Speed[0]))

static void Load_ADChanle(int32 type)
{
	ADChanle.scaler   = 0;
	ADChanle.backaxle = 1;
	ADChanle.preaxle1 = 2;
	ADChanle.preaxle2 = 3;

	debug(Debug_None, "SCS_ZC_%d,ADChanle=%d,AD:%d,%d,%d,%d\r\n", type+1, ADChanleNum, ADChanle.scaler,
		ADChanle.backaxle,ADChanle.preaxle1, ADChanle.preaxle2);
}

static void Set_ADChanle(void)
{
	SysMod mode = Get_System_Mode();

	Load_ADChanle(mode);
}

//判断AD通道故障，置故障标志
//2013-3-2 更改传感器正常后没有更新标志位的bug
static void Charge_ADChanle_Error(int chanle)
{
	if(hs_Main == 0)
		return;

	switch(chanle) {
		case PreAxle1Chanle:
			if(IsAdcError(hs_Main,ADChanle.preaxle1) != 0)
				Updata_AD_Status(ADChanle.preaxle1, 1);
			else 
				Updata_AD_Status(ADChanle.preaxle1, 0);
			break;

		case PreAxle2Chanle:
			if(IsAdcError(hs_Main,ADChanle.preaxle2) != 0)
				Updata_AD_Status(ADChanle.preaxle2, 1);
			else
				Updata_AD_Status(ADChanle.preaxle2, 0);
			break;

		case ScalerChanle:
			if(IsAdcError(hs_Main,ADChanle.scaler) != 0)
				Updata_AD_Status(ADChanle.scaler, 1);
			else
				Updata_AD_Status(ADChanle.scaler, 0);
			break;
			
		case BackAxleChanle:
			if(IsAdcError(hs_Main,ADChanle.backaxle) != 0)
				Updata_AD_Status(ADChanle.backaxle, 1);
			else
				Updata_AD_Status(ADChanle.backaxle, 0);
			break;
			
		default:
			break;
	}
}

//算法模块获取ad值回调函数
long adc_proc(int chanle)
{
	return adbuf[chanle];
}

//计量模式7轴回调函数
static AlexRecoder Mode7AxleInfo = {0};
int ZC7_Alex_Event(void* pADP)
{
	TaskMsg msg = {0};
	sMode8AlexDyncProc *pAxle = (sMode8AlexDyncProc *)pADP;
	uint32 speed = 0;
	
	if(pAxle == NULL) 
	    return -1;
	
	if(pAxle->nAlexNo == 2 || pAxle->nAlexNo == 1) 
	{
		if(pAxle->nAlexINorRemove) 
		{
			msg.msg_id = Msg_PreAlex_Add;
			Mode7AxleInfo.direct = 1;
		} 
		else 
		{
			msg.msg_id = Msg_PreAlex_Remove;
			Mode7AxleInfo.direct = -1;
		}	

		//只有前轴更新车速
		speed = (uint32)(pAxle->fSpeed * 10);
		Set_CarSpeed(speed);
	} 
	else if(pAxle->nAlexNo == 3) 
	{
		if(pAxle->nAlexINorRemove)
		{
			msg.msg_id = Msg_BackAlex_Add;
			Mode7AxleInfo.direct = 1;
		}
		else
		{
			msg.msg_id = Msg_BackAlex_Remove;
			Mode7AxleInfo.direct = -1;
		}
	}
	
	Mode7AxleInfo.AlexKg = pAxle->nAlexWet;
	Mode7AxleInfo.AlexKgRef = pAxle->nAlexMaxWet;
	Mode7AxleInfo.AlexDValue = pAxle->nAlexDValue;
	Mode7AxleInfo.nTimePulse = pAxle->nAlexPulseWith;
	Mode7AxleInfo.bDouble = pAxle->bDouble;

	msg.msg_ctx = (u32)&Mode7AxleInfo;
	Task_QPost(&BusinessMsgQ, &msg);
	
	return 0;
}


//轴识别回调函数
int ZC56_Alex_Event(void* pADP)
{
	static AlexRecoder axleinfo = {0};
	TaskMsg msg = {0};
	sAlexThreeDyncProc *pAxle = (sAlexThreeDyncProc *)pADP;
	uint32 speed = 0;

	if(pAxle == NULL) 
	    return -1;
	
	if(pAxle->nAlexNo == 2 || pAxle->nAlexNo == 1) 
	{
		if(pAxle->nAlexINorRemove) 
		{
			msg.msg_id = Msg_PreAlex_Add;
			axleinfo.direct = 1;
		} 
		else 
		{
			msg.msg_id = Msg_PreAlex_Remove;
			axleinfo.direct = -1;
		}


		speed = (uint32)(pAxle->fSpeed * 10);
		Set_CarSpeed(speed);
	} 
	else if(pAxle->nAlexNo == 3) 
	{
		if(pAxle->nAlexINorRemove)
		{
			msg.msg_id = Msg_BackAlex_Add;
			axleinfo.direct = 1;				
		}
		else
		{
			msg.msg_id = Msg_BackAlex_Remove;
			axleinfo.direct = -1;				
		}
	}
	

	axleinfo.AlexKg = pAxle->nAlexWet;	
	axleinfo.AlexKgRef = pAxle->nAlexMaxWet;
	axleinfo.nTimePulse = pAxle->nAlexPulseWith;

		
	msg.msg_ctx = (u32)&axleinfo;
	Task_QPost(&BusinessMsgQ, &msg);
		
	return 0;
}


//轴识别回调函数
int ZC23_Alex_Event(void* pADP)
{
	static AlexRecoder axleinfo = {0};
	TaskMsg msg = {0};
	sAlexDyncProc *pAxle = (sAlexDyncProc *)pADP;
	uint32 speed = 0;
	
	if(pAxle == NULL) 
	    return -1;
	
	if(pAxle->nAlexNo == 2 || pAxle->nAlexNo == 1) 
	{
		if(pAxle->nAlexINorRemove) 
		{
			msg.msg_id = Msg_PreAlex_Add;
			axleinfo.direct = 1;
		} 
		else 
		{
			msg.msg_id = Msg_PreAlex_Remove;
			axleinfo.direct = -1;
		}

		speed = (uint32)(pAxle->fSpeed * 10);
		Set_CarSpeed(speed);
	} 
	else if(pAxle->nAlexNo == 3)
	{
		if(pAxle->nAlexINorRemove)
		{
			msg.msg_id = Msg_BackAlex_Add;
			axleinfo.direct = 1;
		}
		else
		{
			msg.msg_id = Msg_BackAlex_Remove;
			axleinfo.direct = -1;
		}
	}
	
	axleinfo.AlexKg = pAxle->nAlexWet;
	axleinfo.AlexKgRef = pAxle->nAlexMaxWet;
	axleinfo.nTimePulse = pAxle->nAlexPulseWith;

	msg.msg_ctx = (u32)&axleinfo;
	Task_QPost(&BusinessMsgQ, &msg);
	
	return 0;
}

//内蒙模式
int ZC1_Alex_Event(void *pADP)
{
    static AlexRecoder axleinfo = {0};
	TaskMsg msg = {0};
	sSoftAlexDyncProc *pAxle = (sSoftAlexDyncProc *)pADP;

	if(pAxle != NULL)
	    return -1;
	    
	if(pAxle->nAlexINorRemove) {
        msg.msg_id = Msg_PreAlex_Add;
		axleinfo.direct = 1;
	} else {
	    msg.msg_id = Msg_PreAlex_Remove;
		axleinfo.direct = -1;
	}

	axleinfo.AlexKg = pAxle->nAlexWet;
	axleinfo.AlexKgRef = pAxle->nAlexMaxWet;
	axleinfo.nTimePulse = pAxle->nAlexPulseWith;
			
	msg.msg_ctx = (u32)&axleinfo;
	Task_QPost(&BusinessMsgQ, &msg);
	
	return 0;
}

static void ADChanle_Register(u8 chanle, sFirFilter *fir, sLineFit *line, sZeroTrace *zero)
{
	//注册ad采样回调函数
	RegisterAdcGetProc(hs_Main, chanle, NULL,adc_proc, NULL);
	//注册ADC通道故障检查函数
	RegisterAdcCheckProc(hs_Main, chanle, NULL, ADCValueCheck, NULL);	
	//注册AD通道ADC初级滤波
	RegisterAdcFilterProc(hs_Main, chanle, FirFilterInit ,FirFilterDynamic, fir);
	//注册标准化处理函数
	RegisterAdcStandlizeProc(hs_Main, chanle, NULL, Standlize,&StandParam);
	//注册归一化处理函数
	RegisterAdcNormalizeProc(hs_Main, chanle, NULL, LineFit, line);	
	//AD通道零点跟踪处理函数
	RegisterAdcZeroTraceProc(hs_Main, chanle, NULL, ZeroTrace, SetZero, zero);
}

static char ScalerRegister(void)
{
	ERR_WEIGHT exitCode;
	SysMod mode = Get_System_Mode();
	
	memset((char*)&ZeroTraceA,0,sizeof(sZeroTrace));
	memset((char*)&StateCheckA,0,sizeof(sStateCheck));

	//标准化处理函数参数
	StandParam.fParam = STANDPARAMVALUE;

	////=====================================================================================
	/*******************************秤台通道注册函数**********************************/
	//Scaler通道ADC初级滤波
	FirScaler.bInit = 0;
	FirScaler.nLen  = 100;
	FirScaler.pDelay= FastFirBuffScaler;
	FirScaler.pZNum = znumF800L12;
	
	//归一化处理
	LineFitScaler.pFitPoint = FitPointScaler;
	LineFitScaler.nFitCount = 1;
	FitPointScaler[0].x = gWetPar.ScalerZeroAD;
	FitPointScaler[0].y = 0;
	FitPointScaler[0].k = gWetPar.Scaler_K;

	//scaler通道零点跟踪
	DyncZeroScaler.fHandRange	= gWetPar.HandlZero * gWetPar.MaxWet / 100.0f;
	DyncZeroScaler.fPowerRange	= gWetPar.PowerZero * gWetPar.MaxWet / 100.0f;
	DyncZeroScaler.fTraceRange	= ZeroTrace_Range_Speed[gWetPar.ZeroTruckRang] * gWetPar.StaticDivValue;
	DyncZeroScaler.nTraceTime	= gWetPar.ZeroTruckTick;
	DyncZeroScaler.fTraceStep	= ZeroTrace_Range_Speed[gWetPar.ZeroTruckSpeed] * gWetPar.StaticDivValue;
	DyncZeroScaler.nPowerTime	= DYNAMIC_AD_RATE/2;
	DyncZeroScaler.fDrv = gWetPar.DynamicDivValue;

	ADChanle_Register(ADChanle.scaler, &FirScaler, &LineFitScaler, &DyncZeroScaler);

	////=====================================================================================
	/*******************************后轴识别器通道注册函数**********************************/
	//注册BackAxle通道ADC初级滤波
	FirBackAxle.bInit = 0;
	FirBackAxle.nLen  = 80;
	FirBackAxle.pDelay= FastFirBuffBackAxle;
	FirBackAxle.pZNum = znumF800L15;

	LineFitBackAxle.pFitPoint = FitPointBackAxle;
	LineFitBackAxle.nFitCount = 1;
	FitPointBackAxle[0].x = gWetPar.BackAxleZeroAD;
	FitPointBackAxle[0].y = 0;
	FitPointBackAxle[0].k = 1;

	//零点跟踪处理函数
	DyncZeroBackAxle.fHandRange	= 1000000;
	DyncZeroBackAxle.fPowerRange	= 1000000;
	DyncZeroBackAxle.fTraceRange	= 5.0f;
	DyncZeroBackAxle.fTraceStep   = 2.5f;
	DyncZeroBackAxle.nTraceTime	= DYNAMIC_AD_RATE;
	DyncZeroBackAxle.nPowerTime	= DYNAMIC_AD_RATE/2;
	DyncZeroBackAxle.fDrv = 1;
	
	ADChanle_Register(ADChanle.backaxle, &FirBackAxle, &LineFitBackAxle, &DyncZeroBackAxle);

	////=============================================================================================
	/*******************************前轴识别器第一路通道注册函数**********************************/
	//注册PreAxle1通道ADC初级滤波
	FirPreAxle1.bInit = 0;
	FirPreAxle1.nLen  = 80;
	FirPreAxle1.pDelay= FastFirBuffPreAxle1;
	FirPreAxle1.pZNum = znumF800L15K7;

	LineFitPreAxle1.pFitPoint = FitPointPreAxle1;
	LineFitPreAxle1.nFitCount = 1;
	FitPointPreAxle1[0].x = gWetPar.PreAxle1ZeroAD;		//轴没有k系数
	FitPointPreAxle1[0].y = 0;
	FitPointPreAxle1[0].k = gWetPar.PreAxle1_K;
	
	//PreAxle1通道零点跟踪处理函数
	DyncZeroPreAxle1.fHandRange	= 1000000;
	DyncZeroPreAxle1.fPowerRange	= 1000000;
	DyncZeroPreAxle1.fTraceRange	= 5;  //5*gWetPar.StaticDivValue;
	DyncZeroPreAxle1.fTraceStep   = 2.5; //1*gWetPar.StaticDivValue;
	DyncZeroPreAxle1.nTraceTime	= DYNAMIC_AD_RATE;
	DyncZeroPreAxle1.nPowerTime	= DYNAMIC_AD_RATE/2;
	DyncZeroPreAxle1.fDrv = 1;
	
	ADChanle_Register(ADChanle.preaxle1, &FirPreAxle1, &LineFitPreAxle1, &DyncZeroPreAxle1);
	
	////=============================================================================================
	/*******************************前轴识别器第二路通道注册函数**********************************/
	//注册ADC初级滤波
	FirPreAxle2.bInit = 0;
	FirPreAxle2.nLen  = 80;
	FirPreAxle2.pDelay= FastFirBuffPreAxle2;
	FirPreAxle2.pZNum = znumF800L15;

	LineFitPreAxle2.pFitPoint = FitPointPreAxle2;
	LineFitPreAxle2.nFitCount = 1;
	FitPointPreAxle2[0].x = gWetPar.PreAxle2ZeroAD;		//零点
	FitPointPreAxle2[0].y = 0;		//分段
	FitPointPreAxle2[0].k = gWetPar.PreAxle2_K;		//k系数
	
	//零点跟踪处理函数
	DyncZeroPreAxle2.fHandRange	= 1000000;
	DyncZeroPreAxle2.fPowerRange	= 1000000;
	DyncZeroPreAxle2.fTraceRange	= 5.0f;			//e/s
	DyncZeroPreAxle2.fTraceStep   = 2.5f;
	DyncZeroPreAxle2.nTraceTime	= DYNAMIC_AD_RATE;			//次/s
	DyncZeroPreAxle2.nPowerTime	= DYNAMIC_AD_RATE/2;
	DyncZeroPreAxle2.fDrv = 1;	
	
	ADChanle_Register(ADChanle.preaxle2, &FirPreAxle2, &LineFitPreAxle2, &DyncZeroPreAxle2);
	
	//=============================================================================
	//注册静态秤相关函数=====================================================
	RegisterScalerStaticTotalProc(hs_Main, NULL, StaticProc, NULL);

	//静态采用函数
	SampleA.nCycle = WEIGHT_CYCLE;
	RegisterScalerStaticSampleProc(hs_Main,NULL,DigSample,(void *)&SampleA);

	//静态滤波注册
	FirA.pZNum = znumF25L1;
	FirA.pDelay = FastFirBuffA;
	FirA.nLen   = 25;
	FirA.bInit  = 0;
	RegisterScalerStaticFilterProc(hs_Main,FirFilterInit,FirFilterStatic,(void *)&FirA);


	//静态零位跟踪函数	
	ZeroTraceA.fHandRange	= gWetPar.HandlZero * gWetPar.MaxWet / 100.0f;
	ZeroTraceA.fPowerRange	= gWetPar.PowerZero * gWetPar.MaxWet / 100.0f;
	ZeroTraceA.fTraceRange	= DyncZeroScaler.fTraceRange;
	ZeroTraceA.nTraceTime	= (int)(gWetPar.ZeroTruckTick/WEIGHT_CYCLE);
	ZeroTraceA.fTraceStep  = DyncZeroScaler.fTraceStep;
	ZeroTraceA.fDrv = gWetPar.StaticDivValue;
#ifdef _SIMULATION_
	ZeroTraceA.nPowerTime	= 2;
#else
	ZeroTraceA.nPowerTime	= 3*STATIC_AD_RATE;
#endif

	RegisterScalerStaticZeroTraceProc(hs_Main,NULL,ZeroTrace,SetZero,(void *)&ZeroTraceA);
	
	//静态状态判断函数
	StateCheckA.fSteadyRange = 0.5*gWetPar.StaticDivValue;
	StateCheckA.fZeroRange = 0.5*gWetPar.StaticDivValue;
	StateCheckA.nTimeRange = STATIC_AD_RATE;
	RegisterScalerStaticStateCheckProc(hs_Main,NULL,StateCheck, (void *)&StateCheckA);
	
	//=============================================================================
	//注册动态秤相关函数=====================================================
	memset((char *)&SCS_ZC56_DyncProc, 0, sizeof(sAlexThreeDyncProc));
	SCS_ZC56_DyncProc.iPowerOnDelay = DYNAMIC_AD_RATE / 4 + 2;

	memset((char*)&SCS_ZC23_DyncProc,0,sizeof(sAlexDyncProc));
	SCS_ZC23_DyncProc.iPowerOnDelay = DYNAMIC_AD_RATE / 4 + 2;

	memset((char*)&SCS_ZC1_DyncProc, 0, sizeof(sSoftAlexDyncProc));
	SCS_ZC1_DyncProc.iPowerOnDelay = DYNAMIC_AD_RATE / 4 + 2;

	memset((char*)&SCS_ZC7_DyncProc,0,sizeof(sMode7AlexDyncProc));
	SCS_ZC7_DyncProc.iPowerOnDelay = DYNAMIC_AD_RATE / 4 + 2;

	switch(mode) {
		case SCS_ZC_1:
	        SCS_ZC1_DyncProc.pDecb = ZC1_Alex_Event;
	        RegisterScalerDyncProc(hs_Main, SoftAlexDyncProcInit, SoftAlexDyncProc, (void *)&SCS_ZC1_DyncProc);
			break;

		case SCS_ZC_2:
		case SCS_ZC_3:
	    	SCS_ZC23_DyncProc.fWetUp = gWetPar.PreAxleUpWet;
	    	SCS_ZC23_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
	    	SCS_ZC23_DyncProc.pADP = ZC23_Alex_Event;
			SCS_ZC23_DyncProc.mSpeedLen = gWetPar.AxleLength;
	    	RegisterScalerDyncProc(hs_Main,AlexDyncProcInit,AlexDyncProc,(void *)&SCS_ZC23_DyncProc);
			break;

		case SCS_ZC_5:
		case SCS_ZC_6:
			SCS_ZC56_DyncProc.fWetUp   = gWetPar.PreAxleUpWet;
			SCS_ZC56_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
			SCS_ZC56_DyncProc.fDownAlexWetUp   = gWetPar.BackAxleUpWet;
			SCS_ZC56_DyncProc.fDownAlexWetDown = gWetPar.BackAxleDownWet;
			SCS_ZC56_DyncProc.mSpeedLen = gWetPar.AxleLength;
			SCS_ZC56_DyncProc.pADP = ZC56_Alex_Event;
	    	RegisterScalerDyncProc(hs_Main,AlexDyncProcThreeInit,AlexDyncThreeProc,(void *)&SCS_ZC56_DyncProc);
			break;

		case SCS_ZC_7:
	    	SCS_ZC7_DyncProc.fWetUp = gWetPar.PreAxleUpWet;
	    	SCS_ZC7_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
	    	SCS_ZC7_DyncProc.fLeaveWetUp = gWetPar.BackAxleUpWet;
	    	SCS_ZC7_DyncProc.fLeaveWetDown = gWetPar.BackAxleDownWet;
	    	SCS_ZC7_DyncProc.pADP = ZC7_Alex_Event;
			SCS_ZC7_DyncProc.mSpeedLen = gWetPar.AxleLength;
			SCS_ZC7_DyncProc.fDyncK = gWetPar.DynamicK;
//	    	RegisterScalerDyncProc(hs_Main,Mode7AlexDyncProcInit,Mode7AlexDyncProc,(void *)&SCS_ZC7_DyncProc);
			RegisterScalerDyncProc(hs_Main,Mode8AlexDyncProcInit,Mode8AlexDyncProc,(void *)&SCS_ZC7_DyncProc);
			break;

		default:
			break;
    }
    
	exitCode = ScalerStart(hs_Main);
	if(exitCode != ERR_SUCCESS)
	{
		debug(Debug_Error, "MainScaler Error: %s\r\n",GetError(exitCode));
		return FALSE;
	}

	return TRUE;
}

///主称台
static int InitMainScaler(void)
{
	ERR_WEIGHT exitCode;

	Set_ADChanle();
	hs_Main = ScalerCreate(ADChanleNum,  &exitCode);
	
	if(hs_Main == HSCALER_INVALID)
	{
		debug(Debug_Error, "CreateMainScaler Failed,Exit\r\n");
		return FALSE;
	}
	
	return ScalerRegister();
}

static void Update_Axle_Flag(void)
{
	SysMod mode = Get_System_Mode();
	DeviceStatus device = {0};
	BOOL flag = FALSE;
	
	switch(mode) 
	{
		case SCS_ZC_1:
			break;
			
		case SCS_ZC_2:
		case SCS_ZC_3:	
			flag = TRUE;
			device.bitValue.axle12_valid = SCS_ZC23_DyncProc.nUpFlag;
			break;
			
		case SCS_ZC_5:
		case SCS_ZC_6:
			flag = TRUE;
			device.bitValue.axle12_valid = SCS_ZC56_DyncProc.nUpFlag;
			break;

		case SCS_ZC_7:
			flag = TRUE;
			device.bitValue.axle12_valid = SCS_ZC7_DyncProc.nUpFlag;
			break;
			
		default:
			break;
	}

	device.bitValue.axle11_valid = Get_TyreDevice_Event();
	device.bitValue.axle2_valid = Get_TyreDevice_bDouble();

	if(flag)
		Update_AxleDevice_Status(device);

}

//开机发送系统参数,该函数只执行一次
void Send_SysParm_Info(char *sbuf)
{
	uint16 len = 0;
	TaskMsg msg = {0};
	
	len = sizeof(gWetParam);
	if(len > 320) len = 320;		//一个波形数据包的长度

//	Build_NetData_Pack(sbuf, len+1, Net_Data_Parm);
	Build_Rcmp_HeadPack(CMD_PARAM_DATA, (unsigned char *)sbuf, len);

	memcpy(sbuf + RMCP_HEAD_LEN, &gWetPar, sizeof(gWetParam));
	len += RMCP_HEAD_LEN;

	msg.msg_id = Msg_NetSend;
	msg.msg_ctx = (u32)sbuf;
	msg.msg_len = len;
	Task_QPost(&NetMsgQ, &msg);
}

static int32 sendbuf[4] = {0};
void Weight_Business(void *buf)
{
	ERR_WEIGHT exitCode;
	WetStatue wetpar = {0};
	static uint16 cnt = 0;
	static uint16 cnt1 = 0;
	static BOOL flag = FALSE;

	memcpy(adbuf, buf, 16);
	exitCode  = ScalerRun(hs_Main) ;
	if (ERR_SUCCESS != exitCode) 
		debug(Debug_Error, "Scaler[0x%x] run failed! Error:%s\r\n",hs_Main,GetError(exitCode));

 	//更新滤波后整秤静态重量
	Scaler_Weight = GetScalerStaticZeroTraceValue(hs_Main);
	if(Scaler_Weight > Scaler_MaxWeight) Scaler_MaxWeight = Scaler_Weight;

	//有网络连接才发送波形
	if(Get_NetStatus()) 
	{
		//波形发送内存满延时20ms后再发送
		if(!flag) 
		{
			cnt1++;
			if(cnt1 > 16) 
			{
				cnt1 = 0;
				flag = TRUE;
			}
		}

		Update_Axle_Flag();
    	sendbuf[0] = GetAdcChanleStandValue(hs_Main, 0);
    	sendbuf[1] = GetAdcChanleStandValue(hs_Main, 1);
    	sendbuf[2] = GetAdcChanleStandValue(hs_Main, 2); 	//Scaler_Weight;			
    	sendbuf[3] = GetAdcChanleStandValue(hs_Main, 3);	//adbuf[3];
    	if(flag) 
    	{
			if(!Net_Send_Wave(sendbuf))
				flag = FALSE;
    	}
    }

	//GUI界面显示的重量参数, 5ms更新一次
	cnt++;
	if(cnt >= 4) 
	{
		cnt = 0;
		wetpar.Wet = GetScalerStaticDrvValue(hs_Main);
		wetpar.PreAxle1AD = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle1);
		wetpar.ScalerAD= GetAdcChanleStandValue(hs_Main,ADChanle.scaler);
		wetpar.BackAxleAD = GetAdcChanleStandValue(hs_Main,ADChanle.backaxle);
		wetpar.PreAxle2AD = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle2);
		wetpar.ScalerStableFlag = IsScalerStaticSteady(hs_Main);
		wetpar.ScalerZeroFlag	= IsScalerStaticAtZero(hs_Main);
		
		
		
		Update_Scaler_WetPar(&wetpar);
	}

	//ad通道故障信息判断
	Charge_ADChanle_Error(PreAxle1Chanle);
	Charge_ADChanle_Error(ScalerChanle);
	Charge_ADChanle_Error(BackAxleChanle);
	Charge_ADChanle_Error(PreAxle2Chanle);	
}

int Weight_Init(void)
{
 	if(FALSE == InitMainScaler())
		return -1;

	//设置分度值
	SetScalerStaticDrv(hs_Main, gWetPar.StaticDivValue);
	SetScalerDyncDrv(hs_Main, gWetPar.DynamicDivValue);
	//EnableAdcFreqDiv(hs_Main, 2, 1, 6);
	return 0;
}

uint8 Check_K_Value(float k)
{
	uint8 ret = KERR_NONE;
	
	if(k > 30.0f) 
	{
		ret = KERR_MAX;
	} 
	else if(k < 0.000001f) 
	{
		ret = KERR_MIN;
	}

	return ret;
}


static const char *calib_str[] = {"Scaler", "BackAxle", "PreAxle1", "PreAxle2"};

//计算SCS_ZC_1模式AD通道K系数
//解方程组
//AD11*K1 + AD12*K2 = AD21*K1 + AD22*K2
//K1 === 1
BOOL SCSZC1_Cal_K(int inWet, int chanle, int place)
{
	static float CalibFirAD11 = 0.0;
	static float CalibFirAD12 = 0.0;
	static float CalibSecAD21 = 0.0;
	static float CalibSecAD22 = 0.0;
	static float PreAxle1_K;
	static float Scaler_K;
	static int32 PreWet = 0;
	static int32 BackWet = 0;
	static uint8 flag1 = 0;
	static uint8 flag2 = 0;
	BOOL ret = FALSE;
	float div = 0;

	if(chanle > PreAxle2Chanle || place > 3 || place < 1)
		return FALSE;
	
	debug(Debug_Business, "Calibration Calc K value,AD chanle:%s, place:%d\r\n", calib_str[chanle], place);
	do {
		if(place == 1 && chanle == PreAxle1Chanle) {
			CalibFirAD11 = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle1);	
			CalibFirAD11 = CalibFirAD11 - gWetPar.PreAxle1ZeroAD;

			CalibFirAD12 = GetAdcChanleStandValue(hs_Main,ADChanle.scaler);
			CalibFirAD12 = CalibFirAD12 - gWetPar.ScalerZeroAD;
			ret = TRUE;
			flag1 = 1;
			PreWet = inWet;
			break;
		} 

		if(place == 2 && chanle == ScalerChanle) {
			CalibSecAD21 = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle1);	
			CalibSecAD21 = CalibSecAD21 - gWetPar.PreAxle1ZeroAD;

			CalibSecAD22 = GetAdcChanleStandValue(hs_Main,ADChanle.scaler);
			CalibSecAD22 = CalibSecAD22 - gWetPar.ScalerZeroAD;
			ret = TRUE;
			flag2 = 1;
			BackWet = inWet;
			break;
		}

		if(place == 3) {
			CalibFirAD11 = GetScalerStaticFilterValue(hs_Main);
			div = inWet / CalibFirAD11;	
			if(Check_K_Value(div) != KERR_NONE) {
			    ret = FALSE;
			    break;
			}
			gWetPar.Total_K = div;   
			Param_Write(GET_F32_OFFSET(ParamTotal_K), &gWetPar.Total_K, sizeof(float));

            //标1, 2, 3
			if(flag1 && flag2) {
				flag1 = 0;
				flag2 = 0;
				div = abs(CalibFirAD12 - CalibSecAD22);
				if(div != 0.000001f) {
					PreAxle1_K = 1;
					Scaler_K = abs(CalibSecAD21 - CalibFirAD11) / div;
				} else {
					ret = FALSE;
					break;
				}
				
				PreAxle1_K = PreAxle1_K * gWetPar.Total_K;
				Scaler_K   = Scaler_K * gWetPar.Total_K;
			} 

            //标1, 3
			if(flag1 && !flag2) {
			    flag1 = 0;
			    PreAxle1_K = 1;
			    Scaler_K = (PreWet - CalibFirAD11) / CalibFirAD12;
			}

            //标2, 3
			if(!flag1 && flag2) {
			    flag2 = 0;
			    PreAxle1_K = 1;
			    Scaler_K = (BackWet - CalibSecAD21) / CalibSecAD22;
			}
			
            //标 3
            if(!flag1 && !flag2) {
    			PreAxle1_K = gWetPar.PreAxle1_K * gWetPar.Total_K;
    			Scaler_K = gWetPar.Scaler_K * gWetPar.Total_K;
            }
            
			if((Check_K_Value(PreAxle1_K) != KERR_NONE)	|| (Check_K_Value(Scaler_K) != KERR_NONE)) {
				ret = FALSE;
				break;
			}

			gWetPar.PreAxle1_K = gWetPar.PreAxle1_K * gWetPar.Total_K;
			gWetPar.Scaler_K = gWetPar.Scaler_K * gWetPar.Total_K;
			Param_Write(GET_F32_OFFSET(ParamPreAxle1Zero), &gWetPar.PreAxle1ZeroAD, sizeof(float));
			Param_Write(GET_F32_OFFSET(ParamScalerZero), &gWetPar.ScalerZeroAD, sizeof(float));

			//归一化处理函数
			FitPointPreAxle1[0].x = gWetPar.PreAxle1ZeroAD;
			FitPointPreAxle1[0].y = 0;
			FitPointPreAxle1[0].k = gWetPar.PreAxle1_K;

			//归一化处理函数
			FitPointScaler[0].x = gWetPar.ScalerZeroAD;
			FitPointScaler[0].y = 0;
			FitPointScaler[0].k = gWetPar.Scaler_K;

			ret = TRUE;
			break;
		}
	} while(0);

	return ret;
}

//保存AD通道零点,标定零点
BOOL Weight_Calc_Zero(void)
{
	BOOL ret = FALSE;
	
	gWetPar.PreAxle1ZeroAD = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle1);	
	ret = Param_Write(GET_F32_OFFSET(ParamPreAxle1Zero), &gWetPar.PreAxle1ZeroAD, sizeof(float));
	FitPointPreAxle1[0].x = gWetPar.PreAxle1ZeroAD;
	
	gWetPar.PreAxle2ZeroAD = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle2);
	ret &= Param_Write(GET_F32_OFFSET(ParamPreAxle2Zero), &gWetPar.PreAxle2ZeroAD, sizeof(float));
	FitPointPreAxle2[0].x = gWetPar.PreAxle2ZeroAD;
	
	gWetPar.ScalerZeroAD = GetAdcChanleStandValue(hs_Main,ADChanle.scaler);
	ret &= Param_Write(GET_F32_OFFSET(ParamScalerZero), &gWetPar.ScalerZeroAD, sizeof(float));
	FitPointScaler[0].x = gWetPar.ScalerZeroAD;

	gWetPar.BackAxleZeroAD = GetAdcChanleStandValue(hs_Main,ADChanle.backaxle); 		
	ret &= Param_Write(GET_F32_OFFSET(ParamBackAxleZero), &gWetPar.BackAxleZeroAD, sizeof(float));
	FitPointBackAxle[0].x = gWetPar.BackAxleZeroAD;

	return ret;
}

//计算AD通道K系数
uint8 Weight_Cal_K(int inWet, int chanle)
{
	float mStandValue = 0.0f;
	float tmpk = 0.0f;
	uint8 ret = KERR_NONE;

	if(chanle > PreAxle2Chanle)
		return KERR_ADERR;

	switch(chanle) {
		case PreAxle1Chanle:
			mStandValue = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle1);
				
			if((mStandValue - gWetPar.PreAxle1ZeroAD) != 0.000000f)
				tmpk = inWet / (mStandValue  - gWetPar.PreAxle1ZeroAD);	

			ret = Check_K_Value(tmpk);
			if(ret != KERR_NONE)
				break;
			
			gWetPar.PreAxle1_K = tmpk;
			if(!Param_Write(GET_F32_OFFSET(ParamPreAxle1_K), &gWetPar.PreAxle1_K, sizeof(float)))
				ret = KERR_PARM;
			break;

		case PreAxle2Chanle:
			mStandValue = GetAdcChanleStandValue(hs_Main,ADChanle.preaxle2);
			if((mStandValue - gWetPar.PreAxle2ZeroAD) != 0.000000f)
				tmpk = inWet / (mStandValue  - gWetPar.PreAxle2ZeroAD);	
				
			ret = Check_K_Value(tmpk);
			if(ret != KERR_NONE)
				break;

			gWetPar.PreAxle2_K = tmpk;
			if(!Param_Write(GET_F32_OFFSET(ParamPreAxle2_K), &gWetPar.PreAxle2_K, sizeof(float)))
				ret = KERR_PARM;
			break;

		case ScalerChanle:
			mStandValue = GetAdcChanleStandValue(hs_Main,ADChanle.scaler);
			if((mStandValue - gWetPar.ScalerZeroAD) != 0.000000f)
				tmpk = inWet / (mStandValue  - gWetPar.ScalerZeroAD);

			ret = Check_K_Value(tmpk);
			if(ret != KERR_NONE)
				break;

			gWetPar.Scaler_K = tmpk;

			//归一化处理函数
			FitPointScaler[0].x = gWetPar.ScalerZeroAD;
			FitPointScaler[0].y = 0;
			FitPointScaler[0].k = gWetPar.Scaler_K;
			if(!Param_Write(GET_F32_OFFSET(ParamScaler_K), &gWetPar.Scaler_K, sizeof(float)))
				ret = KERR_PARM;
			break;

		case BackAxleChanle:
			mStandValue = GetAdcChanleStandValue(hs_Main,ADChanle.backaxle);
			if((mStandValue - gWetPar.BackAxleZeroAD) != 0.000000f)
				tmpk = inWet / (mStandValue  - gWetPar.BackAxleZeroAD);
				
			ret = Check_K_Value(tmpk);
			if(ret != KERR_NONE)
				break;
			gWetPar.BackAxle_K = tmpk;
			
			if(!Param_Write(GET_F32_OFFSET(ParamBackAxle_K), &gWetPar.BackAxle_K, sizeof(float)))
				ret = KERR_PARM;
			break;

		default:
			break;
	}
	debug(Debug_Business, "Calibration Calc K,%s chanle\r\n", calib_str[chanle]);
	return ret;
}

//设置秤台动态系数
void Set_ScalerDyncStatic_K(float k)
{
	if(Check_K_Value(k) != KERR_NONE) return;
	
	gWetPar.DynamicK = k;
	Param_Write(GET_F32_OFFSET(ParamDyncK),&k,sizeof(float));
}

//设置秤台静态系数
int Set_ScalerStatic_K(float k)
{
	if(Check_K_Value(k) != KERR_NONE) return -1;
	
	gWetPar.Scaler_K = k;
	Param_Write(GET_F32_OFFSET(ParamScaler_K),&k,sizeof(float));
	FitPointScaler[0].k = gWetPar.Scaler_K;

	return 0;
}

int Set_3Axle_Static_K(float k)
{
	if(Check_K_Value(k) != KERR_NONE) return -1;
	
	gWetPar.PreAxle1_K = k;
	Param_Write(GET_F32_OFFSET(ParamPreAxle1_K),&k,sizeof(float));
	FitPointPreAxle1[0].k = gWetPar.PreAxle1_K;

	return 0;
}

int Set_4Axle_Static_K(float k)
{
	if(Check_K_Value(k) != KERR_NONE) return -1;
	
	gWetPar.PreAxle2_K = k;
	Param_Write(GET_F32_OFFSET(ParamPreAxle2_K),&k,sizeof(float));
	FitPointPreAxle2[0].k = gWetPar.PreAxle2_K;
    
    return 0;
}

void Scaler_Reset_Zero(void)
{
	if(hs_Main == 0)
		return;

	ScalerStaticSetZero(hs_Main, 0);
	ScalerDyncSetZero(hs_Main, 0);
}

//秤台静态置零
void Scaler_Set_Zero(BOOL bAuto)
{
	if(hs_Main == 0)
		return;
	
	//自动置零时要看零位跟踪是否开启
	if(bAuto == TRUE)
	{
		if(gWetPar.ZeroTruckTick == 0) return;
	}
	
	debug(Debug_Notify, "秤台置零\r\n");
	ScalerStaticSetZero(hs_Main, 1);
	ScalerDyncSetZero(hs_Main, 1);
}

void WeightFregDiv(int bEnable, unsigned short nDiv)
{
	if(hs_Main == 0) return;
		
	EnableScalerFreqDiv(hs_Main, bEnable, nDiv);	
}

//设置秤台动态分度值
int Set_Scaler_DynamicDrv(uint8 drv)
{
	if(hs_Main == 0)
		return 0;
		
	return SetScalerDyncDrv(hs_Main, drv);
}

//设置秤台静态分度值
int Set_Scaler_StaticDrv(uint8 drv)
{
	if(hs_Main == 0)
		return 0;
		
	return SetScalerStaticDrv(hs_Main, drv);
}

//获取主秤台秤台动态重量
int32 Get_Dynamic_Weight(void)
{
	if(hs_Main == HSCALER_INVALID) return 0;

	return (int32)GetScalerDyncValue(hs_Main);
}

int32 Get_Static_Weight(void)
{
	return Scaler_Weight;
}

int32 Get_Static_MaxWeight(void)
{
	return Scaler_MaxWeight;
}

void Clear_ScalerMaxWeight(void)
{
	Scaler_MaxWeight = 0;
}
	
//模拟秤台重量
void Set_Static_Weight(int32 wet)
{
	debug(Debug_Business, "设置秤台重量 %d\r\n", wet);
    Scaler_Weight = wet;
}

void Init_Wet_Par(void)
{
	SysMod mode = Get_System_Mode();
	
	if(hs_Main == NULL) return;

	//ZeroTrace_Range_Speed数组越界保护
	if(gWetPar.ZeroTruckRang >= ZEROTRACE_ARRAY_LEN) gWetPar.ZeroTruckRang = 0;
	if(gWetPar.ZeroTruckSpeed >= ZEROTRACE_ARRAY_LEN) gWetPar.ZeroTruckSpeed = 0;
		
	//scaler通道零点跟踪处理函数
	DyncZeroScaler.fHandRange	= gWetPar.HandlZero * gWetPar.MaxWet / 100.0f;
	DyncZeroScaler.fPowerRange	= gWetPar.PowerZero * gWetPar.MaxWet / 100.0f;
	DyncZeroScaler.fTraceRange	= ZeroTrace_Range_Speed[gWetPar.ZeroTruckRang]*gWetPar.StaticDivValue;
	DyncZeroScaler.nTraceTime	= gWetPar.ZeroTruckTick;
	DyncZeroScaler.fTraceStep	= ZeroTrace_Range_Speed[gWetPar.ZeroTruckSpeed]*gWetPar.StaticDivValue;
	DyncZeroScaler.fDrv = gWetPar.DynamicDivValue;

	//静态零位跟踪函数	
	ZeroTraceA.fHandRange	= DyncZeroScaler.fHandRange;
	ZeroTraceA.fPowerRange	= DyncZeroScaler.fPowerRange;
	ZeroTraceA.fTraceRange	= DyncZeroScaler.fTraceRange;
	ZeroTraceA.nTraceTime	= (int)(gWetPar.ZeroTruckTick/WEIGHT_CYCLE);
	ZeroTraceA.fTraceStep  = DyncZeroScaler.fTraceStep;
	ZeroTraceA.fDrv = gWetPar.StaticDivValue;

	debug(Debug_Business, "ZeroTrace Range:%d, Time:%d, Speed:%d\r\n", (u32)DyncZeroScaler.fTraceRange,
		(u32)DyncZeroScaler.nTraceTime, (u32)DyncZeroScaler.fTraceStep);


	//修改轴相关参数
	switch(mode) {
		case SCS_ZC_1:
			break;

		case SCS_ZC_2:
		case SCS_ZC_3:
	    	SCS_ZC23_DyncProc.fWetUp = gWetPar.PreAxleUpWet;
	    	SCS_ZC23_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
			SCS_ZC23_DyncProc.mSpeedLen = gWetPar.AxleLength;
			break;

		case SCS_ZC_5:
		case SCS_ZC_6:
			SCS_ZC56_DyncProc.fWetUp   = gWetPar.PreAxleUpWet;
			SCS_ZC56_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
			SCS_ZC56_DyncProc.fDownAlexWetUp   = gWetPar.BackAxleUpWet;
			SCS_ZC56_DyncProc.fDownAlexWetDown = gWetPar.BackAxleDownWet;
			SCS_ZC56_DyncProc.mSpeedLen = gWetPar.AxleLength;
			break;

		case SCS_ZC_7:
	    	SCS_ZC7_DyncProc.fWetUp = gWetPar.PreAxleUpWet;
	    	SCS_ZC7_DyncProc.fWetDown = gWetPar.PreAxleDownWet;
	    	SCS_ZC7_DyncProc.fLeaveWetUp = gWetPar.BackAxleUpWet;
	    	SCS_ZC7_DyncProc.fLeaveWetDown = gWetPar.BackAxleDownWet;				
			SCS_ZC7_DyncProc.mSpeedLen = gWetPar.AxleLength;
			break;

		default:
			break;
    }

	return;
}

int32 Get_ADChanle_Num(void)
{
	return ADChanleNum;
}


