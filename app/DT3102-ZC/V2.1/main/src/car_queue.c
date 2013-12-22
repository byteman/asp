#include <string.h>
#include "car_queue.h"
#include "car_business.h"
#include "scszc_debug_info.h"

#define MAX_CAR_NUM 			10
static  CarInfo     carArray[MAX_CAR_NUM];
static  CarInfo   *pCarHead=NULL;
static  CarInfo   *pCarTail = NULL;
static  int32 	m_nCarCount = 0;

//车辆信息复位
static void CarInfo_Reset(CarInfo* pCar)
{
	if(pCar == NULL) 
		return;

	memset(pCar, 0, sizeof(CarInfo));
}

//队列初始化
void CarQueue_Init(void)
{
	int i = 0;
	for(i = 0;  i  < MAX_CAR_NUM; i++)
	{
		carArray[i].bUse = FALSE;
		carArray[i].pNext = NULL;
		carArray[i].pPrev = NULL;		
	}

	m_nCarCount = 0;
	pCarHead = NULL;
	pCarTail = NULL;

}

//创建车辆
CarInfo* CarQueue_Create_Car(void)
{
	int i = 0;
	for(i = 0;  i  < MAX_CAR_NUM; i++)
	{
		if(carArray[i].bUse ==  FALSE)
		{
			CarInfo_Reset(&carArray[i]);
			return &(carArray[i]);
		}		
	}

	debug(Debug_Business, "carqueue full\r\n");
	//for(i = 0;  i < MAX_CAR_NUM; i++)
	//{
	//	if(carArray[i].bResponse)
	//	{
	//		CarInfo_Reset(&carArray[i]);
	//		debug(Debug_Business, "carqueue circle\r\n");
	//		return &(carArray[i]);
	//	}		
	//}
	
	return NULL;
}

//从队列尾添加车
void CarQueue_Add_Tail(CarInfo* pCar)
{
	if(pCar == NULL) return;

	if(pCarTail == NULL)
	{
		pCarTail    = pCar;
		pCarHead = pCar;
		pCar->bUse   = TRUE;
		pCar->pNext = NULL;
		pCar->pPrev = NULL;
	}
	else
	{
		pCar->bUse   = TRUE;
		pCar->pPrev = pCarTail;
		pCarTail->pNext = pCar;
		pCarTail = pCar;
		pCarTail->pNext = NULL;
	}
	m_nCarCount++;
}

//从队列头添加车
void CarQueue_Add_Head(CarInfo* pCar)
{
	if(pCar == NULL) return;
	
	if(pCarHead == NULL)
	{
		pCarTail    = pCar;
		pCarHead = pCar;
		pCar->bUse   = TRUE;
		pCar->pNext = NULL;
		pCar->pPrev = NULL;
	}
	else
	{
		pCar->bUse   = TRUE;
		pCar->pNext = pCarHead;
		pCarHead->pPrev= pCar;
		pCarHead = pCar;
		pCarHead->pPrev = NULL;
	}
	
	m_nCarCount++;
}

//设置车辆删除
void CarQueue_Car_Kill(CarInfo* pCar)
{
	if(pCar == NULL) return;

	pCar->bCarIsDead = TRUE;
	debug(Debug_Business, "Kill car! CarState: %s\r\n", Get_Car_StatusName(pCar->nStatus));
}

//判断车是否已删除
BOOL CarQueue_Car_Is_Dead(CarInfo* pCar)
{
	if(pCar == NULL) return FALSE;

	return pCar->bCarIsDead;
}

//移除要删除的车辆
void CarQueue_Remove_Dead(void)
{
	CarInfo* pIter = NULL;
	CarInfo* pCar = NULL;
	pIter = CarQueue_Get_Head();
	while(pIter != NULL)
	{
		pCar = pIter;
		pIter = pIter->pNext;

		if(pCar->bCarIsDead == TRUE)
		{			
			pCar->bUse = FALSE;

			if(m_nCarCount > 0) m_nCarCount--;
			
			if(pCar->pPrev != NULL)
			{
				pCar->pPrev->pNext = pCar->pNext;
			}
			else
			{
				pCarHead = pCar->pNext;
			}
			
			if(pCar->pNext!= NULL)
			{
				pCar->pNext->pPrev = pCar->pPrev;
			}
			else
			{
				pCarTail = pCar->pPrev;
			}
			
			pCar->pPrev =  NULL;
			pCar->pNext = NULL;
		}		
	}	
}

//移除车头
CarInfo*  CarQueue_Remove_Head(void)
{
	CarInfo* pTempCar = NULL;
	if(pCarHead == NULL) return NULL;

	pTempCar  = pCarHead;

	pCarHead = pCarHead->pNext;
	if(pCarHead == NULL) 
	{
		pCarTail = NULL;
	}
	else
	{
		pCarHead->pPrev = NULL;
	}

	if(m_nCarCount > 0)
		m_nCarCount--;

	pTempCar->bUse = FALSE;
	pTempCar->pNext = NULL;
	pTempCar->pPrev = NULL; 

	return pCarHead;	
}

//移除车尾
CarInfo*  CarQueue_Remove_Tail(void)
{
	CarInfo* pTempCar = NULL;
	if(pCarTail == NULL) return NULL;

	pTempCar  = pCarTail;

	pCarTail = pCarTail->pPrev;	
	if(pCarTail == NULL) 
	{
		pCarHead = NULL;
	}
	else
	{
		pCarTail->pNext = NULL;
	}

	if(m_nCarCount > 0) m_nCarCount--;

	pTempCar->bUse = FALSE;
	pTempCar->pNext = NULL;
	pTempCar->pPrev = NULL;

	return pCarTail;
}

//是否为尾车
BOOL  CarQueue_Car_Is_Tail(CarInfo* pCar)
{
	if(pCar == NULL) return FALSE;
	if(pCarTail == NULL) return FALSE;
	if(pCar == pCarTail) return TRUE;

	return FALSE;
}


//判断是否为头车
BOOL CarQueue_Car_Is_Head(CarInfo* pCar)
{
	if(pCar == NULL) return FALSE;
	if(pCarHead == NULL) return FALSE;

	if(pCar == pCarHead) return TRUE;

	return FALSE;
}

//判断队列是否为空
BOOL CarQueue_Is_Empty(void)
{
	if(pCarHead == NULL) return TRUE;

	return FALSE;
}

//获取队列车头
CarInfo* CarQueue_Get_Head(void)
{
	return pCarHead;
}

//获取队列车尾
CarInfo* CarQueue_Get_Tail(void)
{
	return pCarTail;
}

//获取队列中车辆个数
int32 CarQueue_Get_Count(void)
{
	int32  i = 0;
	CarInfo* pCar = NULL;
	pCar = CarQueue_Get_Head();
	while(pCar != NULL)
	{
		i++;
		pCar = pCar->pNext;
	}
	
	return i;
}

//获取在秤上的车的个数
int32 CarQueue_Get_OnScaler_Count(void)
{
	int32  i = 0;
	CarInfo* pCar = NULL;
	pCar = CarQueue_Get_Head();
	while(pCar != NULL)
	{
		if((pCar->nStatus >= stCarComing) && (pCar->nStatus <= stCarLeaving) && (!pCar->bCarIsDead)) 
		{
			i++;
		}

		pCar = pCar->pNext;
	}

	return i;
}

CarInfo *CarQueue_Get_OnScaler_FirstCar(void)
{
	CarInfo* pCar = NULL;
	
	pCar = CarQueue_Get_Head();
	while(pCar != NULL)
	{
		if((pCar->nStatus >= stCarComing) && (pCar->nStatus <= stCarLeaving)) 
		{
			break;
		}

		pCar = pCar->pNext;
	}

	return pCar;
}


//判断车辆状态是否
BOOL CarQueue_CarState_Is_Change(CarInfo* pCar)
{
	if(pCar == NULL) return FALSE;

	return pCar->bStateChanged;
}

#if 0
//移除车辆
BOOL CarQueue_Remove(CarInfo* pCar)
{
	if(pCar == NULL) return FALSE;
	if(pCarTail == NULL) return FALSE;
	if(pCarHead== NULL) return FALSE;
	if(pCar->bUse == FALSE) return FALSE;

	if(m_nCarCount > 0) m_nCarCount--;

	pCar->bUse = FALSE;

	if(pCar->pPrev != NULL)
	{
		pCar->pPrev->pNext = pCar->pNext;
	}
	else
	{
		//表明pCar是Head
		pCarHead = pCar->pNext;
	}
	
	if(pCar->pNext!= NULL)
	{
		pCar->pNext->pPrev = pCar->pPrev;
	}
	else
	{
		//表明pCar是Tail
		pCarTail = pCar->pPrev;
	}
	
	pCar->pPrev =  NULL;
	pCar->pNext = NULL;

	return TRUE;	
}


//获取系统中存在的车辆个数
int32 CarQueue_Get_LiveCount(void)
{
	int32  i = 0;
	CarInfo* pCar = NULL;
	pCar = CarQueue_Get_Head();
	while(pCar != NULL)
	{
		if(pCar->bCarIsDead == FALSE)
		{
			i++;
		}

		pCar = pCar->pNext;
	}
	
	return i;
}
#endif
//获取可以缴费，并且还未缴费的车辆个数
int32 CarQueue_Get_CanPayCount(void)
{
	int32  i = 0;
	CarInfo* pCar = NULL;
	pCar = CarQueue_Get_Head();
	while(pCar != NULL)
	{
		//车辆没有离开，或者车辆处于离秤缴费状态
		if((pCar->nStatus < stCarLeaving) || (pCar->nStatus == stCarFarPay)) 
		{
			if(pCar->bWetVaild == TRUE)
			{
				i++;
			}
		}

		pCar = pCar->pNext;
	}
	return i;
}




