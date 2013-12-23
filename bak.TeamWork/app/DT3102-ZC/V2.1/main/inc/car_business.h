/* 
 * File:    car_business.h
 * Brief:   car business
 *
 * History:
 * 1. 2012-11-24 创建文件, 从杨承凯云南项目移植	 river
 */

#ifndef CAR_BUS_H
#define CAR_BUS_H

#include "car_queue.h"
#include "app_msg.h"

//#define CAR_STCUTOFF_TIME	800			//车辆断尾状态延时时间(ms)
#define CAR_TICK			10		    //车辆状态计时单位10ms
#define USCAR_WET_TICK		1000
//#define SCAR_WET_TICK     2000


#define SCALER_MIN_KG		500

//CarStatus定义
//注意排序原则：从车辆从进入到离开排序
//如果要增加状态，必须遵循此原则
typedef enum
{
	stCarNone = 0,

	//以下状态表示车在秤上============================
	stCarComing,		//车辆开始上称状态

	stCarEnter,			//车辆进入

	stCarInScaler,		//车辆完全处于称台上

	stCarWaitPay,		//待收费状态,车辆触发后地感

	stLongCar,			//长车在称台上，头部已经触发后地感

	stCarLeaving,		//车辆离开,按下了计费成功按键后

	//以上状态表示车在秤上============================

	stCarFarPay,		//离秤缴费	

	stDead				//已删除
}CarStatus;

typedef enum
{
	LevelNoneCar=0,

	//跟车延时1.2秒后取值
	LevelMultiCar = 100,

	//长车自动称重取CutOff重量
	LevelAutoLongCar,	

	//单车情况下，后车触发光幕
	LevelSingleCarBy,	

	//单车准备下秤，触发了后地感
	LevelSingleCarBeginDown,

	//长车分段
	LevelLongCar,

	//单车断尾延时1.2s稳定
	LevelSingleCarStable	
}CarKgLevel;

BOOL IsSingleCarComming(CarInfo *pCar);

void CarSetBestKg(CarInfo *pCar, int nKg, unsigned char nLevel);

void FSM_Change_CarState(CarInfo *pCar, int32 state);

void Car_Business_RepeatCmd(CarInfo *pCar, TaskMsg *msg);

void Send_Simulate_CarInfo(uint8 axle, int32 wet, int cmd);

//sdk主动请求命令异常数据回应
void AbnormalAckInfo_To_SDK(uint8 cmd, uint8 CutOffState);


void LongCarReset(void);
BOOL IsLongCar(void);
void LongCarRecognize(int32 nWet);

#endif

