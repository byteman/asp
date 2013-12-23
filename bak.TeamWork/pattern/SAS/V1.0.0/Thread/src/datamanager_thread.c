#include "includes.h"
#include "task_timer.h"
#include "task_msg.h"										//消息队列
#include "app_msg.h"
#include "serial_protocol.h"								//串口协议解析
#include "sdk_serial.h"
#include "overtime_queue.h"									//待发送车辆数据队列操作
#include "data_store_module.h"							//车辆数据分区存储模块
#include "sys_param.h"									//存储在铁电里的满量程数据
#include "debug_info.h"
#include "device_info.h"
#include "task_timer.h"

//与sdk通信协议流水号,范围60000~65535; 1~59999由车辆信息与倒车信息使用
static uint16 OtherSerialID = 60000;
static uint16 Get_SerialID(void)
{
	uint16 tmp;

	tmp = OtherSerialID;
	OtherSerialID++;

	//回绕
	if(OtherSerialID == 0) OtherSerialID = 60000;

	return tmp;
}

static void Data_Manager_Business(TaskMsg *msg)
{
	OvertimeData *pCar = NULL;
	uint16 type = msg->msg_id;

	switch(type)										//根据不同的消息号进行流程处理
	{
		case Msg_Device_Fault:							//外设即时故障信息
		case Msg_Device_Heart:							//10s定时上报外设状态, OverQueue为空时上传
			Send_SDK_Ack(0, 0x01, Get_SerialID(), 0);
			break;
			
		case Msg_Overtime:								//超时信息
			//1、队头元素组包发送
			pCar = OverQueue_Get_Head();				//获取队头车辆数据
			if(pCar == NULL) break;
			
			Send_SDK_OverQueueData(pCar);
			//增加发送次数
			if(pCar->SendNum < 255) pCar->SendNum++;
			break;
			
		case Msg_BackCarInfo:							//倒车信息
		case Msg_CarInfo_Ready:							//车辆信息（准备完成通知）	
			//==============================================
			//获取队头车辆数据,发送到sdk
			pCar = OverQueue_Get_Head();							
			if(pCar == NULL) 
			{
				debug(Debug_Business, "ERROR: OverQueue_Get_Head NULL!");
				break;
			}

			if(pCar->SendMod == SendActiveMod)
			{	
				debug(Debug_Business, "@@Send carinfo to SDK,cmd:%d,SendNum:%d,OverQueueLen:%d\r\n", 
					pCar->cmd, pCar->SendNum,OverQueue_Get_Size());

				//如果队头车辆未发送,则立即发送车辆信息,否则由超时机制发送
				if(pCar->bSend == 0)
				{
					Send_SDK_OverQueueData(pCar);

					//增加发送次数
					if(pCar->SendNum < 255) pCar->SendNum++;

					//复位超时计数
					Reset_OvertimeTick();
				}
			}
			//轴数为0不用存储
			if(pCar->AxleNum > 0) 
			{				
				//==============================================
				//获取队尾车辆数据, 存储到flash
				pCar = OverQueue_Get_Tail();	
				if(pCar == NULL) 
				{
					debug(Debug_Business, "ERROR: OverQueue_Get_Tail NULL!");
					break;
				}
				
				//如果队尾车辆超限,存储超限车辆数据
				if(pCar->ScalerOverWetFlag)		
				{
					if(pCar->bSave == 0)
						Store_Overweight_Vehicle(pCar, sizeof(OvertimeData));	
				}

				//存储车辆信息到历史车辆信息区域
				if(pCar->cmd != BackCarCmd)
				{
					if(pCar->bSave == 0)
						Store_Normal_Vehicle(pCar, sizeof(OvertimeData));	
				}
				
				//设置存储标志
				if(pCar->bSave == 0) 
				{
					debug(Debug_Business, "@@Store CarInfo completed!\r\n");
					pCar->bSave = 1;		
				}
				debug(Debug_Business, "\r\n");
			}

			pCar = OverQueue_Get_Head();	
			if(pCar != NULL)
			{
				//被动发送的车辆数据立即删掉
				if(pCar->bSendAttr) OverQueue_Remove_Head();
			}	
			break;
			
		case Msg_USARTSDKTest:
			Send_SDK_Ack(0, USARTSDKTest, 0, 0);
			break;
			
		default: 
			break;
	}
}

void DataManager_Thread(void *arg)
{
	uint8 err;
	TaskMsg msg = {0};											

	while(1) 
	{
		err = Task_QPend(&DataManagerMsgQ, &msg, sizeof(TaskMsg), 0);		
		if(err == SYS_ERR_NONE) 
		{
			Data_Manager_Business(&msg);
		}
	}
}

