#ifndef AXLETYPE_BUSINESS_H
#define AXLETYPE_BUSINESS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dt3102_io.h"

typedef struct _TyreDevice 
{
	int32  direct;				
	int32  nCntInterval;		//记录到下一个轮胎的间隔时间
	int32  nCntPulse;			//脉宽时间，记录轴压在轮胎识别器上的时间
	char   bDouble;				//单双轮
}TyreDevice;


void AxleType_Business(sAxisType1_State_TypeDef *status);
void Car_Business_WBAxle(void);


//轮胎识别器调试信号
char Get_TyreDevice_Event(void);
char Get_TyreDevice_bDouble(void);



//轮胎数据循环队列
int TyreQueue_Get(TyreDevice *data);
void TyreQueue_Put(TyreDevice *data);
u32 TyreQueue_len(void);
int TyreQueue_Get_Rear(TyreDevice *data);
void TyreQueue_Init(void);


#ifdef __cplusplus
}
#endif

#endif

