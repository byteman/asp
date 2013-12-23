/* 
 * File:    overtime_data.h
 * Brief:   overtime carinfo queue
 *
 * History:
 * 1. 2012-11-14 创建文件，定义接口 river
 * 2. 2012-11-24 实现队列 吴君
 */
#ifndef OVERTIME_DATA_H
#define OVERTIME_DATA_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"

#define MAX_AXIS_NUM 20

typedef struct  {
	BOOL	 SendMod;		//主动发送 被动发送标志位
	uint8	 cmd; 			//命令号
	uint8	 bSendAttr;	//发送属性: 主动 被动
	uint8	 SendNum;  		//发送次数
	uint16   SerialID;		//流水号
	uint8	 bSend;			//发送标志
	uint8    bSave;			//车辆存储标志
	uint8	 LongCarSegs; 	//长车段数， 段数大于0时为长车
	uint16	 year;	    	//时间
	uint8	 mon;
	uint8	 day;
	uint8	 hour;
	uint8	 min;
	uint8	 sec;
	uint8	 CarStatus;				//车辆状态字(断尾状态)
	uint8	 CarDirection;				//车辆行进方向(双秤台)
	uint8  	 ScalerOverWetFlag;		//秤台超重标志
	uint8	 OverWetFlag;				//车辆超重标志
	uint32   TotalWet;                 //总重	
	uint8	 AxleNum;                  	//轴数
	uint8	 AxleGroupNum;				//轴组数
	uint16   AxleWet[MAX_AXIS_NUM];  	//轴重数组
	uint8	 AxleType[MAX_AXIS_NUM];	//轴型, 单双轮
	uint8	 AxleGroupType[MAX_AXIS_NUM]; 	//轴组类型
	uint16	 AxleLen[MAX_AXIS_NUM];			//轴间距
	uint16   speed;					   		//车速
	uint16	 speeda;						//加速度
	uint16	 sCheck;						//以上信息的校验和, 存储校验使用
}OvertimeData;


//车辆信息命令
enum {
	IORequstCmd = 1,
	NormalCarCmd,
	BackCarCmd,
	RepeatCarCmd,
	ForceCarCmd,
	LongCarCmd,
	PayNoticeCmd,
	IOContrlCmd,
	TimeSetCmd,
	USARTSDKTest,
	ACKStaticWetAndAxle,	//璧山项目
	ClearLastCarAxle,			//璧山项目-清除最后一个车的轴数
	SetZeroCmd						//璧山项目-置零
};

enum {
	SendActiveMod = 1,		//主动发送
	SendUnActiveMod		//被动发送
};

/* 队列初始化*/
void OverQueue_Init(void);

//获取序列号
unsigned short OverQueue_GetSeriID(void);

/* 数据入队 */
int OverQueue_Enqueue(OvertimeData *data);

/* 移除队头元素 */
void OverQueue_Remove_Head(void);

/* 移除队尾元素 */
void OverQueue_Remove_Tail(void);

/* 获取队头元素 */
OvertimeData *OverQueue_Get_Head(void);

/* 获取队尾元素 */
OvertimeData *OverQueue_Get_Tail(void);

/* 增加队头元素发送次数 */
void OverQueue_Add_HeadSendNum(void);

/* 获取队头元素发送次数 */
int OverQueue_Get_HeadSendNum(void);

/* 队列是否为空 */
//BOOL OverQueue_Empty(void);

//获取队列长度
int OverQueue_Get_Size(void);

OvertimeData *OverQueue_Get_Index(uint8 index);


#ifdef __cplusplus
}
#endif

#endif



