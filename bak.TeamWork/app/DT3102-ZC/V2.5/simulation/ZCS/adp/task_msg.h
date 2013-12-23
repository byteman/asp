/* 
 * File:    msg.h
 * Brief:   task message defined
 *
 * History:
 * 1. 2012-11-14 创建文件 river 
 */


/* 
 * File:    task_msg.h
 * Brief:   task message defined
 */
#ifndef TASK_MSG_H
#define TASK_MSG_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"

#define SYS_MEM		int
#define SYS_EVENT	int

typedef struct _MsgQ 
{
	SYS_MEM *pMem;
	SYS_EVENT *pEvent;
}MsgQ;

int8 Task_Msg_Create(MsgQ *pMsgQ, void ** pSysQ, void *pMemBlock, uint32 nMsgCount, uint32 nMsgSize);

uint8 Task_QPost(MsgQ *pMsgQ, void *pMsg);

uint8 Task_QPostFront(MsgQ *pMsgQ, void *pMsg);

uint8 Task_QPend(MsgQ *pMsgQ, void *pMsg, int len, uint16 timeout);


#ifdef __cplusplus
}
#endif

#endif














//#ifndef MSG_H
//#define MSG_H
//	 
//#ifdef __cplusplus
//	 extern "C" {
//#endif
//
//#include "includes.h"

//任务消息号基址，根据需要修改
//enum {
//	Msg_Data_Base		= 100,
//	Msg_Business_Base = 200,
//	Msg_NetSend_Base	= 300,
//};

//业务处理线程接收消息
//enum{
//	Msg_None  =  Msg_Business_Base,
//
///*整车式消息号*/
//	Msg_Reset,			//系统复位
//	Msg_Refresh,		//系统刷新
//	Msg_GC1_Enter,		//前地感触发
//	Msg_GC1_Leave,		//前地感下沿
//	Msg_GC2_Enter,		//后地感
//	Msg_GC2_Leave,		
//	Msg_LC1_Enter,		//前光幕上沿
//	Msg_LC1_Leave,		//前光幕下沿
//	Msg_LC2_Enter,	  	//分车光幕
//	Msg_LC2_Leave,
//	Msg_PreAlex_Add,		//前轴识别器进轴
//	Msg_PreAlex_Remove,	//前轴识别器倒轴
//	Msg_BackAlex,			//后轴识别器进轴
//	Msg_Cmd_Repeat,		//重新采集
//	Msg_Cmd_PlusSave,		//分段采集命令
//	Msg_Cmd_PayNotify,
//	Msg_Cmd_Force,			//强制采集命令
//	Msg_Tick				//10ms时钟
//
///*双秤台消息号*/
//};
//
////业务数据管理线程接收消息
//enum {
//	Msg_Device_Heart = Msg_Data_Base,	 //10s定时上报外设状态, 秤上有车不上传
//	Msg_Device_Fault,		//外设即时故障信息
//	Msg_Overtime,			//超时信息
//	Msg_BackCarInfo,		//倒车信息
//	Msg_CarInfo_Ready		//车辆信息
//};
//
////网络发送线程接收消息
//#define Msg_NetSend  Msg_NetSend_Base
//
////各任务OS_EVENT
//enum {
//	MSG_NET_ID = 0,			//网络消息队列
//	MSG_BUSINESS_ID,			//业务处理消息队列
//	MSG_DATASEND_ID,			//业务数据管理消息队列
//
//	MSG_COUNT
//};
//
//#define TaskMsgNetLen		4
//
//#define TaskMsgBusinessLen	32
//#define TaskMsgDataSendLen	24
//#define TaskMsgBufLen			TaskMsgNetLen+TaskMsgBusinessLen+TaskMsgDataSendLen
//	
//typedef struct 
//{
//	uint32 priv;		//用户自定义指针
//	uint16 msg_id;		//消息号
//	uint16 msg_len;		//数据长度
//	uint32 msg_ctx;		//数据指针
//}TaskMsg;
//
//void Task_Msg_Init(void);
//
// /* 投递消息 */ 
//uint8 Task_QPost(uint8 task_msg_id, TaskMsg* pMsgInfo);
//
///* 等待消息 */ 
//void* Task_QPend(uint8 task_msg_id, uint16 to, uint8 *err);
//
///* 发送传感器ad值与设备状态  */
//BOOL Net_Send_Wave(void *buf);
//
///* 接收到消息释放内存 */
//uint8 Wave_Mem_Free(void *pblk);
//
//
//
////uint8 Task_SemPost(uint8 task_msg_id);
////void Task_SemPend(uint8 task_msg_id, uint16 to, uint8 *err);
//
//
//#ifdef __cplusplus
//}
//#endif
//
//#endif
//
