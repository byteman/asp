#include "DyncAlexParserThree.h"
#include <math.h>  
#include <stdlib.h>

//2S钟数据缓冲
#define MAX_DELAY_ALEX  10						//定义长度为两秒钟
static int nBigDelayArr[MAX_DELAY_ALEX];			//
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
static int TempCarDir = 0;

typedef struct{
	float  Bf_AD;
	int    Up_Counter;
	int    Find_HighPt_Flag; //find high pt flag
	int    Up_Flag;   //up flag
	int    Down_Flag; //down flag
	int    Ret;
	
}SENSOR_STATUS,*PTR_SENSOR_STATUS;

SENSOR_STATUS	mAlexSensor1;
SENSOR_STATUS	mAlexSensor2;

static void TotalDelayQueue(int nBigKg)			//
{
	if(nBigDelayIndex >= MAX_DELAY_ALEX) nBigDelayIndex= 0;

	//每200ms一个数据
	nBigDelayArr[nBigDelayIndex++] = nBigKg;
}

//获取两秒以前的总重
static int GetBigWeightBefore2S()
{
	int n = nBigDelayIndex;

	if(n >= MAX_DELAY_ALEX) n= 0;

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
	
static void MsgPostAlexAdd(void *pDecb,int event,int alexno,int alexwet,int alexvalidtime,int alexmax)
{
	sAlexThreeDyncProc *pDync = (sAlexThreeDyncProc *)pDecb;
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith			= alexvalidtime;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexNo				= alexno;
	if(alexvalidtime != 0)
		pDync->fSpeed				= (float)(pDync->mSpeedLen * 0.8f * 3.6f) / alexvalidtime;
	pDync->pADP(pDecb);
}

//发送轴信号
static void AlexParser_SendAlexSignal(void *pDecb,int BfRaiseKg, int AftFallKg, int AftFallAvg, int nAlexMax)
{
	if(TempCarDir == 1)
	{
		MsgPostAlexAdd(pDecb,1,2, AftFallAvg - BfRaiseKg, nPulseWidth, nAlexMax);
	}
	else if(TempCarDir == 2)
	{
		MsgPostAlexAdd(pDecb,0,2, AftFallAvg - BfRaiseKg, nPulseWidth, nAlexMax);
	}
	//MsgPostAlexAdd(pDecb,1,2, nAlexStopKg - BfRaiseKg, nPulseWidth, nAlexMax);
	//触发前的值
	//如果脉冲超宽（车辆停在轴上），判断两秒以前的值
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
//////////////////////////////////////////////

 /*
 * 功能:    动态轴处理初始化
 */
char  AlexDyncProcThreeInit(void* pDecb)
{
	if (pDecb == 0) return (char)-1;

	SALEXTHREEDYNC(pDecb).nAlexValid = 0;
	SALEXTHREEDYNC(pDecb).nPulseWidth = 0;
	SALEXTHREEDYNC(pDecb).nUpFlag = 0;
	SALEXTHREEDYNC(pDecb).nUpCnt = 0;
	SALEXTHREEDYNC(pDecb).nUPkeepCnt = 0;
	SALEXTHREEDYNC(pDecb).nDownkeepCnt = 0;
	SALEXTHREEDYNC(pDecb).nDownCnt = 0;
	//SALEXTHREEDYNC(pDecb).fWetUp = 0;
	//SALEXTHREEDYNC(pDecb).fWetDown = 0;
	if(SALEXTHREEDYNC(pDecb).mSpeedLen == 0)
		SALEXTHREEDYNC(pDecb).mSpeedLen = 80;
	SALEXTHREEDYNC(pDecb).fWet = 0;      
	SALEXTHREEDYNC(pDecb).fWetReliab = 0;     
	SALEXTHREEDYNC(pDecb).iPowerOnIndex = 0;  
	//SALEXTHREEDYNC(pDecb).iPowerOnDelay = 0; 

	SALEXTHREEDYNC(pDecb).nAlexNo = 0; 
	SALEXTHREEDYNC(pDecb).nAlexINorRemove = 0; 
	SALEXTHREEDYNC(pDecb).nAlexWet = 0; 
	SALEXTHREEDYNC(pDecb).nAlexPulseWith = 0; 
	SALEXTHREEDYNC(pDecb).nAlexMaxWet = 0; 
	
	AlexParser_ResetFilter();
	
    return 0;
}

static  void AlexDealDownScaleAlex(void* pDecb,float mAlexWet) 
{
	sAlexThreeDyncProc *pDync = (sAlexThreeDyncProc *)pDecb;
	
	if(mAlexWet > pDync->fDownAlexWetUp)
	{
		pDync->nDownAlexUpCnt++;
		pDync->nDownAlexDownCnt = 0;

		if(pDync->nDownAlexUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
		{
			
			if(pDync->nDownAlexUpFlag == 0)
			{
				if(pDync->nDownAlexAlexValid == 1)
				{
					nAlexRaiseBeforeTmp = AlexParser_Get200MsAvgWet();
				}
				else
				{
					nAlexRaiseBefore200msKg = AlexParser_Get200MsAvgWet();
				}
				pDync->nDownAlexUpFlag = 1;				//记录已经上翻转				
				//发送轴触发消息
				//MsgPost(msgInAlexCome);
			}
			
			
		}		
	}
	else if( mAlexWet < pDync->fDownAlexWetDown )
	{
		pDync->nDownAlexDownCnt++;
		if (pDync->nDownAlexDownCnt >= 2)
		{
			if(pDync->nDownAlexUpFlag  == 1) //在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->nDownAlexAlexValid = 1;				//记录轴有效
				MsgPostAlexAdd(pDecb,1,3, pDync->nDownAlexMaxWet, pDync->nDownAlexPulseWidth, pDync->nDownAlexMaxWet);
			}				
			pDync->nDownAlexUpCnt= 0;
			pDync->nDownAlexDownCnt = 0;
			pDync->nDownAlexUpFlag = 0;
			pDync->nDownAlexDownkeepCnt = 0;
			pDync->nDownAlexPulseWidth = 0;
			pDync->nDownAlexMaxWet = 0;
		}
	}
	if(pDync->nDownAlexUpFlag )
	{
		if(pDync->nDownAlexPulseWidth < 100000000)		//防止溢出
			pDync->nDownAlexPulseWidth++;
		
		if(mAlexWet > pDync->nDownAlexMaxWet)
			pDync->nDownAlexMaxWet = mAlexWet;
	}
	
}
static int   Axis_Analysis_Channel(float inAD,int wet_up,PTR_SENSOR_STATUS status)
{
	status->Down_Flag = 0;
	
	if ( inAD > wet_up ) 
	{	
		//上秤台了
		if (  inAD > status->Bf_AD ) 
		{
			status->Bf_AD = inAD;
			status->Up_Counter++;
			if ( status->Up_Counter > 5 )
            		{
				status->Up_Flag = 1;
				status->Ret     = 0;
				return 0;//上称台(该路传感器AD值大于上秤台阀值后,连续5个点都呈现递增态，就认为上秤台了).
			}
		} 
		else if ( ( status->Bf_AD  - inAD ) >= 5 ) 
		{
			//在上秤台后至少发生了10次信号递增的情况，并且信号强度大于零点的某个阀值
			if ( ( status->Up_Counter >  10)  &&  ( ( status->Bf_AD - wet_up ) > 30 ) ) 
			{
				status->Up_Counter       	= 0;
				status->Find_HighPt_Flag 	= 1;
				status->Ret     		 	= 1;	
				return 1;///该路找到最高点.
			}
		}

	} 
	else 
	{
		status->Bf_AD                = 0;
		if ( status->Find_HighPt_Flag ) 
		{
			status->Down_Flag        = 1;
			status->Ret     		 = 2;			
			return 2;//一路找到了最高点,并已经下称台了
		}
	}
	
	status->Ret     		         = 3;
	
	return 3;//无状态正常返回.
}

/**
 * @Function Name: f_Lsw_Axis_Reset_AxisInfo
 * @Description:
 *               轴过完后，复位该轴的信息
 * @author coff (2008-8-27)
 */
static void  f_Lsw_Axis_Reset_AxisInfo(void)
{
	mAlexSensor1.Bf_AD               = 0;
	mAlexSensor2.Bf_AD               = 0;
	mAlexSensor1.Up_Counter          = 0;
	mAlexSensor2.Up_Counter          = 0;
	mAlexSensor1.Up_Flag 			= 0;
	mAlexSensor2.Up_Flag 			= 0;
	mAlexSensor1.Down_Flag 			= 0;
	mAlexSensor2.Down_Flag 			= 0;
	mAlexSensor1.Find_HighPt_Flag 	= 0;
	mAlexSensor2.Find_HighPt_Flag 	= 0;
}

static void AlexDyncFindHigh(float wet1,float wet2, int wetup )
{	

	Axis_Analysis_Channel(wet1,wetup,&mAlexSensor1);
	Axis_Analysis_Channel(wet2,wetup,&mAlexSensor2);

	if((mAlexSensor1.Ret == 0) || (mAlexSensor2.Ret == 0))
	{
		
	}
	//传感器找到了最高点====================================================================================
	if(mAlexSensor1.Ret == 1) 
	{ //这是一个瞬态值,表示找到一个最高点，但不一定是有效的最高点，保存下来，并判断方向
		if(!mAlexSensor2.Find_HighPt_Flag) 
		{
			TempCarDir = 1;
		}
	}
	if(mAlexSensor2.Ret == 1) 
	{
		if(!mAlexSensor1.Find_HighPt_Flag)
		{ 
			TempCarDir = 2;			
		}
	}

	if( mAlexSensor1.Ret == 2 ) 
	{//传感器1检测到一个瞬态的上下称台(而且找到过最高点)
		
       	 if ( mAlexSensor2.Up_Flag == 0 ) //但传感器2还没有上秤台过，可以判断为
		{
			//f_Lsw_Axis_Reset_AxisInfo();  
		}
	}

	if( mAlexSensor2.Ret == 2 ) //传感器2检测到一个瞬态的上下称台(而且找到过最高点)
	{

       	 if ( mAlexSensor1.Up_Flag == 0 ) //但传感器1还没有上秤台过，可以判断为
		{
			//f_Lsw_Axis_Reset_AxisInfo();
		}
	}
	//两路传感器都检测到下称台了======================================================================
	if(mAlexSensor1.Down_Flag && mAlexSensor2.Down_Flag) 
	{
		f_Lsw_Axis_Reset_AxisInfo();
	}
	
}
/*
 * 功能:    动态轴处理
 */
float AlexDyncThreeProc(void* pDecb, float * pInArr, int nInCount)
{
	sAlexThreeDyncProc *pDync = (sAlexThreeDyncProc *)pDecb;
	float mAlexWet = 0.0;
	
	pDync->mScaleWet = pInArr[0];

	pDync->mDownScaleAlex1Wet 	= pInArr[1];
	
	pDync->mUpScaleAlex1Wet 	= pInArr[2];
	pDync->mUpScaleAlex2Wet 	= pInArr[3];

	mAlexWet = pDync->mUpScaleAlex1Wet + pDync->mUpScaleAlex2Wet;
	
	//参数保护处理
	if(pDecb == 0) return pDync->mScaleWet;
	if(pDync->iPowerOnIndex < pDync->iPowerOnDelay) {
	    pDync->iPowerOnIndex++;
	    return pDync->mScaleWet;
	}
	if(pDync->fWetUp < 20) return pDync->mScaleWet;
	if(pDync->fWetDown < 20) return pDync->mScaleWet;

	//按50Hz频率对总重5点滤波
	AlexParser_UpdateWet(pDync->mScaleWet);

	//下称轴识别
	AlexDealDownScaleAlex(pDecb,pDync->mDownScaleAlex1Wet);
	AlexDyncFindHigh(pDync->mUpScaleAlex1Wet,pDync->mUpScaleAlex2Wet,pDync->fWetUp);
	
	// 上轴识别处理
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
	
	return pDync->mScaleWet;
	
}

