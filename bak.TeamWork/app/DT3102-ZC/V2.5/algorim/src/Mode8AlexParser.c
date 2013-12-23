#include "Mode8AlexParser.h"
#include <math.h>  
#include <stdlib.h>

#include "axletype_business.h"
#include "device_info.h"
#include "FirFilter.h"
#include "axle_business.h"
#include "scszc_debug_info.h"
#include "Weight_Param.h"

#define ALEXUPDOWN	150

#define ALEX_DELAY_NUM		24
static float BuffAlexDelay[ALEX_DELAY_NUM] = {0};
void PushAlexWeight(float in)
{
	int i;
	for(i = 0; i < ALEX_DELAY_NUM - 1; i++)
	{
		BuffAlexDelay[i+1] = BuffAlexDelay[i];
	}

	//最新的一个点放在队首
	BuffAlexDelay[0] = in;
}

float GetAlexBuffSum(int nCount)
{
	float sum;
	int i;

	if(nCount > ALEX_DELAY_NUM) nCount = ALEX_DELAY_NUM;

	sum = 0;
	for(i = 0; i < nCount; i++)
	{
		sum += BuffAlexDelay[i];
	}

	return sum;
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
#if 1
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
#else
	arrK[0] = 1.0f;
	arrK[1] = 1.0f;
	arrK[2] = 1.0f;
	arrK[3] = 1.0f;
	arrK[4] = 1.0f;
	arrK[5] = 1.0f;
	arrK[6] = 1.0f;
	arrK[7] = 1.0f;
	arrK[8] = 1.0f;
	arrK[9] = 1.0f;
#endif

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
	if((wet < 0.00001f) || (wet > 5.0f*fWet)) return fWet;

	return wet;
}

//发送轴消息
static void MsgPostAlexEvent(void *pDecb, int event, int alexwet, int alexvalidtime, int alexmax)
{
	sMode8AlexDyncProc *pDync = (sMode8AlexDyncProc *)pDecb;
	pDync->nAlexINorRemove 		= event;
	pDync->nAlexWet				= alexwet;
	pDync->nAlexPulseWith		= alexvalidtime;
	pDync->nAlexMaxWet			= alexmax;
	pDync->nAlexNo				= 2;
	
	if(alexvalidtime != 0)
	{
		pDync->fSpeed = (float)(pDync->mSpeedLen * 0.8f * 3.6f) / alexvalidtime;

		//轴重速度修正
		pDync->nAlexWet = (int)AlexSpeedCorrect((float)alexwet, pDync->fSpeed);
	}

	pDync->pADP(pDecb);
}

//预处理轴信号，发送轴信号
static void AlexParserSignal(void *pDecb, int nDerect, int nAlexWet, int nAlexMax)
{
	int nAlexUpDownGate;
	nAlexUpDownGate = (int)((float)nAlexMax * 0.1f);
	if(nAlexUpDownGate < ALEXUPDOWN) nAlexUpDownGate = ALEXUPDOWN;

	//触发前的值变化量大于出发后的变化量 表示上轴
	if(nDerect  > 0)
	{
		MsgPostAlexEvent(pDecb,1, nAlexWet, SMODE8ALEXDYNC(pDecb).nPulseWidth, (float)nAlexMax);
		SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;	//轴相关索引置零
	}
	else if(nDerect < 0)
	{
		MsgPostAlexEvent(pDecb,0, nAlexWet, SMODE8ALEXDYNC(pDecb).nPulseWidth, (float)nAlexMax);
	}

	//下面反倒可能多轴，情况是车辆压上轴识别器边缘迅速退下
	//else if((nDerect == 0) && (SMODE8ALEXDYNC(pDecb).nPulseWidth < 500))
	//{
	//	MsgPostAlexEvent(pDecb,1, nAlexWet, SMODE8ALEXDYNC(pDecb).nPulseWidth, (float)nAlexMax);
	//	SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;	//轴相关索引置零
	//}
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

//============================
//轴重对比
static int mAlexIndex = 0;

static float mRightTopWet = 0.0f;
static float mRightHalfAlexSum = 0.0f;
static int mRightHalfCnt = 0;

static float mLeftTopWet = 0.0f;
static float mLeftHalfAlexSum = 0.0f;
static int mLeftHalfCnt = 0;

static float Calc_Ratio_AlexWet(void)
{
	float mAlexLeftAvg = 0.0f;
	float mAlexRightAvg = 0.0f;
	float val = 0.0f;
	int nExceptCount = 0;

	nExceptCount = (int)((float)mAlexIndex * 0.3f);
	if(nExceptCount > ALEX_DELAY_NUM) nExceptCount = ALEX_DELAY_NUM;
	
	//计算右边轴重
	if(mRightHalfCnt > nExceptCount)
	{
		mAlexRightAvg = (mRightHalfAlexSum - GetAlexBuffSum(nExceptCount)) / (mRightHalfCnt - nExceptCount);
	}
	else
	{
		mAlexRightAvg = mRightTopWet;
	}
	//计算左边轴重
	if(mLeftHalfCnt > 0)
	{
		mAlexLeftAvg = mLeftHalfAlexSum / mLeftHalfCnt;
	}
	else
	{
		mAlexLeftAvg = mLeftTopWet;
	}
	debug(Debug_Business, "$左边轴重:%d,右边轴重:%d\r\n", (int)mAlexLeftAvg, (int)mAlexRightAvg);

	//被除数保护
	if((mAlexRightAvg<0.000001f) || (mAlexLeftAvg<0.000001f)) return mRightTopWet;

	//计算左右轴重比率
	if(mAlexLeftAvg > mAlexRightAvg)
	{
		val = (mAlexLeftAvg - mAlexRightAvg) / mAlexRightAvg;
	}
	else
	{
		val = (mAlexRightAvg - mAlexLeftAvg) / mAlexLeftAvg;
	}

	//0.25以下取平均值
	if(val < 0.25f) return ((mAlexRightAvg + mAlexLeftAvg)/2);

	//0.25~1.5取最小值
	if(val < 1.5f)
	{
		return (((mAlexRightAvg - mAlexLeftAvg) > 0.000001f) ? mAlexLeftAvg : mAlexRightAvg);
	}	
	//1.5~ 认为存在跳秤，取最大值
	return (((mAlexRightAvg - mAlexLeftAvg) > 0.000001f) ? mAlexRightAvg : mAlexLeftAvg);
}

static int Direction_Abnormal_Business(int direction, int lcnt, int rcnt)
{
	int tmp = direction;
	
	if(tmp == 0)
	{
		if((lcnt > 0) || (rcnt > 0))
		{
			if(lcnt > (rcnt+5))
			{
				tmp = 1;
			}
			else if((lcnt+5) < rcnt)
			{
				tmp = -1;
			}
		}
		debug(Debug_Business, "谷值分车轴数纠正,方向:%d LCnt=%d, RCnt=%d\r\n", tmp, lcnt, rcnt);
	}
	
	return tmp;
}
/*
 * 功能:    动态轴处理
 */
float Mode8AlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{
	//静态变量
	static int bDerectionUp = 0;
	static int bDerectionDown = 0;
	static float mAlexLeftMax = 0.0f;
	static float mAlexRightMax = 0.0f;
	static int mAlexLeftValid = 0;
	static int mAlexRightValid = 0;
	static float mAlexSum = 0.0f;
	
	static float mFloatZero = 0;
	static float mAlexHalfMax = 0;
	
	//临时变量
	TaskMsg msg = {0};
	sMode8AlexDyncProc *pDync = (sMode8AlexDyncProc *)pDecb;
	float mScaleWet = pInArr[0] + pInArr[2] + pInArr[3];		//整秤总重动态值
	float mAlexWet = pInArr[2] + pInArr[3];						//轴重动态值，包含左右和

	float mAlexLeftWet = pInArr[2];
	float mAlexRightWet = pInArr[3];
//	float mAlexAvg = 0.0f;

	static int mAlexLCnt = 0, mAlexRCnt = 0;	

	
	//参数保护处理
	if(pDecb == 0) return mScaleWet;
	if(pDync->iPowerOnIndex++ < pDync->iPowerOnDelay) return 0;

	//轴相关索引，用于判别在上秤端有加轴信号10秒内下秤端不该发生退轴
	if(SMODE8ALEXDYNC(pDecb).nAlexConvIndex < 100000)
	{
		SMODE8ALEXDYNC(pDecb).nAlexConvIndex++;	//累加
	}

	//================================================================================================
	//================================================================================================
	// 触发前轴识别处理
	if(mAlexWet > pDync->fWetUp)
	{
		pDync->nUpCnt++;
		pDync->nDownCnt = 0;
		if(pDync->nUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
		{			
			if(pDync->nUpFlag == 0)
			{
				//记录已经上翻转
				pDync->nUpFlag = 1;		

				//方向识别
				//常规触发可以确定方向，但要求设置时上秤阈值一定要大于下秤阈值，默认200的150
				if(mAlexLeftWet > mAlexRightWet)
				{
					bDerectionUp = 1;
				}
				else
				{
					bDerectionUp = -1;
				}

				bDerectionDown = 0;
					
				mAlexLeftMax = 0.0f;
				mAlexRightMax = 0.0f;
				mAlexLeftValid = 0;
				mAlexRightValid = 0;
				mAlexSum = 0.0f;
				mAlexIndex = 0;

				mRightHalfAlexSum = 0.0f;
				mRightHalfCnt = 0;
				mLeftHalfAlexSum = 0.0f;
				mLeftHalfCnt = 0;
				mRightTopWet = 0.0f;
				mLeftTopWet  = 0.0f;

				pDync->fAlexMaxWet = 0;
				mAlexHalfMax = 0;
				mFloatZero = 0;
				pDync->nPulseWidth = 0;
				SMODE8ALEXDYNC(pDecb).nAlexConvIndex = 0;

				//发送轴触发消息
				msg.msg_id = Msg_PreAlex_Come;
				Task_QPost(&BusinessMsgQ, &msg);
			}	
		}		
	}

	//是否轴识别器
	else if(mAlexWet < pDync->fWetDown)
	{
		pDync->nDownCnt++;
		pDync->nUpCnt= 0;
		if (pDync->nDownCnt >= 2)
		{
			if(pDync->nUpFlag == 1)						//在低于设定值以后，如果有上过一次的动作，表示下了。
			{
				pDync->nAlexValid = 1;					//记录轴有效
				
				//方向识别
				//下秤有可能处于跟轴谷值点临界识别，不能单纯的比大小
				if(bDerectionDown == 0)
				{
					if((mAlexLeftWet + pDync->fWetDown / 2) < mAlexRightWet)
					{
						bDerectionDown = 1;	//向前
					}
					else if((mAlexRightWet + pDync->fWetDown / 2) < mAlexLeftWet)
					{
						bDerectionDown = -1;//向后
					}
					else
					{
						//临界区，按触发时方向处理
						bDerectionDown = bDerectionUp;
					}
				}

				//胎型识别
				pDync->bDouble = 1;				
				if(Get_Tire_ErrStatus())
				{
					Judge_Axle_Business(pDync);
				}
			}				
			
			pDync->nUpFlag = 0;			
		}
	}

	//当轴有效的时候进行轴重处理
	if(pDync->nUpFlag == 1)
	{
		//如果两个都还处于上升趋势，轴重累计清零
		if((mAlexLeftWet > mAlexLeftMax) && (mAlexRightWet > mAlexRightMax))
		{
			//轴重清零
			mAlexSum = 0;
			mAlexIndex = 0;
			mAlexLeftValid = 0;
			mAlexRightValid = 0;

			mRightHalfAlexSum = 0.0f;
			mRightHalfCnt = 0;
			mLeftHalfAlexSum = 0.0f;
			mLeftHalfCnt = 0;
			mRightTopWet = 0.0f;
			mLeftTopWet  = 0.0f;

			//如果方向为决，判断方向
			//在后轮快速上来产生谷值分车时，那时不能确定方向，在此时确定
			if((bDerectionUp == 0) || (mAlexWet <= 1000))
			{
				if(mAlexLeftWet > mAlexRightWet)
				{
					bDerectionUp = 1;
				}
				else
				{
					bDerectionUp = -1;
				}
			}
		}

		//识别左峰值
		if(mAlexLeftWet > mAlexLeftMax)
		{
			mAlexLeftMax = mAlexLeftWet;
			mLeftTopWet = mAlexWet;
		}
		else if((mAlexLeftWet + 80) < mAlexLeftMax)
		{
			mAlexLeftValid = 1;
		}

		//识别右峰值
		if(mAlexRightWet > mAlexRightMax)
		{
			mAlexRightMax = mAlexRightWet;
			mRightTopWet = mAlexWet;			//记录右峰值轴重
		}
		else if((mAlexRightWet + 80) < mAlexRightMax)
		{
			mAlexRightValid = 1;
		}

		//区间内累加重量
		if(mAlexLeftValid != mAlexRightValid)
		{
			mAlexSum += mAlexWet;
			mAlexIndex++;

			if(mAlexRightWet >= mAlexLeftWet)
			{
				mRightHalfAlexSum += mAlexWet;		//右边轴重和
				mRightHalfCnt++;
			}
			else
			{
				mLeftHalfAlexSum += mAlexWet;		//左边轴重和
				mLeftHalfCnt++;
			}

			//缓存最后几个点
			PushAlexWeight(mAlexWet);
		}

		//记录轴重的最大值
		if(mAlexWet > pDync->fAlexMaxWet)
		{
			pDync->fAlexMaxWet = mAlexWet;					//记录轴识别器最大值
			mFloatZero = pDync->fAlexMaxWet;				//目的是将mFloatZero初始化为最大值
		}

		//当重量开始下降时，记录最大值的一半，作为联轴分段的参考
		if((mAlexWet + 80) < pDync->fAlexMaxWet)
		{
			mAlexHalfMax = pDync->fAlexMaxWet / 3;			
		}

		//在达到峰值之后，寻找谷值，认为谷值不能超过峰值的一半
		if((mAlexWet < mAlexHalfMax) && (mAlexWet < mFloatZero))
		{
			//更新谷值点
			mFloatZero = mAlexWet;

			//在未达到谷值时一直更新下秤方向
			//下秤有可能处于跟轴谷值点临界识别，不能单纯的比大小
			if(mAlexWet >= pDync->fAlexMaxWet / 10)
			{
				if((mAlexLeftWet + pDync->fWetDown / 2) < mAlexRightWet)
				{
					bDerectionDown = 1;	//向前
					mAlexLCnt++;
				}
				else if((mAlexRightWet + pDync->fWetDown / 2) < mAlexLeftWet)
				{
					bDerectionDown = -1;//向后
					mAlexRCnt++;
				}
				else
				{
					//临界区，按触发时方向处理
					bDerectionDown = bDerectionUp;
				}
			}
		}

		//从时序上讲，表明已经达到谷值并开始回升，可以判断后一个轴完成
		if(mAlexWet > (mFloatZero + 100))
		{
			//发送一个轴信号--------------------------------------------------
			{
				if(mAlexIndex <= 0)
				{
					AlexParserSignal(pDecb, Direction_Abnormal_Business(bDerectionUp+bDerectionDown, mAlexLCnt, mAlexRCnt), pDync->fAlexMaxWet,  pDync->fAlexMaxWet);
				}
				else
				{
					//mAlexAvg = Calc_Ratio_AlexWet();
					AlexParserSignal(pDecb, Direction_Abnormal_Business(bDerectionUp+bDerectionDown, mAlexLCnt, mAlexRCnt), Calc_Ratio_AlexWet(),  pDync->fAlexMaxWet);
				}

				pDync->fAlexMaxWet = 0;
				pDync->nAlexValid = 0;
				mAlexLCnt = 0;
				mAlexRCnt = 0;
			}
			//-----------------------------------------------------------------

			//初始化下一个轴信号-----------------------------------------------
			//方向识别，过了谷值点，可以直接比大小
			if(mAlexLeftWet > mAlexRightWet)
			{
				bDerectionUp = 1;
			}
			else
			{
				bDerectionUp = -1;
			}

			bDerectionDown = 0;
					
			mAlexLeftMax = 0.0f;
			mAlexRightMax = 0.0f;
			mAlexLeftValid = 0;
			mAlexRightValid = 0;
			mAlexSum = 0.0f;
			mAlexIndex = 0;

			mRightHalfAlexSum = 0.0f;
			mRightHalfCnt = 0;
			mLeftHalfAlexSum = 0.0f;
			mLeftHalfCnt = 0;
			mRightTopWet = 0.0f;
			mLeftTopWet  = 0.0f;

			pDync->fAlexMaxWet = 0;
			mAlexHalfMax = 0;
			mFloatZero = 0;
			pDync->nPulseWidth = 0;

			//发送轴触发消息
			msg.msg_id = Msg_PreAlex_Come;
			Task_QPost(&BusinessMsgQ, &msg);
		}		



		//记录脉宽时间算速度
		if(pDync->nPulseWidth < 100000000)		//防止溢出
		{
			pDync->nPulseWidth++;
		}	
	}


	//轴信号方波完成后延时一定时间取轴重
	if(pDync->nAlexValid == 1)
	{
		if(mAlexIndex <= 0)
		{
			AlexParserSignal(pDecb, Direction_Abnormal_Business(bDerectionUp+bDerectionDown, mAlexLCnt, mAlexRCnt), pDync->fAlexMaxWet,  pDync->fAlexMaxWet);
		}
		else
		{
			//mAlexAvg = Calc_Ratio_AlexWet();
			AlexParserSignal(pDecb, Direction_Abnormal_Business(bDerectionUp+bDerectionDown, mAlexLCnt, mAlexRCnt), Calc_Ratio_AlexWet(), pDync->fAlexMaxWet);
		}

		pDync->fAlexMaxWet = 0;
		pDync->nAlexValid = 0;
		mAlexLCnt = 0;
		mAlexRCnt = 0;
	}


	g_bAlexHold = pDync->nUpFlag;
	
	return mScaleWet;

	//================================================================================================
	//================================================================================================
	// 后轴识别处理
	//if(mLeaveWet > pDync->fLeaveWetUp)
	//{
	//	pDync->nLeaveUpCnt++;
	//	pDync->nLeaveDownCnt = 0;

	//	if(pDync->nLeaveUpCnt > 2) //连续有2次保持在设定的AD值以上，表示真的上到来。
	//	{			
	//		if(pDync->nLeaveUpFlag == 0)
	//		{
	//			if(pDync->nLeaveValid == 1)
	//			{
	//				fLeaveUpWetTmp = mScaleWet;
	//			}
	//			else
	//			{
	//				pDync->fLeaveUpWet = mScaleWet;
	//				pDync->fLeaveMaxWet = mLeaveWet;
	//			}
	//			
	//			pDync->nLeaveUpFlag = 1;				//记录已经上翻转
	//			bPreAlexEvent = 0;

	//			//发送下秤轴触发消息
	//			msg.msg_id = Msg_BackAlex_Come;
	//			Task_QPost(&BusinessMsgQ, &msg);
	//		}	
	//	}		
	//}
	//else if(mLeaveWet < pDync->fLeaveWetDown)
	//{
	//	pDync->nLeaveDownCnt++;
	//	pDync->nLeaveUpCnt= 0;
	//	if (pDync->nLeaveDownCnt >= 2)
	//	{
	//		if(pDync->nLeaveUpFlag == 1) //在低于设定值以后，如果有上过一次的动作，表示下了。
	//		{
	//			pDync->fLeaveDownWet = mScaleWet;
	//			pDync->nLeaveValid = 1;				//记录轴有效
	//			pDync->nLeaveDownkeepCnt = 0;
	//		}
	//		
	//		pDync->nLeaveUpFlag = 0;			
	//	}
	//}
	//
	//if(pDync->nLeaveUpFlag == 1)
	//{
	//	//记录下秤轴识别器的最大值
	//	if(mLeaveWet > pDync->fLeaveMaxWet)
	//	{
	//		pDync->fLeaveMaxWet = mLeaveWet;
	//	}

	//	//记录下轴脉宽
	//	if(pDync->nLeavePulseWidth < 100000000)		//防止溢出
	//	{
	//		pDync->nLeavePulseWidth++;
	//	}

	//	//判断上秤端有没有车同时上秤
	//	//if(pDync->nUpFlag == 1)
	//	//{
	//	//	bPreAlexEvent = 1;
	//	//}

	//	//如果十秒钟以内，上秤端进轴过，认为下秤端进轴
	//	if(SMODE8ALEXDYNC(pDecb).nAlexConvIndex < 8000)
	//	{
	//		bPreAlexEvent = 1;
	//	}
	//}

	////后轴消息
	//if(pDync->nLeaveValid ) //保存轴数的一个延时判断
	//{
	//	nLeaveFallSumKg += (int)mScaleWet;
	//	nLeaveFallIndex++;

	//	if((pDync->nLeaveDownkeepCnt++ >= 80*2) || (pDync->nLeaveUpFlag == 1))		//10个点以后了才取从触发下降沿口200ms后 的重量
	//	{	
	//		LeaveParserSignal(pDecb, pDync->fLeaveUpWet, pDync->fLeaveDownWet, pDync->fLeaveMaxWet, pDync->fLeaveUpWet, nLeaveFallSumKg / nLeaveFallIndex, bPreAlexEvent);

	//		pDync->nLeaveValid = 0;
	//		pDync->nLeaveDownkeepCnt = 0;
	//		pDync->fLeaveMaxWet = 0;
	//		pDync->nLeavePulseWidth = 0;

	//		nLeaveFallSumKg = 0;
	//		nLeaveFallIndex = 0;
	//		bPreAlexEvent = 0;

	//		if(pDync->nLeaveUpFlag == 1)
	//		{
	//			pDync->fLeaveUpWet = fLeaveUpWetTmp;
	//		}
	//	}
	//}

	//return mScaleWet;
}

