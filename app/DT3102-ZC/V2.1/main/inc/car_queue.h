/* 
 * File:    car_queue.h
 * Brief:   carinfo FSM queue
 *
 * History:
 * 1. 2012-11-16 创建函数， 从杨承凯云南项目移植	 river
 */

#ifndef CAR_QUEUE_H
#define CAR_QUEUE_H

#include "includes.h"
#include "app_msg.h"

#define MAX_AXLE_NUM    20

struct  _CarInfo;
typedef struct  _CarInfo  CarInfo;
typedef void (*CarBusinessFun)(CarInfo *pCar, TaskMsg *pMsg);

struct _AxleInfo 
{
	int32	axle_wet;
	int32   axle_len;
	char 	axle_type;
};

struct  _CarInfo
{
	struct  _CarInfo* pPrev;
	struct  _CarInfo* pNext;
	struct  _CarInfo* pHeadWhenComing;
	CarBusinessFun CarFSMFun;
	
	BOOL	bUse;	  
	BOOL	bResponse;					//是否已发送
	BOOL	bCarIsDead;			
	BOOL	bStateChanged;				//车辆状态改变标志
	BOOL	bAxleValid;				//轴有效标志
	BOOL	bWetVaild;					//数据是否有效
	BOOL	bWetTimeFlag;				//取重量延时标志

	uint8	cmd;						//命令号
	uint8	CutOffState;				//车辆断尾状态
	uint8   nStatus;					//车辆状态
	uint8	nPreStatus;				//上一个状态
	int32   nStatusKeepTime;			//对状态进行计时
	int32	nLiveTime;					//汽车在系统中的存活时间
	int32   nHeadStatusWhenComing;		//光幕触发时头车状态
	int32   nHeadWetWhenComing;			//光幕触发时头车的重量
	int32	nHeadAlexsWhenComing;      	//光幕触发时头车已经下秤的轴数
	int32   nScalerKgComing;			//光幕触发的秤台重量
	int32	nScalerKgDown;				//光幕释放时的秤台重量
	
	uint8   WetLevel;				    //取重量置信度
	uint8   WetFlag;                  	//取重量类型
	int32   nWetKeepTime;				//重量延时计时;

	int8    nAxleNum; 					//车辆轴数
	int8    nBackAxleNum;				//车辆下秤轴数
	int32   nBestKg;					//车辆重量
	int32 	nLongCarHalfWet;			//长车自动称重前半截重量
	int8	nLongCarHalfAxle;			//长车前半截轴数

	//长车用 
	BOOL   bLongCarReady;				//长车分段称重完成标志
	uint8  nLongCarSegs;				//长车分段数

	uint16	 year;						//时间
	uint8	 mon;
	uint8	 day;
	uint8	 hour;
	uint8	 min;
	uint8	 sec;
	uint16  speed; 
	struct _AxleInfo AxleInfo[MAX_AXLE_NUM];		//轴重
}; 

//车辆重量状态
enum {
	NoneWetFlag = 0,
	SingleWetFlag,
	SingleUnstableFlag,
	FollowWetFlag
};
	
/* 队列初始化 */
void CarQueue_Init(void);

/* 创建车辆, 返回车辆信息指针 */
CarInfo* CarQueue_Create_Car(void);

/* 从队尾入队一辆车 */
void CarQueue_Add_Tail(CarInfo* pCar);

/* 从队头入队一辆车 */
void CarQueue_Add_Head(CarInfo* pCar);

/* 移除队头车辆, 返回队头车辆指针 */
CarInfo*  CarQueue_Remove_Head(void);	

/* 移除队尾车辆, 返回队尾车辆指针 */
CarInfo*  CarQueue_Remove_Tail(void);	

/* 判断是否是尾车 */
BOOL CarQueue_Car_Is_Tail(CarInfo* pCar);

/* 判断是否是头车 */
BOOL CarQueue_Car_Is_Head(CarInfo* pCar);

/* 判断队列是否为空　*/
BOOL CarQueue_Is_Empty(void);				

/* 获取队头车辆指针 */
CarInfo* CarQueue_Get_Head(void);

/* 获取队尾车辆指针 */
CarInfo* CarQueue_Get_Tail(void);

/* 判断车辆状态是否改变 */
BOOL CarQueue_CarState_Is_Change(CarInfo* pCar);

int32 CarQueue_Get_CanPayCount(void);


CarInfo *CarQueue_Get_OnScaler_FirstCar(void);


/* 设置车辆删除标志bIsDead */
void CarQueue_Car_Kill(CarInfo* pCar);

//判断车是否已删除
BOOL CarQueue_Car_Is_Dead(CarInfo* pCar);


/* 删除队列里设置了删除标志的车辆 */
void CarQueue_Remove_Dead(void);

/* 返回队列车辆车数 */
int32 CarQueue_Get_Count(void);

/* 返回在秤上的车数 */
int32 CarQueue_Get_OnScaler_Count(void);


#endif

