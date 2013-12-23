#include "includes.h"
#include "car_queue.h"
#include "sys_config.h"
#include "weight_param.h"
#include "axle_business.h"
#include "car_business.h"
#include "device_info.h"
#include "wet_algorim.h"
#include "debug_info.h"

#define SCALER_EMPTY_MAX		200		//空秤最大值，如果大于该值，认为是有车在秤上
#define SCALER_BIG_CAR_MIN		2000	//大车(三轴以上的车)的最小重量
#define  ALEX_ARR_MAX			20
#define AXLE_ERR_TIME			2500	//两轴间隔时间

#define SCALER_EMPTY_WET		500		//空秤阈值100kg
#define SCALER_CLEAR_TIME		120000	//空秤时间阈值20s

static volatile int32 m_nAlexCount = 0;
static AlexRecoder AlexRecoderArr[ALEX_ARR_MAX];
static AlexRecoder AlexRecoderArrNormal[ALEX_ARR_MAX];
static AlexRecoder m_AlexRecoder;

//时间增加
static int32 MsgTimeAdd(int nBase, int nAdd)
{
	//防止溢出
	if(nBase > 3600000) return nBase;		//已ms为单位，算下来都已经1小时了

	return nBase + nAdd;
}

//清除轴记录
void Clear_Axle_Recoder(void)
{
	m_nAlexCount = 0;				//轴计数
}

//处理AlexRecoderArr溢出的异常情况
static void Modify_AxleRecoder_Overflow(void)
{
	int i = 0;
	
	for(i = 1; i < m_nAlexCount; i++)
	{
		if(AlexRecoderArr[i].direct != AlexRecoderArr[i-1].direct)
		{
			//规定时间内不能反轴
			if(AlexRecoderArr[i-1].nTimeInterval < AXLE_ERR_TIME)
			{
				AlexRecoderArr[i].direct = AlexRecoderArr[i-1].direct;
			}
			else
			{
				//丢掉一对加减轴,拷贝数组[i+1]后的所有元素到[i-1]地址处
				if(m_nAlexCount >= 2) m_nAlexCount -= 2;	      
				if((i > 0) && (m_nAlexCount + 1 > i))
				{
					memmove(&AlexRecoderArr[i-1], &AlexRecoderArr[i+1], (m_nAlexCount-i+1)*sizeof(AlexRecoder));
					i--;
				}					
			}			
		}
	}
}

//增加轴记录
static void AddAlexRecoder(AlexRecoder* pAlexRecoder)
{
	if(m_nAlexCount >= ALEX_ARR_MAX)
	{
		Modify_AxleRecoder_Overflow();
		if(m_nAlexCount >= ALEX_ARR_MAX) return;
	}

	AlexRecoderArr[m_nAlexCount].direct = pAlexRecoder->direct;
	AlexRecoderArr[m_nAlexCount].AlexKg = pAlexRecoder->AlexKg;
	AlexRecoderArr[m_nAlexCount].AlexKgRef = pAlexRecoder->AlexKgRef;
	AlexRecoderArr[m_nAlexCount].nTimePulse = pAlexRecoder->nTimePulse;
	AlexRecoderArr[m_nAlexCount].bDouble = pAlexRecoder->bDouble;
	AlexRecoderArr[m_nAlexCount].nTimeInterval = 0;

	m_nAlexCount++;
}

//复制轴信息
static void Copy_AxleInfo(CarInfo* pCar, int nAlexCount)
{
	int i;

	if(pCar == NULL) return;

	for(i = 0; i < nAlexCount; i++)
	{
		pCar->AxleInfo[i].axle_wet = AlexRecoderArr[i].AlexKg;
		pCar->AxleInfo[i].axle_type = AlexRecoderArr[i].bDouble;
		pCar->AxleInfo[i].axle_len = AlexRecoderArr[i].nTimeInterval;
	}
}

//处理没有轴信号的车
static int32 PraseNoneAlexCar(CarInfo* pCar)
{
	int i = 0;
	
	if(pCar == NULL) return 0;

	//如果上秤前后重量大于SCALER_EMPTY_MAX，不用考虑了，非上秤莫属
	if(pCar->nScalerKgComing < 500)
	{
		if(Get_Static_Weight() - pCar->nScalerKgComing > 600)
		{		
			for(i = 0; i < 2; i++)
			{
				pCar->AxleInfo[i].axle_wet= (Get_Static_Weight() - pCar->nScalerKgComing) / 2;
				pCar->AxleInfo[i].axle_type = 0;
				pCar->AxleInfo[i].axle_len = 100;
			}
			debug(Debug_Business, "UpDownWet>600,maybe a little car!\r\n");
			return 2;
		}
	}

	return 0;
}

//处理只有一个轴的信号
static int32 PraseSingleAlexCar(CarInfo* pCar)
{
	int i = 0;
	
	if(pCar == NULL) 
		return 0;

	for(i = 0; i < 2; i++)
	{
		pCar->AxleInfo[i].axle_wet = AlexRecoderArr[0].AlexKg;
		pCar->AxleInfo[i].axle_type = AlexRecoderArr[0].bDouble;
		pCar->AxleInfo[i].axle_len = AlexRecoderArr[0].nTimeInterval;
	}

	//按方向判断
	if(AlexRecoderArr[0].direct > 0)
	{
		return 2;
	}
	else
	{
		if(Get_Static_Weight() - pCar->nScalerKgComing < -200)
		{
			return -2;
		}
		else
		{
			return 2;
		}
	}
}

static int32 PraseLittleCarAlex(CarInfo* pCar)
{
	if(pCar == NULL) return 0;

	//对四种情况作枚举分析

	//第一个为加轴，第二个也为加轴，肯定是上秤了-------------------------------------------
	if((AlexRecoderArr[0].direct > 0) && (AlexRecoderArr[1].direct > 0)) 
	{
		Copy_AxleInfo(pCar, 2);
		return 2;
	}

	//如果第一个轴为加轴，第二个为减轴-----------------------------------------------------
	if(AlexRecoderArr[0].direct > 0)
	{
		//如果第二个减轴时间小于3S，赌他不是倒车
		if(AlexRecoderArr[0].nTimeInterval < AXLE_ERR_TIME)
		{
			Copy_AxleInfo(pCar, 2);
			return 2;
		}

		//如果在两秒钟开外，重量在增加
		//如果上秤前后重量大于SCALER_EMPTY_MAX，不用考虑了，非上秤莫属
        //小车跟大车,同时倒车时会有问题
		if(Get_Static_Weight() - pCar->nScalerKgComing > SCALER_EMPTY_MAX)
		{		
			Copy_AxleInfo(pCar, 2);
			return 2;
		}

		//重量在减少，但是减少量操作两轴小车最大值2.5吨，表明是前面在下车，视为前进
		if(Get_Static_Weight() - pCar->nScalerKgComing < -2500)
		{	
			Copy_AxleInfo(pCar, 2);
			return 2;
		}

		return 0;
	}
	
	//如果第一个轴是减轴，第二个轴也是减轴，视为倒车---------------------------------------
	if((AlexRecoderArr[0].direct < 0) && (AlexRecoderArr[1].direct < 0)) 
	{
		//如果秤台重量低于200kg，视为倒车
		if(Get_Static_Weight() <= 200)
		{
			return -2;
		}
	
		//如果两个轴的脉宽都小于20ms，表明车速快，视为上车
		if((AlexRecoderArr[0].nTimePulse < 20) && (AlexRecoderArr[0].nTimePulse < 20))
		{
			return 2;
		}
	
		//如果两个轴的时间小于1.5s，视为上车
		if(AlexRecoderArr[0].nTimeInterval < 500)
		{
			return 2;
		}	
	
		return -2;
	}

	//如果第一个轴是减轴，第二个轴是加轴---------------------------------------------------
	//如果第二个加轴时间小于8S，赌他不是倒车
	if(AlexRecoderArr[0].nTimeInterval < 8000)
	{
		//当第一个轴上秤的时候，前面正在下秤，会出现这种情况
		Copy_AxleInfo(pCar, 2);
		return 2;
	}

	//如果秤台重量增加了，表明前一个轴波形识别错误
	//如果上秤前后重量大于SCALER_EMPTY_MAX，不用考虑了，非上秤莫属
	if(Get_Static_Weight() - pCar->nScalerKgComing > SCALER_EMPTY_MAX)
	{		
		Copy_AxleInfo(pCar, 2);
		return 2;
	}

	//遇见一种情况，前轴上的时候一直压在轴识别器上，但是前面车下称了，本车再跟上
	//大秤重量识别为减小
	//第一个轴会被识别为减轴信号
	//PS: 这种情况已经在轴识别器处理时解决，信号已经是加轴，这里不考虑了


	//有可能退下一个轴，又上秤
	return 0;
}

//计算累计轴数
int AlexCountQueue(void)
{
	int i;
	int SumCount = 0;

	//计算轴数中和参数
	for(i = 0; i < m_nAlexCount; i++)
	{
		SumCount += AlexRecoderArr[i].direct;
	}

	return SumCount;
}

//计算累计轴数
static int AlexNormalCountQueue()
{
	int i;
	int SumCount = 0;

	//计算轴数中和参数
	for(i = 0; i < m_nAlexCount; i++)
	{
		SumCount += AlexRecoderArrNormal[i].direct;
	}

	return SumCount;
}

//在高速情况下，对轴做正常化处理
static void AlexCountByNormalizeHiSpeed(int nMinTime)
{
	int i;

	//设置第一个
	AlexRecoderArrNormal[0].direct = AlexRecoderArr[0].direct;
	AlexRecoderArrNormal[0].AlexKg = AlexRecoderArr[0].AlexKg;
	AlexRecoderArrNormal[0].nTimePulse = AlexRecoderArr[0].nTimePulse;
	AlexRecoderArrNormal[0].nTimeInterval = AlexRecoderArr[0].nTimeInterval;
	AlexRecoderArrNormal[0].bDouble = AlexRecoderArr[0].bDouble;

	//遍历后面的
	for(i = 1; i < m_nAlexCount; i++)
	{
		if(AlexRecoderArr[i].direct != AlexRecoderArrNormal[i-1].direct)
		{
			//规定时间内不能反轴
			if(AlexRecoderArrNormal[i-1].nTimeInterval < nMinTime)
			{
				AlexRecoderArrNormal[i].direct = AlexRecoderArrNormal[i-1].direct;
			}
			else
			{
				AlexRecoderArrNormal[i].direct = AlexRecoderArr[i].direct;
			}			
		}
		else
		{
			AlexRecoderArrNormal[i].direct = AlexRecoderArr[i].direct;
		}

		AlexRecoderArrNormal[i].AlexKg = AlexRecoderArr[i].AlexKg;
		AlexRecoderArrNormal[i].nTimePulse = AlexRecoderArr[i].nTimePulse;
		AlexRecoderArrNormal[i].nTimeInterval = AlexRecoderArr[i].nTimeInterval;
		AlexRecoderArrNormal[i].bDouble = AlexRecoderArr[i].bDouble;

	}
}

static int32 PraseCarAlexBigger3(CarInfo* pCar)
{
	int i, j;
	int SumCount = 0;
	int IncCount = 0;
	int SumNormalCount = 0;	
	int DecCount = 0;
	int nAlexRefMax;

	//过滤轴识别器抖动引起的误信号
	nAlexRefMax = 0;
	for(i = 0; i < m_nAlexCount; i++)
	{
		//寻找最大值
		if(AlexRecoderArr[i].AlexKgRef > nAlexRefMax)
		{
			nAlexRefMax = AlexRecoderArr[i].AlexKgRef;
		}
	}
	for(i = 0, j = 0; i < m_nAlexCount; i++)
	{
		//保留有效的轴信号
		//if(AlexRecoderArr[i].AlexKgRef * 10 >= nAlexRefMax)
		if(AlexRecoderArr[i].AlexKgRef * 28 >= nAlexRefMax)		//28主要是识别假轴，这个系数可以设置为可改
		{
			AlexRecoderArr[j] = AlexRecoderArr[i];
			j++;
		}
	}
	m_nAlexCount = j;


	//计算轴数综合参数
	for(i = 0; i < m_nAlexCount; i++)
	{
		SumCount += AlexRecoderArr[i].direct;
		if(AlexRecoderArr[i].direct > 0) IncCount++;
		if(AlexRecoderArr[i].direct < 0) DecCount++;
	}

	//处理全部为加轴的情况-----------------------------------
	if(DecCount == 0) 
	{
		for(i = 0; i < m_nAlexCount; i++)
		{
			pCar->AxleInfo[i].axle_wet = AlexRecoderArr[i].AlexKg;
			pCar->AxleInfo[i].axle_type = AlexRecoderArr[i].bDouble;
			pCar->AxleInfo[i].axle_len = AlexRecoderArr[i].nTimeInterval;
		}
		return SumCount;
	}

	//处理全部为减轴的情况
	if(IncCount == 0) return SumCount;

	//处理混合情况，如果第一个轴为加轴
	if(AlexRecoderArr[0].direct > 0)
	{
		//三秒钟以内出现反转，纠错
		 AlexCountByNormalizeHiSpeed(AXLE_ERR_TIME);	//3秒钟

		 SumNormalCount = AlexNormalCountQueue();
		
		 //处理轴数为1的特殊情况，这个情况实际不可能发生，如果存在，表明软件识别错误
		 if(SumNormalCount == 1)
		 {
			SumNormalCount = 0;
			if(IncCount > DecCount)
			{
				for(i = 0, j = 0; i < m_nAlexCount; i++)
				{				
					if(AlexRecoderArr[i].direct > 0)
					{
						AlexRecoderArrNormal[j].AlexKg = AlexRecoderArr[i].AlexKg;
						AlexRecoderArrNormal[j].bDouble = AlexRecoderArr[i].bDouble;
						AlexRecoderArrNormal[j].nTimeInterval = AlexRecoderArr[i].nTimeInterval;
						j++;
						SumNormalCount++;
					}
				}
			}
			else
			{
				for(i = 0, j = 0; i < m_nAlexCount; i++)
				{				
					if(AlexRecoderArr[i].direct > 0)
					{
						AlexRecoderArrNormal[j].AlexKg = AlexRecoderArr[i].AlexKg;
						AlexRecoderArrNormal[j].bDouble = AlexRecoderArr[i].bDouble;
						AlexRecoderArrNormal[j].nTimeInterval = AlexRecoderArr[i].nTimeInterval;
						j++;
						SumNormalCount++;
					}
				}
			}
		 }

		if(SumNormalCount > 6) SumNormalCount = 6;
		for(i = 0, j = 0; (i < SumNormalCount) && (j < m_nAlexCount); j++)
		{
			if(AlexRecoderArrNormal[j].direct > 0)
			{
				pCar->AxleInfo[i].axle_wet = AlexRecoderArrNormal[j].AlexKg;
				pCar->AxleInfo[i].axle_type = AlexRecoderArrNormal[j].bDouble;
				pCar->AxleInfo[i].axle_len = AlexRecoderArrNormal[j].nTimeInterval;
				i++;
			}
			else
			{
 				if(i > 0) i--;		//回退已保存的加轴重量
			}
		}

		return SumNormalCount;
	}
	//处理混合情况，如果第一个轴为退轴	
	else
	{
		if((DecCount == 1) && (IncCount == 2))
		{
			for(i = 1; i < m_nAlexCount; i++)
			{
				pCar->AxleInfo[i].axle_wet = AlexRecoderArr[i-1].AlexKg;
				pCar->AxleInfo[i].axle_type = AlexRecoderArr[i-1].bDouble;
				pCar->AxleInfo[i].axle_len = AlexRecoderArr[i-1].nTimeInterval;
			}
			return IncCount;
		}


		//如果秤台上只有他这一个车，此时秤台重量绝对值大于SCALER_BIG_CAR_MIN，认为是在上车，第一个轴设置为加轴
		if(CarQueue_Get_OnScaler_Count() == 1)
		{
			if(Get_Static_Weight() > SCALER_BIG_CAR_MIN)
			{
				AlexRecoderArr[0].direct = 1;
			}
		}

		//三秒钟以内出现反转，纠错
		 AlexCountByNormalizeHiSpeed(AXLE_ERR_TIME);	//3秒钟

		 SumNormalCount = AlexNormalCountQueue();

		if(SumNormalCount > 6) SumNormalCount = 6;
		for(i = 0, j = 0; (i < SumNormalCount) && (j < m_nAlexCount); j++)
		{
			if(AlexRecoderArrNormal[j].direct > 0)
			{
				pCar->AxleInfo[i].axle_wet = AlexRecoderArrNormal[j].AlexKg;
				pCar->AxleInfo[i].axle_type = AlexRecoderArrNormal[j].bDouble;
				pCar->AxleInfo[i].axle_len = AlexRecoderArrNormal[j].nTimeInterval;
				i++;
			}
			else
			{
 				if(i > 0) i--;		//回退已保存的加轴重量
			}
		}

		return SumNormalCount;
	}
}

static void PreAxle_Add_Tick(int nTime)
{
	if(m_nAlexCount >= ALEX_ARR_MAX) 
		return;

	if(m_nAlexCount > 0)
	{
		AlexRecoderArr[m_nAlexCount - 1].nTimeInterval = MsgTimeAdd(AlexRecoderArr[m_nAlexCount - 1].nTimeInterval, nTime);
	}
}

static void Empty_Scaler_Business(void)
{
	CarInfo* pCar = NULL;

	//秤台清零有风险，调用需谨慎
	//置零是可能刚好遇上错误AD值，导致零位出错
	//需要开启零位跟踪才有效
	Scaler_Set_Zero(TRUE);

	//对车辆队列进行处理
	pCar = CarQueue_Get_Head();
	while(pCar != NULL) 
	{
		if(pCar->nStatus < stCarWaitPay) 
		{
			//还没进入待收费的肯定不对
			CarQueue_Car_Kill(pCar);			
		}
		
		pCar = pCar->pNext;
	}
	CarQueue_Remove_Dead();
	Clear_Axle_Recoder();
}

//系统定时处理
static uint32 m_nScalerEmpetyCounter = 0;

void Clear_ScalerEmpty_Count(void)
{
	m_nScalerEmpetyCounter = 0;
}

void SysTimer_Business(void)
{
	//系统空闲计时
	if(Get_Static_Weight() < SCALER_EMPTY_WET) 
	{
		//增加时间
		m_nScalerEmpetyCounter = MsgTimeAdd(m_nScalerEmpetyCounter, CAR_TICK);

		//判断是否需要复位, 空秤且光幕没有被挡住
		if(m_nScalerEmpetyCounter >= SCALER_CLEAR_TIME) 
		{
			if(!Get_LC_Status(2)) 
			{
				//debug(Debug_Notify, "Empty Scaler Reset! tick = %ldms\r\n", Get_Sys_Tick());
				Empty_Scaler_Business();	//空秤处理
			}

			m_nScalerEmpetyCounter = 0;
		}
	}
	else 
	{
		m_nScalerEmpetyCounter = 0;
	}

	//处理轴定时器
	PreAxle_Add_Tick(CAR_TICK);
}

int32 Apply_LongCar_Axle(CarInfo *pCar)
{
	int i, j;
	int SumCount = 0;
	int IncCount = 0;
	int SumNormalCount = 0;	
	int DecCount = 0;
	int nAlexRefMax;

	//过滤轴识别器抖动引起的误信号
	nAlexRefMax = 0;
	for(i = 0; i < m_nAlexCount; i++)
	{
		//寻找最大值
		if(AlexRecoderArr[i].AlexKgRef > nAlexRefMax)
		{
			nAlexRefMax = AlexRecoderArr[i].AlexKgRef;
		}
	}
	for(i = 0, j = 0; i < m_nAlexCount; i++)
	{
		//保留有效的轴信号
		if(AlexRecoderArr[i].AlexKgRef * 10 >= nAlexRefMax)
		{
			AlexRecoderArr[j] = AlexRecoderArr[i];
			j++;
		}
	}
	m_nAlexCount = j;

	//计算轴数综合参数
	for(i = 0; i < m_nAlexCount; i++)
	{
		SumCount += AlexRecoderArr[i].direct;
		if(AlexRecoderArr[i].direct > 0) IncCount++;
		if(AlexRecoderArr[i].direct < 0) DecCount++;
	}

	//处理全部为加轴的情况-----------------------------------
	if(DecCount == 0)
	{
		for(i = 0; i < m_nAlexCount; i++)
		{
			pCar->AxleInfo[i].axle_wet = AlexRecoderArr[i].AlexKg;
			pCar->AxleInfo[i].axle_type = AlexRecoderArr[i].bDouble;
			pCar->AxleInfo[i].axle_len = AlexRecoderArr[i].nTimeInterval;
		}
		return SumCount;
	}

	//处理全部为减轴的情况
	if(IncCount == 0) return SumCount;

	//三秒钟以内出现反转，纠错
	 AlexCountByNormalizeHiSpeed(AXLE_ERR_TIME);	//3秒钟

	 SumNormalCount = AlexNormalCountQueue();

	if(SumNormalCount > 6) SumNormalCount = 6;
	for(i = 0, j = 0; (i < SumNormalCount) && (j < m_nAlexCount); j++)
	{
		if(AlexRecoderArrNormal[j].direct > 0)
		{
			pCar->AxleInfo[i].axle_wet = AlexRecoderArrNormal[j].AlexKg;
			pCar->AxleInfo[i].axle_type = AlexRecoderArrNormal[j].bDouble;
			pCar->AxleInfo[i].axle_len = AlexRecoderArrNormal[j].nTimeInterval;
			i++;
		}
		else
		{
 			if(i > 0) i--;		//回退已保存的加轴重量
		}
	}

	return SumNormalCount;
}

int32 Get_Axle_Weight(uint8 axle_num)
{
	return 0;
}

void Save_Car_AxleWet(CarInfo *pCar)
{
	return;
}

void Save_Axle_Info(AlexRecoder* pAxle)
{
	//记录轴信息
	memcpy(&m_AlexRecoder, pAxle, sizeof(AlexRecoder));
	m_AlexRecoder.nTimeInterval = 0;

	AddAlexRecoder(&m_AlexRecoder);
}

//车过完之后综合处理轴数据
int32 Apply_Car_Axle(CarInfo* pCar)
{
	if(pCar == NULL) return 0;

	//没收到轴信号处理
	if(m_nAlexCount == 0)
	{
		return PraseNoneAlexCar(pCar);
	}

	//对1个轴做处理
	if(m_nAlexCount == 1)
	{
		return PraseSingleAlexCar(pCar);
	}

	//处理2轴小车问题，主要是小车容易误判，重点关注对象
	if(m_nAlexCount == 2)
	{
		return PraseLittleCarAlex(pCar);
	}

	//处理3个轴以上的车
	return PraseCarAlexBigger3(pCar);
}






