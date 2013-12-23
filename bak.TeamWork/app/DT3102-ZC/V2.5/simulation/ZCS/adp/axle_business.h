/* 
 * File:    axle_business.h
 * Brief:   car's axle business
 *
 * History:
 * 1. 2012-11-27 创建文件, 从杨承凯云南项目移植	 river
 */

#ifndef AXLE_BUSINESS_H
#define AXLE_BUSINESS_H

#include "includes.h"
#include "car_queue.h"

typedef struct
{
	int32  direct;
	int32  AlexKg;	
	int32  AlexKgRef;			//轴重峰值
	int32  AlexRightTopWet;
	int32  AlexRightWet;
	int32  nTimeInterval;		//记录到下一个轴的间隔时间
	int32  nTimePulse;			//脉宽时间，记录轴压在轴识别器上的时间
	BOOL   bDouble;				//单双轮
}AlexRecoder;


int32 Apply_Car_Axle(CarInfo* pCar);					//返回轴数
int32 Apply_LongCar_Axle(CarInfo *pCar);

void Save_Axle_Info(AlexRecoder* pAxle);
void SysTimer_Business(void);



//清除轴记录
void Clear_Axle_Recoder(void);

int32 Get_Axle_Weight(uint8 axle_num);
void Save_Car_AxleWet(CarInfo *pCar);




#endif
