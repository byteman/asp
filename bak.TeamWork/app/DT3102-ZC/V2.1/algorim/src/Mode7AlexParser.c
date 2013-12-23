#include "Mode7AlexParser.h"
#include <math.h>  
#include <stdlib.h>
#include "device_info.h"
#include "axletype_business.h"
#if 0
//2S钟数据缓冲
#define MAX_DELAY  10						//定义长度为两秒钟
static int nBigDelayArr[MAX_DELAY];			//
static int nBigDelayIndex;					//

static int				nPulseWidth = 0;	//轴识别器滤波
static int				nAlexStopKg = 0;	//车停在轴识别器上一秒后取的数据
static int				nLeaveStopKg = 0;	//车停在轴识别器上一秒后取的数据

//动态处理50Hz的5点均值滤波初始化
static int nAlexRaiseBefore200msKg = 0;
static int nAlexRaiseBeforeTmp = 0;
static int nAlexFallAfter200msKg	 = 0;
static int nAlexFallAfterAvgKg	 = 0;
static int nAlexFallAfterAvgIndex = 0;
static int   nAlexMaxKg = 0; 


static int nLeaveRaiseBefore200msKg = 0;
static int nLeaveRaiseBeforeTmp = 0;
static int nLeaveFallAfter200msKg	 = 0;
static int nLeaveFallAfterAvgKg	 = 0;
static int nLeaveFallAfterAvgIndex = 0;
static int nLeaveMaxKg = 0; 

static void TotalDelayQueue(int nBigKg)			//
{
	if(nBigDelayIndex >= MAX_DELAY) nBigDelayIndex= 0;

	//每200ms一个数据
	nBigDelayArr[nBigDelayIndex++] = nBigKg;
}

//获取两秒以前的总重
static int GetBigWeightBefore2S()
{
	int n = nBigDelayIndex;

	if(n >= MAX_DELAY) n= 0;

	return nBigDelayArr[n];
}

static void MsgPostAlexAdd(void *pDecb,int event,int alexwet,int alexvalidtime,int alexmax)
{
	sMode7AlexDyncProc *pDync = (sMode7AlexDyncProc *)pDecb;
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith			= alexvalidtime;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexNo				= 2;
	if(alexvalidtime != 0)
	{
		pDync->fSpeed				= 2.0f * (float)(pDync->mSpeedLen * 0.8f * 3.6f) / alexvalidtime;
	}

#if 1
	if(pDync->nAlexINorRemove == 1)
	{
		if(pDync->bDouble != 2)
			pDync->pADP(pDecb);
		else
			debug(Debug_Business, "Warning:TyreQueue empty, ingore this AxleAdd event!\r\n");
	}
	else
	{
		pDync->pADP(pDecb);
	}	
#else
	pDync->pADP(pDecb);
#endif
}

//发送轴信号
static void AlexParser_SendAlexSignal(void *pDecb,int BfRaiseKg, int AftFallKg, int AftFallAvg, int nAlexMax)
{
	//触发前的值
	if((AftFallKg - BfRaiseKg) >= -200)
	{
		//触发轴均值减触发前的值，精度会高些
		MsgPostAlexAdd(pDecb,1, AftFallAvg - BfRaiseKg, nPulseWidth, nAlexMax);
	}

	//如果脉冲超宽（车辆停在轴上），判断两秒以前的值
	else if(nPulseWidth > 1600)	//800Hz采样率，1600代表两秒
	{
		if((GetBigWeightBefore2S() - AftFallKg)  <= 200)
		{
			MsgPostAlexAdd(pDecb,1, nAlexStopKg - BfRaiseKg, nPulseWidth, nAlexMax);
		}
		else
		{
			MsgPostAlexAdd(pDecb,0, nAlexStopKg - BfRaiseKg, nPulseWidth, nAlexMax);
		}
	}
	else 
	{
		int diff_kg = AftFallKg - BfRaiseKg;

		//取消反向，轴重补偿是要使用负的数据
		//diff_kg = (diff_kg < 0) ? -diff_kg : diff_kg;

		MsgPostAlexAdd(pDecb,0, diff_kg, nPulseWidth, nAlexMax);
	}
}

//刷新大秤台重量
static void AlexParser_UpdateWet(int bigScalerKg)
{
	static int cnt_200ms = 0;	
	if(++cnt_200ms >= 16)
	{
		//延迟队列
		TotalDelayQueue(bigScalerKg);
		cnt_200ms = 0;
	}
}
#endif
 /*
 * 功能:    动态轴处理初始化
 */
char  Mode7AlexDyncProcInit(void* pDecb)
{
	if (pDecb == 0) return (char)-1;

	SMODE7ALEXDYNC(pDecb).nAlexValid = 0;
	SMODE7ALEXDYNC(pDecb).nPulseWidth = 0;
	SMODE7ALEXDYNC(pDecb).nUpFlag = 0;
	SMODE7ALEXDYNC(pDecb).nUpCnt = 0;
	SMODE7ALEXDYNC(pDecb).nUPkeepCnt = 0;
	SMODE7ALEXDYNC(pDecb).nDownkeepCnt = 0;
	SMODE7ALEXDYNC(pDecb).nDownCnt = 0;
	//SALEXDYNC(pDecb).fWetUp = 0;
	//SALEXDYNC(pDecb).fWetDown = 0;
	if(SMODE7ALEXDYNC(pDecb).mSpeedLen == 0)
		SMODE7ALEXDYNC(pDecb).mSpeedLen = 80;
	
	SMODE7ALEXDYNC(pDecb).fWet = 0;      
	SMODE7ALEXDYNC(pDecb).fWetReliab = 0;     
	SMODE7ALEXDYNC(pDecb).iPowerOnIndex = 0;  
	//SALEXDYNC(pDecb).iPowerOnDelay = 0; 

	SMODE7ALEXDYNC(pDecb).nAlexNo = 0; 
	SMODE7ALEXDYNC(pDecb).nAlexINorRemove = 0; 
	SMODE7ALEXDYNC(pDecb).nAlexWet = 0; 
	SMODE7ALEXDYNC(pDecb).nAlexPulseWith = 0; 
	SMODE7ALEXDYNC(pDecb).nAlexMaxWet = 0; 
	
	//memset((char *)SALEXDYNC(pDecb).fAD1Buf,0,sizeof(SALEXDYNC(pDecb).fAD1Buf));
	//memset((char *)SALEXDYNC(pDecb).fAD2Buf,0,sizeof(SALEXDYNC(pDecb).fAD2Buf));

    return 0;
}

//===================================================================
//轮胎识别器
/* 保存轴型 */
#if 0
static void Judge_Axle_Business(sMode7AlexDyncProc *pDync)
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
			debug(Debug_Business, "Tryetype=%d\r\n", pDync->bDouble);
		}
		else
		{
			pDync->bDouble = 2;
		}
		//保存后清轮胎队列
		TyreQueue_Init();
	}
}
#endif
/*
 * 功能:    动态轴处理
 */
float Mode7AlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{
	return pInArr[0];
#if 0
	sMode7AlexDyncProc *pDync = (sMode7AlexDyncProc *)pDecb;
	float mScaleWet = pInArr[0];
	float mAlexWet 	= pInArr[3];
	float mLeaveWet = pInArr[1];

	//参数保护处理
	if(pDecb == 0) return mScaleWet;
	if(pDync->iPowerOnIndex < pDync->iPowerOnDelay) 
	{
	    pDync->iPowerOnIndex++;
	    return 0;
	}

	//按50Hz频率对总重5点滤波
	AlexParser_UpdateWet((int)mScaleWet);
		
	//================================================================================================
	//================================================================================================
	// 前轴识别处理
	if(mAlexWet > pDync->fWetUp)
	{
		pDync->nUpCnt++;
		pDync->nDownCnt = 0;

		if(pDync->nUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
		{
			
			if(pDync->nUpFlag == 0)
			{
				if(pDync->nAlexValid == 1)
				{
					nAlexRaiseBeforeTmp = (int)mScaleWet;
				}
				else
				{
					nAlexRaiseBefore200msKg = (int)mScaleWet;
				}
				pDync->nUpFlag = 1;				//记录已经上翻转
				
				//发送轴触发消息
				//MsgPost(msgInAlexCome);
			}	
		}		
	}
	else if( mAlexWet < pDync->fWetDown )
	{
		pDync->nDownCnt++;
		if (pDync->nDownCnt >= 2)
		{
			if(pDync->nUpFlag  == 1) //在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->nAlexValid = 1;				//记录轴有效
				pDync->nDownkeepCnt = 0;
				
				//保存胎型, 根据轮胎队列是否为空决定事件响应
				pDync->bDouble = 1;
				if(Get_Tire_ErrStatus())
				{
					Judge_Axle_Business(pDync);
				} 
			}				
			pDync->nUpCnt= 0;
			pDync->nDownCnt = 0;
			pDync->nUpFlag = 0;
			
		}
	}
	//当轴有效的时候进行计数，记录脉宽的时间
	if(pDync->nUpFlag == 1)
	{
		if(pDync->nPulseWidth < 100000000)		//防止溢出
		{
			pDync->nPulseWidth++;
			nPulseWidth = pDync->nPulseWidth;
			if(pDync->nPulseWidth == 850)	//车停在轴上1S钟时取一个大秤台数据,AlexParser_Get200MsAvgWet有点延时，等850次采样
			{
				nAlexStopKg = (int)mScaleWet;
			}
		}
	}

	//识别轴有效
	if(pDync->nAlexValid ) //保存轴数的一个延时判断
	{
		//计算过轴后的均值
		nAlexFallAfter200msKg = (int)mScaleWet;
		nAlexFallAfterAvgKg += nAlexFallAfter200msKg;
		nAlexFallAfterAvgIndex++;

		if((pDync->nDownkeepCnt++ >= 16*10) || (pDync->nUpFlag == 1))		//10个点以后了才取从触发下降沿口200ms后 的重量
		{
			AlexParser_SendAlexSignal(pDecb,nAlexRaiseBefore200msKg, nAlexFallAfter200msKg, nAlexFallAfterAvgKg/nAlexFallAfterAvgIndex,nAlexMaxKg);

			pDync->nPulseWidth = 0;			//脉宽清零
			pDync->nAlexValid = 0;
			pDync->nDownkeepCnt = 0;
			
			nAlexFallAfter200msKg 		=  0;
			nAlexRaiseBefore200msKg	 = 0;
			
			nAlexFallAfterAvgKg = 0;
			nAlexFallAfterAvgIndex = 0;
			nAlexMaxKg = 0;

			
			if(pDync->nUpFlag == 1)
			{
				nAlexRaiseBefore200msKg = nAlexRaiseBeforeTmp;
			}
		}
	}

	//记录轴的峰值
	if((pDync->nUpFlag == 1) && (mAlexWet > nAlexMaxKg))
	{
		nAlexMaxKg = (int)mAlexWet;
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
				pDync->nLeaveUpFlag = 1;				//记录已经上翻转
				
				//发送轴触发消息
				//MsgPost(msgInLeaveCome);
			}	
		}		
	}
	else if( mLeaveWet < pDync->fLeaveWetDown )
	{
		pDync->nLeaveDownCnt++;
		if (pDync->nLeaveDownCnt >= 2)
		{
			if(pDync->nLeaveUpFlag  == 1) //在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->nLeaveValid = 1;				//记录轴有效
				pDync->nDownkeepCnt = 0;
			}				
			pDync->nLeaveUpCnt= 0;
			pDync->nLeaveDownCnt = 0;
			pDync->nLeaveUpFlag = 0;
			
		}
	}
#if 0
	//后轴消息
	if(pDync->nLeaveValid ) //保存轴数的一个延时判断
	{
		if((pDync->nLeaveDownkeepCnt++ >= 16*10) || (pDync->nLeaveUpFlag == 1))		//10个点以后了才取从触发下降沿口200ms后 的重量
		{	
			pDync->nLeaveValid = 0;
			pDync->nLeaveDownkeepCnt = 0;
			
			pDync->nAlexINorRemove		= 0;
			pDync->nAlexNo				= 3;
			pDync->pADP(pDecb);
		}
	}
#endif
	return mScaleWet;
#endif	
}

