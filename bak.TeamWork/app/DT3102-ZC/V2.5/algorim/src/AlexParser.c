#include "AlexParser.h"
#include <math.h>  
#include <stdlib.h>

//2S钟数据缓冲
#define MAX_DELAY  2				//不使用，节约内存，改短到2，以前是10		//定义长度为两秒钟
static int nBigDelayArr[MAX_DELAY];			//
static int nBigDelayIndex;					//

static int				nPulseWidth = 0;	//轴识别器滤波
static int				nAlexStopKg = 0;	//车停在轴识别器上一秒后取的数据

//动态处理50Hz的5点均值滤波初始化
static int m_200msAvgWet = 0;
static int filter_cnt = 0;
static int filter_buf[5];
static int filter_valid_cnt = 0;

static int nAlexRaiseBefore200msKg = 0;
static int nAlexRaiseBeforeTmp = 0;
static int nAlexFallAfter200msKg	 = 0;
static int nAlexFallAfterAvgKg	 = 0;
static int nAlexFallAfterAvgIndex = 0;
static int   nAlexMaxKg = 0; 

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


static void AlexParser_ResetFilter(void)
{
	filter_cnt = 0;
	filter_valid_cnt = 0;
}

//50Hz的5点均值滤波，带异常点去除功能（输入范围在0--120000之间）
static void AlexParser_UpdateAvgWet(int wet)
{
	int i = 0;
	int  sum  = 0;

	if((wet >= -10000) && (wet <= 120000))
	{
		if(filter_cnt >= 5) filter_cnt = 0;
	 
		filter_buf[filter_cnt++] = wet;
		
		if(filter_valid_cnt < 5) filter_valid_cnt++;
		
		for(i = 0; i < filter_valid_cnt; i++)
		{
			sum += filter_buf[i];
		}
		m_200msAvgWet = sum / filter_valid_cnt;		
	}
}

//获取大秤台50Hz的5点均值滤波，带异常点去除功能（输入范围在0--120000之间）
static int AlexParser_Get200MsAvgWet(void)
{
	return m_200msAvgWet;
}
	
static void MsgPostAlexAdd(void *pDecb,int event,int alexwet,int alexvalidtime,int alexmax)
{
	sAlexDyncProc *pDync = (sAlexDyncProc *)pDecb;
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith			= alexvalidtime;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexNo				= 2;
	if(alexvalidtime != 0)
		pDync->fSpeed				= (float)(pDync->mSpeedLen * 0.8f * 3.6f) / alexvalidtime;
	
	pDync->pADP(pDecb);
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
	
	if(cnt_200ms++ >= 16)
	{
		//以50Hz的速率来处理		
		//对总重进行5点滤波
		AlexParser_UpdateAvgWet(bigScalerKg);
		//延迟队列
		TotalDelayQueue(bigScalerKg);
		cnt_200ms = 0;
	}
}

 /*
 * 功能:    动态轴处理初始化
 */
char  AlexDyncProcInit(void* pDecb)
{
	if (pDecb == 0) return (char)-1;

	SALEXDYNC(pDecb).nAlexValid = 0;
	SALEXDYNC(pDecb).nPulseWidth = 0;
	SALEXDYNC(pDecb).nUpFlag = 0;
	SALEXDYNC(pDecb).nUpCnt = 0;
	SALEXDYNC(pDecb).nUPkeepCnt = 0;
	SALEXDYNC(pDecb).nDownkeepCnt = 0;
	SALEXDYNC(pDecb).nDownCnt = 0;
	//SALEXDYNC(pDecb).fWetUp = 0;
	//SALEXDYNC(pDecb).fWetDown = 0;
	if(SALEXDYNC(pDecb).mSpeedLen == 0)
		SALEXDYNC(pDecb).mSpeedLen = 80;
	
	SALEXDYNC(pDecb).fWet = 0;      
	SALEXDYNC(pDecb).fWetReliab = 0;     
	SALEXDYNC(pDecb).iPowerOnIndex = 0;  
	//SALEXDYNC(pDecb).iPowerOnDelay = 0; 

	SALEXDYNC(pDecb).nAlexNo = 0; 
	SALEXDYNC(pDecb).nAlexINorRemove = 0; 
	SALEXDYNC(pDecb).nAlexWet = 0; 
	SALEXDYNC(pDecb).nAlexPulseWith = 0; 
	SALEXDYNC(pDecb).nAlexMaxWet = 0; 
	
	//memset((char *)SALEXDYNC(pDecb).fAD1Buf,0,sizeof(SALEXDYNC(pDecb).fAD1Buf));
	//memset((char *)SALEXDYNC(pDecb).fAD2Buf,0,sizeof(SALEXDYNC(pDecb).fAD2Buf));

	AlexParser_ResetFilter();
	
    return 0;
}
 
/*
 * 功能:    动态轴处理
 */
float AlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{
	sAlexDyncProc *pDync = (sAlexDyncProc *)pDecb;
	float mScaleWet = pInArr[0];
	float mAlexWet 	= pInArr[2];

	//参数保护处理
	if(pDecb == 0) return mScaleWet;
	if(pDync->iPowerOnIndex < pDync->iPowerOnDelay) {
	    pDync->iPowerOnIndex++;
	    return 0;
	}
	if(pDync->fWetUp < 20) return mScaleWet;
	if(pDync->fWetDown < 20) return mScaleWet;

	//按50Hz频率对总重5点滤波
	AlexParser_UpdateWet(mScaleWet);
		
	// 轴识别处理
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
					nAlexRaiseBeforeTmp = AlexParser_Get200MsAvgWet();
				}
				else
				{
					nAlexRaiseBefore200msKg = AlexParser_Get200MsAvgWet();
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
				nAlexStopKg = AlexParser_Get200MsAvgWet();
			}
		}
	}

	//识别轴有效
	if(pDync->nAlexValid ) //保存轴数的一个延时判断
	{
		//计算过轴后的均值
		nAlexFallAfter200msKg = AlexParser_Get200MsAvgWet();
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
		nAlexMaxKg = mAlexWet;
	}
	
	return mScaleWet;
	
}

