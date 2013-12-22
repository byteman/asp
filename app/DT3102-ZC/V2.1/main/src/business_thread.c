/* 
 * File:    business_thread.c
 * Brief:   业务处理线程
 *
 * History:
 * 1. 2012-11-16 创建文件, 从杨承凯云南项目移植	    river
 * 2. 2012-11-28 删掉SysBusinessArray数组,效率不高  river
 * 3. 2012-12-4  修改长车分段称重模式 				river
 * 4. 2013-1-9   流程打印信息置事件发生之后打印
 * 5. 2013-1-16  增加车辆队列元素sdk回应标志和队列循环
 * 6. 2013-3-12  增加道闸自动控制, 从长明内蒙程序移植
 */
#include "includes.h"
#include "task_msg.h"
#include "app_msg.h"
#include "task_timer.h"
#include "car_queue.h"
#include "overtime_queue.h"
#include "car_business.h"
#include "axle_business.h"
#include "sys_config.h"
#include "weight_param.h"
#include "overtime_queue.h"
#include "axle_business.h"
#include "axletype_business.h"
#include "device_info.h"
#include "wet_algorim.h"
#include "io.h"
#include "softdog.h"
#include "scszc_debug_info.h"
#include "barriergate_ctl.h"

#if 0
static BOOL LC_ABNORMAL_FLAG = FALSE;

/**************************光幕异常处理****************************/
static void Set_LC_Abnormal(uint16 *msg_id)
{
    TaskMsg msg = {0};
    
	msg.msg_id = Msg_Device_Fault;				//post LC err
	Task_QPost(&BusinessMsgQ, &msg);
	
	LC_ABNORMAL_FLAG = TRUE;					//设置光幕故障
	if(*msg_id == Msg_GC1_Enter)
		*msg_id = Msg_LC2_Enter;
	debug(Debug_Warning, "Warning:前光幕异常!\r\n");
}

static void LC_Abnormal_Business(uint16 *msg_id)
{
	static uint8 flag = 0;
	static int32 car_status = stCarNone;
	CarInfo *pCar;

	// 光幕异常，以地感替代光幕
	if(LC_ABNORMAL_FLAG) {
		//如果光幕又好了, 2012-12-12, let's go!
		if(*msg_id == Msg_LC2_Enter || *msg_id == Msg_LC2_Leave) {
			*msg_id = Msg_None;			//先触发了地感,故忽略掉该次光幕信号
			LC_ABNORMAL_FLAG = FALSE;
			return;
		}
			
		if(*msg_id == Msg_GC1_Enter)
			*msg_id = Msg_LC2_Enter;
		if(*msg_id == Msg_GC1_Leave)
			*msg_id = Msg_LC2_Leave;
		return;
	}
	
	if(*msg_id == Msg_GC1_Enter) {
		pCar = CarQueue_Get_Tail();
		if(Get_LC_Status(2)) {							
			//地感触发，光幕被挡住；检查光幕一直有信号的异常情况
			if(pCar != NULL) {
				if(pCar->nStatus != stCarComing) 
					Set_LC_Abnormal(msg_id);
			} else {
				Set_LC_Abnormal(msg_id);	
			}
		} else {
			if(pCar != NULL) 	//地感触发，光幕没有被挡住，记录尾车状态
				car_status = pCar->nStatus;
			flag = 1;
		}
	}

	//结合前轴识别器检查光幕一直没有信号的情况
	if((*msg_id == Msg_PreAlex_Add) || (*msg_id == Msg_PreAlex_Remove)) {
		if(flag) {
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL) {
				Set_LC_Abnormal(msg_id);
			} else {
			 	if(pCar->nStatus != stCarComing) {		
			 		if(car_status != stCarInScaler) {
						Set_LC_Abnormal(msg_id);
						Create_New_Car(NormalCarCmd);	//光幕故障，车辆已过前轴，补一个新车
					}
				}			
			}
			flag = 0;
			car_status = stCarNone;
		}
	}	
}
#endif

static void Car_Begin_Comming(CarInfo *car, int8 cmd)
{
	CarInfo *headcar;

	car->cmd = cmd;
	car->nScalerKgComing = Get_Static_Weight();

	if(cmd != LongCarCmd) 
	{
		//单车上秤的判断只考虑秤台重量
		if(car->nScalerKgComing < 500) 
		{		
			car->WetFlag = SingleWetFlag;		//新车未入队，置单车标志
			Clear_ScalerMaxWeight();
			debug(Debug_Business, "~~Signal car coming, scaler_wet=%d!\r\n", car->nScalerKgComing);
		} 
		else 
		{
			headcar = CarQueue_Get_OnScaler_FirstCar();		//跟车保存秤头车辆信息
			if(headcar != NULL) 
			{		
				car->nHeadWetWhenComing = headcar->nBestKg;
				car->nHeadAlexsWhenComing = headcar->nBackAxleNum;
				car->nHeadStatusWhenComing = headcar->nStatus;		
				debug(Debug_Business, "~~Follow car, TotalCarNum=%d, scaler_wet=%d, scaler_first_car{downaxle=%d, wet=%d, %s}\r\n", 
					CarQueue_Get_Count(), car->nScalerKgComing, headcar->nBackAxleNum, headcar->nBestKg, Get_Car_StatusName(headcar->nStatus));
			} 
			car->WetFlag = FollowWetFlag;		
		}
		FSM_Change_CarState(car, stCarComing);
	} 
	else 
	{
		FSM_Change_CarState(car, stLongCar);
		debug(Debug_Business, "~~LongCar next segs\r\n");
	}
}

static void More_Car_Alarm(void)
{
	return;
}

static void Create_New_Car(int8 cmd) 
{
	CarInfo *newCar = NULL;
	//新车入队
	newCar = CarQueue_Create_Car();
	debug(Debug_Business, "\r\n>>新车入队\r\n");
	if(newCar != NULL) 
	{
		Car_Begin_Comming(newCar, cmd);
		CarQueue_Add_Tail(newCar);
	} 
	else 
	{
		debug(Debug_Error, "Error: No mem for new car!\r\n");
	}
}

static void FSM_CarFunction(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL))
		return;
		
	if(pCar->CarFSMFun != NULL)
		pCar->CarFSMFun(pCar, pMsg);
}


/**************************定时事件处理****************************/
//队列所有车辆响应定时事件处理
static void Car_Business_Timer(TaskMsg *msg)
{
	CarInfo *iter;

	iter = CarQueue_Get_Head();

	while(iter != NULL) 
	{
		FSM_CarFunction(iter, msg);
		iter = iter->pNext;
	}	
}

/**************红绿灯道闸控制******************/
static void TrafficSignal_Business(void)
{
	//道闸自动控制
	if(Get_BGFlag()) 
	{
		BarrierGate_Service();
	}
	
	//红绿灯根据设置的重量阈值控制
	if(Get_TrafficSignal_Flag())
	{
		if(Get_Static_Weight() > (int32)gWetPar.MaxTrafficWet)
		{
			//debug(Debug_Business, "------红灯亮-----\r\n");
			fPeripheral_Control(TrafficSignal, 1);
		}
		else
		{
			//debug(Debug_Business, "-----绿灯亮-----\r\n");
			fPeripheral_Control(TrafficSignal, 0);
		}
	}
}

/**************状态机系统处理******************/
void Sys_Business(TaskMsg *msg)
{
	CarInfo *pCar = NULL;
	AlexRecoder *tmpAxle = NULL;
	static BOOL force_save_flag = FALSE;
	static long lc_tick = 0;

	//忽略强制收尾后的光幕释放事件
	if(force_save_flag)	
	{			
		if(msg->msg_id == Msg_LC2_Leave) 
		{
			force_save_flag = FALSE;
			return;
		}
	}

	//将强制采集命令转换为光幕收尾信息
	if(msg->msg_id == Msg_Cmd_Force) 
	{
		force_save_flag = TRUE;
		msg->msg_id = Msg_LC2_Leave;
	}
	
	switch(msg->msg_id) 
	{
		case Msg_LC2_Enter:						//光幕触发事件
			//必须先处理前车轴数再清轴信息
			pCar = CarQueue_Get_Tail();
			FSM_CarFunction(pCar, msg);

			lc_tick = Get_Sys_Tick();
			Clear_Axle_Recoder();
			TyreQueue_Init();
		
			Create_New_Car(NormalCarCmd);
			More_Car_Alarm();
			break;
			
		case Msg_PreAlex_Add:					//加轴事件			
		case Msg_PreAlex_Remove:
			tmpAxle = (AlexRecoder *)msg->msg_ctx;
			if(tmpAxle == NULL) break;

			debug(Debug_Business, "\r\n>轴重:%d,轴峰值:%d,胎型:%d", tmpAxle->AlexKg,tmpAxle->AlexKgRef, tmpAxle->bDouble);

			tmpAxle->AlexKg = (tmpAxle->AlexKg > (int)((float)tmpAxle->AlexKgRef*0.5f) ? tmpAxle->AlexKg : tmpAxle->AlexKgRef);
			Save_Axle_Info(tmpAxle);		//保存轴信息
			pCar = CarQueue_Get_Tail();
			if(pCar != NULL)
			{
				FSM_CarFunction(pCar, msg);
			}
			break;
		
		case Msg_LC2_Leave:
			Clear_ScalerMaxWeight();
			BarrierGateAction_After_LC2DownOff();
			
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL) break;

			//保存命令号
			if(force_save_flag) pCar->cmd = ForceCarCmd;
			
			if(Get_Sys_Tick() - lc_tick < 100) 
			{
				CarQueue_Car_Kill(pCar);
				debug(Debug_Business, "Light curtain keep tick<100ms, distrub!\r\n");
				break;
			}
			FSM_CarFunction(pCar, msg);
			break;
			
		case Msg_BackAlex:			
		case Msg_GC2_Enter:
		case Msg_GC2_Leave:
			pCar = CarQueue_Get_OnScaler_FirstCar();
			if(pCar == NULL) break;
			FSM_CarFunction(pCar, msg);
			break;
		
		case Msg_Cmd_PlusSave:
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL) break;
			
			FSM_CarFunction(pCar, msg);
			if(!pCar->bLongCarReady) 
			{
				Clear_Axle_Recoder();						//创建分段采集长车下一段
				Create_New_Car(LongCarCmd);
			}
			break;

		case Msg_Cmd_PayNotify:
			pCar = CarQueue_Get_Head();
			if(pCar != NULL) 
			{
				//冗余处理,防止队列异常
				if(pCar->nStatus == stCarComing) 
				{
					CarQueue_Car_Kill(pCar);
					if(pCar->pNext != NULL)
						FSM_CarFunction(pCar->pNext, msg);
				} 
				else 
				{
					FSM_CarFunction(pCar, msg);
				}
			}
			
			break;

		case Msg_Cmd_Repeat:
			pCar = CarQueue_Get_Head();		
			Car_Business_RepeatCmd(pCar, msg);
			break;
			
		case Msg_Tick:
			SysTimer_Business();
			Car_Business_Timer(msg);
			break;

		case Msg_WBAxle:
			Car_Business_WBAxle();
			break;
			
		default:
			break;
	}
		
	CarQueue_Remove_Dead();
}

static int B_Handle;
void Business_Thread(void *arg)
{
	TaskMsg event_msg = {0};
	uint8 err;

	BarrierGate_Reset_Init();
	B_Handle = Register_SoftDog(300, SOFTDOG_RUN, "业务线程");

	while(1) 
	{
		err = Task_QPend(&BusinessMsgQ, &event_msg, sizeof(TaskMsg), 0);	
		if(err == SYS_ERR_NONE) 
		{			
//			LC_Abnormal_Business(&event_msg.msg_id);

			if(B_Handle >= 0) SoftDog_Feed(B_Handle);
			
			//空秤定时器清零
			if(event_msg.msg_id != Msg_Tick)
			{
				Clear_ScalerEmpty_Count();
			}
			else
			{
				TrafficSignal_Business();
			}
			
		 	Sys_Business(&event_msg);
			MsgInfo_Print(&event_msg);			//打印事件消息,在事件发生后打印
		}
	}
}

