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

#define MAX_AXLE_NUM    12


struct  _CarInfo;
typedef struct  _CarInfo  CarInfo;
typedef void (*CarBusinessFun)(CarInfo *pCar, TaskMsg *pMsg);

struct _AxleInfo 
{
	int32	axle_wet;
	int32	axle_peak;
	int32   axle_len;
	char 	axle_type;
};

struct  _CarInfo
{
	struct  _CarInfo* pPrev;
	struct  _CarInfo* pNext;
	struct  _CarInfo* pHeadWhenComing;
	CarBusinessFun CarFSMFun;
	
	BOOL	bUse;						//单元是否已使用
	BOOL	bCarIsDead;					//单元是否已删除
	BOOL	bResponse;					//车辆重量是否已发送
	BOOL	bWetVaild;					//数据是否有效
	BOOL	bConfirm;					//是否已经分车
	BOOL	bAckErr;					//ack abnormal data
	BOOL	bFollowToSingle;			//前车离开后车未断尾标志位
	uint8   WetLevel;				    //取重量置信度
	uint8	cmd;						//命令号
	uint8	CutOffState;				//车辆断尾状态
	uint8   nStatus;					//车辆状态
	uint8	nPreStatus;					//上一个状态
	uint8	nNextStatus;				//预先记录下一个状态的去向
	int32   nStatusKeepTime;			//对状态进行计时
	int32	nLiveTime;					//汽车在系统中的存活时间
	int32   nScalerKgComing;			//光幕触发时的秤台稳定重量
	int32   nScalerKgDyncComing;		//光幕触发时的秤台动态重量
	int32	nScalerKgIn;				//光幕释放时的秤台稳定重量
	int32   nBestKg;					//车辆重量
	int32 	nLongCarHalfWet;			//长车自动称重前半截重量
	int32	nDownAlexMaxKg;				//前一个下秤轴信号最大值
	
	int8	nAlexMsgCount;				//轴消息数
	int8    nAxleNum; 					//车辆轴数
	int8    nBackAxleNum;				//车辆下秤轴数
	int8	nLongCarHalfAxle;			//长车前半截轴数
	int8    nLongCarSegAxle;			//分段长车上传的轴数

	int8    nFollowAxleNum;			//前车离开后车未断尾时的轴数

	//长车用 
	uint8  nLongCarSegs;				//长车分段数

	uint16	 year;						//时间
	uint8	 mon;
	uint8	 day;
	uint8	 hour;
	uint8	 min;
	uint8	 sec;
	uint16  speed; 

	//用于计算收尾后动态均值
	int fDyncMaxKg;
	int fDyncMinKg;
	int fDyncAvgKg;
	int fDyncSumKg;
	int fDyncMaxLastKg;
	int   nDyncKgIndex;
	int	  nDyncKgCount;
	uint8 bDyncMaxOk;
	uint8 bDyncMinOk;
	uint8 bDyncValid;
	uint8 bDyncRaise;


	//轴重
	struct _AxleInfo AxleInfo[MAX_AXLE_NUM];
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

/* 获取待缴费车辆个数 */
int32 CarQueue_Get_CanPayCount(void);

/* 获取秤台上第一辆车 */
CarInfo *CarQueue_Get_OnScaler_FirstCar(void);

/*获取第一辆在秤中的车*/
CarInfo *CarQueue_Get_InScaler_FirstCar(void);

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

