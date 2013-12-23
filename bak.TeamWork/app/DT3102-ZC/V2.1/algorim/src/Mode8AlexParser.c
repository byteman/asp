#include "Mode8AlexParser.h"
#include <math.h>  
#include <stdlib.h>

#include "axletype_business.h"
#include "device_info.h"
#include "FirFilter.h"
#include "axle_business.h"
#include "scszc_debug_info.h"
#include "Weight_Param.h"
#include "debug_info.h"

#define ALEXUPDOWN	150


/* 80 Tap Low Pass Root Raised Cosine                                         */
/* Finite Impulse Response                                                    */
/* Sample Frequency = 800.0 Hz                                                */
/* Standard Form                                                              */
/* Arithmetic Precision = 7 Digits                                            */
/*                                                                            */
/* Pass Band Frequency = 10.00 Hz                                             */
/* Alpha = 1.0000                                                             */
static float const znumF800L10R1[80] = {
        -2.157603e-03,-2.223231e-03,-2.235263e-03,-2.186788e-03,-2.071255e-03,-1.8826e-03,-1.615354e-03,-1.264759e-03,-8.268721e-04,-2.986579e-04,
        3.219224e-04,1.035839e-03,1.842935e-03,2.741877e-03,3.73013e-03,4.803945e-03,5.958361e-03,7.187231e-03,8.48326e-03,9.838064e-03,
        1.124225e-02,1.268549e-02,1.415667e-02,1.564394e-02,1.713493e-02,1.861682e-02,2.007654e-02,2.150091e-02,2.287679e-02,2.419129e-02,
        2.543187e-02,2.658654e-02,2.764402e-02,2.859386e-02,2.942658e-02,3.013384e-02,3.070848e-02,3.114467e-02,3.143796e-02,3.158536e-02,
        3.158536e-02,3.143796e-02,3.114467e-02,3.070848e-02,3.013384e-02,2.942658e-02,2.859386e-02,2.764402e-02,2.658654e-02,2.543187e-02,
        2.419129e-02,2.287679e-02,2.150091e-02,2.007654e-02,1.861682e-02,1.713493e-02,1.564394e-02,1.415667e-02,1.268549e-02,1.124225e-02,
        9.838064e-03,8.48326e-03,7.187231e-03,5.958361e-03,4.803945e-03,3.73013e-03,2.741877e-03,1.842935e-03,1.035839e-03,3.219224e-04,
        -2.986579e-04,-8.268721e-04,-1.264759e-03,-1.615354e-03,-1.8826e-03,-2.071255e-03,-2.186788e-03,-2.235263e-03,-2.223231e-03,-2.157603e-03
    };
static float FirBuffMainAlex[80] = {0};
static sFirFilter FirMainAlex;
float ProcessMainAlexWeight(float in)
{
	static unsigned short nDivIndex = 0;
	static unsigned short nDivMax = 6;
	static float fDivSum = 0.0f;
	static float fResult = 0.0f;

	fDivSum += in;
	nDivIndex++;
	if(nDivIndex < nDivMax) return fResult;

	fResult = FirFilterStatic(&FirMainAlex, fDivSum / nDivIndex);
	fDivSum = 0.0f;
	nDivIndex = 0;

	//动态处理后有一定衰减作用，将其恢复
	fResult /= 0.955f;

	return fResult;
}

/*
 * 功能:    动态轴处理初始化
 */
char  Mode8AlexDyncProcInit(void* pDecb)
{
	if (pDecb == 0) return (char)-1;

	SMODE8ALEXDYNC(pDecb).fAlexMaxWet = 0;
	SMODE8ALEXDYNC(pDecb).fAlexWet = 0;
	SMODE8ALEXDYNC(pDecb).fAlexUpWet = 0;
	SMODE8ALEXDYNC(pDecb).fAlexDownWet 	= 0;
	SMODE8ALEXDYNC(pDecb).nAlexDealOver	= 0;
		
	SMODE8ALEXDYNC(pDecb).nAlexValid = 0;
	SMODE8ALEXDYNC(pDecb).nPulseWidth = 0;
	SMODE8ALEXDYNC(pDecb).nUpFlag = 0;
	SMODE8ALEXDYNC(pDecb).nUpCnt = 0;
	SMODE8ALEXDYNC(pDecb).nDownkeepCnt = 0;
	SMODE8ALEXDYNC(pDecb).nDownCnt = 0;

	if(SMODE8ALEXDYNC(pDecb).mSpeedLen == 0)
		SMODE8ALEXDYNC(pDecb).mSpeedLen = 80;
	
	SMODE8ALEXDYNC(pDecb).iPowerOnIndex = 0;  
	SMODE8ALEXDYNC(pDecb).nAlexNo = 0; 
	SMODE8ALEXDYNC(pDecb).nAlexINorRemove = 0; 
	SMODE8ALEXDYNC(pDecb).nAlexWet = 0; 
	SMODE8ALEXDYNC(pDecb).nAlexPulseWith = 0; 
	SMODE8ALEXDYNC(pDecb).nAlexMaxWet = 0; 

	SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;

	FirMainAlex.nLen = 80;
	FirMainAlex.pDelay = FirBuffMainAlex;
	FirMainAlex.pZNum = znumF800L10R1;
	FirMainAlex.bInit = 0;

    return 0;
}

#define SPEED_POINT_COUNT		10
static const float arrSpeedPoint[SPEED_POINT_COUNT] = {0.0f, 2.5f, 5.0f, 7.5f, 10.0f, 12.5f, 15.0f, 20.0f, 25.0f, 30.0f};
static float arrK[SPEED_POINT_COUNT];
extern gWetParam gWetPar;
float AlexSpeedCorrect(float fWet, float fSpeed)
{
	int i;
	float k, wet = 0.0f;

	arrK[0] = (float)gWetPar.Follow_0k / 1000.0f;
	arrK[1] = (float)gWetPar.Follow_2P5k / 1000.0f;
	arrK[2] = (float)gWetPar.Follow_5k / 1000.0f;
	arrK[3] = (float)gWetPar.Follow_7P5k / 1000.0f;
	arrK[4] = (float)gWetPar.Follow_10k / 1000.0f;
	arrK[5] = (float)gWetPar.Follow_12P5k / 1000.0f;
	arrK[6] = (float)gWetPar.Follow_15k / 1000.0f;
	arrK[7] = (float)gWetPar.Follow_20k / 1000.0f;
	arrK[8] = (float)gWetPar.Follow_25k / 1000.0f;
	arrK[9] = (float)gWetPar.Follow_30k / 1000.0f;

	do
	{
		//低于最低速度
		if(fSpeed <= arrSpeedPoint[0])
		{
			wet = fWet * arrK[0];
			break;
		}

		//超过最高速度
		if(fSpeed >= arrSpeedPoint[SPEED_POINT_COUNT-1])
		{
			wet = fWet * arrK[9];
			break;
		}

		//寻找输入点所在的段
		for(i = 1; i < SPEED_POINT_COUNT; i++)
		{
			if(fSpeed < arrSpeedPoint[i])
			{
				k = arrK[i-1] +  (arrK[i] - arrK[i-1]) * (fSpeed-arrSpeedPoint[i-1]) / (arrSpeedPoint[i]-arrSpeedPoint[i-1]);
				wet = fWet * k;
				break;
			}
		}
	}while(0);

	//速度修正系数异常保护
	if((wet < 10.0f) || (wet > 5.0f*fWet)) return fWet;

	return wet;
}

//发送轴消息
static void MsgPostAlexEvent(void *pDecb, int event, int alexwet, int alexmax, int mAxleDValue)
{
	sMode8AlexDyncProc *pDync = (sMode8AlexDyncProc *)pDecb;
	int tmptick = 0, tick = pDync->nPulseWidth;
	
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith		= pDync->nPulseWidth;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexDValue 			= mAxleDValue;
	pDync->nAlexNo				= 2;
	
	if(tick != 0)
	{
		//按比例计算轴的脉冲时间
		if(((alexmax - pDync->fWetUp) > 0) && ((alexmax - pDync->fWetDown) > 0) )
		{
			//触发前的时间
			tmptick = (int)((pDync->nHalfPulseWidth + 2) * pDync->fWetUp / (alexmax - pDync->fWetUp));
			//释放后的时间
			tmptick += (int)((pDync->nPulseWidth - pDync->nHalfPulseWidth + 2) * pDync->fWetDown / (alexmax - pDync->fWetDown));
			tmptick += pDync->nPulseWidth;
		}
		
		if(tmptick > 0) tick = tmptick;

		pDync->fSpeed = 2.0f * (float)(pDync->mSpeedLen * 0.8f * 3.6f) / tick;
		pDync->nAlexWet = (int)AlexSpeedCorrect((float)alexwet, pDync->fSpeed);
	}

	pDync->pADP(pDecb);
}

//预处理轴信号，发送轴信号
static void AlexParserSignal(void *pDecb,int BfRaiseKg, int AftFallKg, int nAlexMax, int BfScaleRaiseKg,int AftScaleFallKg, int mScalerDValue)
{
	int nAlexUpDownGate;
	nAlexUpDownGate = (int)((float)nAlexMax * 0.1f);
	if(nAlexUpDownGate < ALEXUPDOWN) nAlexUpDownGate = ALEXUPDOWN;

	//触发前的值变化量大于出发后的变化量 表示上轴
	if(AftFallKg  >= BfRaiseKg  + nAlexUpDownGate)
	{
		MsgPostAlexEvent(pDecb,1, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * SMODE8ALEXDYNC(pDecb).fDyncK, nAlexMax, mScalerDValue);
		SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;	//轴相关索引置零
	}
	else if(AftFallKg <=  BfRaiseKg - nAlexUpDownGate)
	{
		MsgPostAlexEvent(pDecb,0, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * SMODE8ALEXDYNC(pDecb).fDyncK, nAlexMax, mScalerDValue);
	}
	else if(SMODE8ALEXDYNC(pDecb).nPulseWidth < 200)
	{
		MsgPostAlexEvent(pDecb,1, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * SMODE8ALEXDYNC(pDecb).fDyncK, nAlexMax, mScalerDValue);
		SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;	//轴相关索引置零
	}
}

//发送轴消息
static void MsgPostLeaveEvent(void *pDecb, int event, int alexwet, int alexvalidtime, int alexmax)
{
	sMode8AlexDyncProc *pDync = (sMode8AlexDyncProc *)pDecb;
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith		= alexvalidtime;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexNo				= 3;
	
	if(alexvalidtime != 0)
	{
		pDync->fSpeed = 2.0f * (float)(pDync->mSpeedLen * 0.8f * 3.6f) / alexvalidtime;
	}

	pDync->pADP(pDecb);
}


//预处理下秤轴，发送下秤轴信号
static void LeaveParserSignal(void *pDecb,int BfRaiseKg, int AftFallKg, int nAlexMax, int BfScaleRaiseKg,int AftScaleFallKg, char nPreAlexEvent)
{
	int nAlexUpDownGate;
	nAlexUpDownGate = (int)((float)nAlexMax * 0.1f);
	if(nAlexUpDownGate < ALEXUPDOWN) nAlexUpDownGate = 100;

	if(AftFallKg  >= BfRaiseKg  + nAlexUpDownGate)
	{
		if(nPreAlexEvent == 1)
		{
			MsgPostLeaveEvent(pDecb, 1, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * 0.15, SMODE8ALEXDYNC(pDecb).nLeavePulseWidth, (float)nAlexMax);
			debug(Debug_Business, "**后轴强制修正为进轴\r\n");
		}
		else
		{
			MsgPostLeaveEvent(pDecb, 0, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * 0.15, SMODE8ALEXDYNC(pDecb).nLeavePulseWidth, (float)nAlexMax);
		}	
	}
	else if(AftFallKg <=  BfRaiseKg - nAlexUpDownGate)
	{
		MsgPostLeaveEvent(pDecb, 1, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * 0.15, SMODE8ALEXDYNC(pDecb).nLeavePulseWidth, (float)nAlexMax);
	}
	else if(SMODE8ALEXDYNC(pDecb).nLeavePulseWidth < 200)
	{
		MsgPostLeaveEvent(pDecb, 1, AftScaleFallKg - BfScaleRaiseKg + BfScaleRaiseKg * 0.15, SMODE8ALEXDYNC(pDecb).nLeavePulseWidth, (float)nAlexMax);
	}
}


//===================================================================
//轮胎识别器
/* 保存轴型 */
static void Judge_Axle_Business(sMode8AlexDyncProc *pDync)
{
	TyreDevice tyreinfo = {0};
	
	//轮胎队列为空
	if(TyreQueue_len() == 0) 
	{	
		pDync->bDouble = 2;
	}
	else
	{
		if(TyreQueue_Get_Rear(&tyreinfo) == 0)
		{
			pDync->bDouble = tyreinfo.bDouble;
			debug(Debug_Business, "胎识别=%s\r\n", Get_Car_TryeType(pDync->bDouble));
		}
		else
		{
			pDync->bDouble = 2;
		}

		//保存后清轮胎队列
		TyreQueue_Init();
	}
}


int g_bAlexHold = 0;
int IsAlexHold(void)
{
	return g_bAlexHold;
}

static float mMainAlexSteadyWet;

float GetSmallWet(void)
{
	return mMainAlexSteadyWet;

}

//============================================
//秤台轴重差变量
static int nAlexFallAfter200msKg	 = 0;		//轴触发时的秤台重量
static int nAlexFallAfterAvgKg	 = 0;		//轴有效释放后200ms的秤台重量和
static int nAlexFallAfter200msKgTmp = 0;
static int nAlexDValue  = 0;
static BOOL bDvalueFlag = FALSE;

//计时时间 160*1.25ms = 200ms
#define D_VALUE_TICK	160

static void Calc_DValue(int tick)
{
	if(tick <= 0) return;

	nAlexDValue = nAlexFallAfterAvgKg/tick - nAlexFallAfter200msKgTmp;
	nAlexFallAfterAvgKg = 0;
	nAlexFallAfter200msKgTmp = 0;
}

/*
 * 功能:    动态轴处理
 */
float Mode8AlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{
	//静态变量
	static int fAlexUpWetTmp = 0;
	static float mMainAlexSteadyMin;
	static float mMainAlexSteadyMax;
	static float mMainAlexSteadyMinTmp;
	static float mMainAlexSteadyMaxTmp;

	static int nLeaveFallSumKg = 0;
	static int nLeaveFallIndex = 0;
	static int fLeaveUpWetTmp = 0;
	static char bPreAlexEvent = 0;

	//临时变量
	TaskMsg msg = {0};
	sMode8AlexDyncProc *pDync = (sMode8AlexDyncProc *)pDecb;
	float mScaleWet = pInArr[0];
	float mLeaveWet = pInArr[1];
	float mMainAlexWet = pInArr[2];
	float mRegAlexWet = pInArr[3];
	
	
	//参数保护处理
	if(pDecb == 0) return mScaleWet;
	if(pDync->iPowerOnIndex++ < pDync->iPowerOnDelay) return 0;

	//调用轴重滤波处理
	mMainAlexSteadyWet = ProcessMainAlexWeight(mMainAlexWet);

	//轴相关索引，用于判别在上秤端有加轴信号10秒内下秤端不该发生退轴
	if(SMODE8ALEXDYNC(pDecb).nAlexConvIndex < 100000)
	{
		SMODE8ALEXDYNC(pDecb).nAlexConvIndex++;	//累加
	}

	//================================================================================================
	//================================================================================================
	// 前轴识别处理
	if(mRegAlexWet > pDync->fWetUp)
	{
		pDync->nUpCnt++;
		pDync->nDownCnt = 0;

		if(pDync->nUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
		{			
			if(pDync->nUpFlag == 0)
			{
				if(pDync->nAlexValid == 1)
				{
					fAlexUpWetTmp = mMainAlexWet;				//在下一个轴到来前轴还没确定时缓存大秤轴触发重量
					mMainAlexSteadyMinTmp = mMainAlexSteadyWet;
					mMainAlexSteadyMaxTmp = mMainAlexSteadyWet;
				}
				else
				{
					pDync->fAlexUpWet = mMainAlexWet;			//记录大秤轴触发重量
					mMainAlexSteadyMin = mMainAlexSteadyWet;
					mMainAlexSteadyMax = mMainAlexSteadyWet;

					pDync->fAlexMaxWet = mRegAlexWet;	//另外一种情况，当pDync->nAlexValid为1是，自然会清零最大值
				}

				nAlexFallAfter200msKg = (int)mScaleWet;    //记录轴触发时的秤台重量

				//记录已经上翻转
				pDync->nUpFlag = 1;		
				SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;

				//发送轴触发消息
				msg.msg_id = Msg_PreAlex_Come;
				Task_QPost(&BusinessMsgQ, &msg);
			}	
		}		
	}
	else if(mRegAlexWet < pDync->fWetDown)
	{
		pDync->nDownCnt++;
		pDync->nUpCnt= 0;
		if (pDync->nDownCnt >= 2)
		{
			if(pDync->nUpFlag == 1)						//在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->fAlexDownWet = mMainAlexWet;		//记录下小秤轴重来
				pDync->nAlexValid = 1;					//记录轴有效
				pDync->nDownkeepCnt = 0;

				pDync->bDouble = 1;
				if(Get_Tire_ErrStatus())
				{
					Judge_Axle_Business(pDync);
				}
			}				
			
			pDync->nUpFlag = 0;			
		}
	}

	//当轴有效的时候进行计数，记录脉宽的时间
	if(pDync->nUpFlag == 1)
	{
		if(pDync->nPulseWidth < 100000000)		//防止溢出
		{
			pDync->nPulseWidth++;
		}		
	}

	//记录轴识别器最大值
	if(pDync->nUpFlag == 1)
	{
		if(mRegAlexWet > pDync->fAlexMaxWet)
		{
			pDync->fAlexMaxWet = mRegAlexWet;					//记录轴识别器最大值
			pDync->nHalfPulseWidth = pDync->nPulseWidth;
		}
	}

	//当轴有效的时候，从大秤轴重峰值开始计算大秤轴重的平均值
	if((pDync->nUpFlag == 1) || (pDync->nAlexValid == 1))
	{
		if(mMainAlexSteadyWet < mMainAlexSteadyMin)
		{
			mMainAlexSteadyMin = mMainAlexSteadyWet;
		}

		if(mMainAlexSteadyWet > mMainAlexSteadyMax)
		{
			mMainAlexSteadyMax = mMainAlexSteadyWet;
		}
	}
	
	//轴识别器有效信号
	if((pDync->nUpFlag == 1) || (pDync->nAlexValid == 1))
	{
		g_bAlexHold = 1;
	}
	else
	{
		g_bAlexHold = 0;
	}

	//轴信号方波完成后延时一定时间取轴重
	if(pDync->nAlexValid == 1)
	{
		//缓存触发时秤台重量
		if(pDync->nDownkeepCnt == 0)
		{
			nAlexFallAfter200msKgTmp = nAlexFallAfter200msKg;
			nAlexFallAfter200msKg = 0;
		}
		
		//记录轴释放后200ms内的秤台重量和
		if(pDync->nDownkeepCnt < D_VALUE_TICK)
		{
			nAlexFallAfterAvgKg += (int)mScaleWet;
		}
		//200ms计时到,计算秤台重量差值
		if(pDync->nDownkeepCnt == D_VALUE_TICK)
		{
			Calc_DValue(D_VALUE_TICK);
			bDvalueFlag = TRUE;
		}
		
		if((pDync->nDownkeepCnt++ >= 80*5) || (pDync->nUpFlag == 1))		//10个点以后了才取从触发下降沿口200ms后 的重量
		{
			//计时未到200ms,以当前计时计算
			if(!bDvalueFlag) Calc_DValue(pDync->nDownkeepCnt);

			AlexParserSignal(pDecb, pDync->fAlexUpWet, pDync->fAlexDownWet,  pDync->fAlexMaxWet, 
				mMainAlexSteadyMin, mMainAlexSteadyMax, nAlexDValue);
			
			bDvalueFlag  = FALSE;

			pDync->fAlexMaxWet = 0;
			pDync->fAlexWet = 0;
			pDync->fAlexUpWet = 0;
			pDync->fAlexDownWet = 0;
			pDync->nAlexDealOver = 0;
				
			pDync->nPulseWidth = 0;
			pDync->nAlexValid = 0;
			pDync->nDownkeepCnt = 0;

			if(pDync->nUpFlag == 1)
			{
				pDync->fAlexUpWet = fAlexUpWetTmp;
				mMainAlexSteadyMin = mMainAlexSteadyMinTmp;
				mMainAlexSteadyMax = mMainAlexSteadyMaxTmp;
			}
		}
	}
	





	//================================================================================================
	//================================================================================================
	// 后轴识别处理
	if(mLeaveWet > pDync->fLeaveWetUp)
	{
		pDync->nLeaveUpCnt++;
		pDync->nLeaveDownCnt = 0;

		if(pDync->nLeaveUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
		{			
			if(pDync->nLeaveUpFlag == 0)
			{
				if(pDync->nLeaveValid == 1)
				{
					fLeaveUpWetTmp = mScaleWet;
				}
				else
				{
					pDync->fLeaveUpWet = mScaleWet;
					pDync->fLeaveMaxWet = mLeaveWet;
				}
				
				pDync->nLeaveUpFlag = 1;				//记录已经上翻转
				bPreAlexEvent = 0;

				//发送下秤轴触发消息
				msg.msg_id = Msg_BackAlex_Come;
				Task_QPost(&BusinessMsgQ, &msg);
			}	
		}		
	}
	else if(mLeaveWet < pDync->fLeaveWetDown)
	{
		pDync->nLeaveDownCnt++;
		pDync->nLeaveUpCnt= 0;
		if (pDync->nLeaveDownCnt >= 2)
		{
			if(pDync->nLeaveUpFlag == 1) //在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->fLeaveDownWet = mScaleWet;
				pDync->nLeaveValid = 1;				//记录轴有效
				pDync->nLeaveDownkeepCnt = 0;
			}
			
			pDync->nLeaveUpFlag = 0;			
		}
	}
	
	if(pDync->nLeaveUpFlag == 1)
	{
		//记录下秤轴识别器的最大值
		if(mLeaveWet > pDync->fLeaveMaxWet)
		{
			pDync->fLeaveMaxWet = mLeaveWet;
		}

		//记录下轴脉宽
		if(pDync->nLeavePulseWidth < 100000000)		//防止溢出
		{
			pDync->nLeavePulseWidth++;
		}

		//判断上秤端有没有车同时上秤
		//if(pDync->nUpFlag == 1)
		//{
		//	bPreAlexEvent = 1;
		//}

		//如果十秒钟以内，上秤端进轴过，认为下秤端进轴
		if(SMODE8ALEXDYNC(pDecb).nAlexConvIndex < 8000)
		{
			bPreAlexEvent = 1;
		}
	}

	//后轴消息
	if(pDync->nLeaveValid ) //保存轴数的一个延时判断
	{
		nLeaveFallSumKg += (int)mScaleWet;
		nLeaveFallIndex++;

		if((pDync->nLeaveDownkeepCnt++ >= 80*2) || (pDync->nLeaveUpFlag == 1))		//10个点以后了才取从触发下降沿口200ms后 的重量
		{	
			LeaveParserSignal(pDecb, pDync->fLeaveUpWet, pDync->fLeaveDownWet, pDync->fLeaveMaxWet, pDync->fLeaveUpWet, nLeaveFallSumKg / nLeaveFallIndex, bPreAlexEvent);

			pDync->nLeaveValid = 0;
			pDync->nLeaveDownkeepCnt = 0;
			pDync->fLeaveMaxWet = 0;
			pDync->nLeavePulseWidth = 0;

			nLeaveFallSumKg = 0;
			nLeaveFallIndex = 0;
			bPreAlexEvent = 0;

			if(pDync->nLeaveUpFlag == 1)
			{
				pDync->fLeaveUpWet = fLeaveUpWetTmp;
			}
		}
	}

	return mScaleWet;
}

