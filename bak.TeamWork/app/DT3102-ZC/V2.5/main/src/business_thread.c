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
#include "speed_check.h"
#include "debug_info.h"

#define PAY_TIME_OUT_TICK	10000

static volatile int nPayTimeOutTick = 10000;			//开机初始化为饱和值
static volatile int nPayTimeOutThreshold = 10000;

//排队时间复位
void ResetPayTimeOutTick(void)
{
	nPayTimeOutTick = 0;
}

void SetPayTimeOutThreshold(int tick)
{
	nPayTimeOutThreshold = tick;
}

//排队计时
void Car_Confirm_Wet(CarInfo *pCar);
void TickPayTimeOut(void)
{
	CarInfo *pCar;

	if(nPayTimeOutTick < nPayTimeOutThreshold)
	{
		nPayTimeOutTick += CAR_TICK;
		if(nPayTimeOutTick == nPayTimeOutThreshold)
		{
			//多线程运行，要直接处理，不能发消息，如果发消息可能产生排队误差
			//有可能刚发出消息然后再ClosePayTimeOut()，导致ClosePayTimeOut()失效
			//msg.msg_id = Msg_Pay_TimeOut;
			//Task_QPost(&BusinessMsgQ, &msg);

			if(Get_FollowCarMode() == DevideAtOnce) return;	//上秤分车模式不响应该机制

			//寻找应该处理该事务的车辆
			pCar = CarQueue_Get_InScaler_FirstCar();

			//如果没有车在秤中，推迟2秒钟
			if(pCar == NULL) nPayTimeOutTick -= 200;

			//确认并上传车辆重量
			Car_Confirm_Wet(pCar);
		}
	}
}

//关闭缴费超时
void ClosePayTimeOut(void)
{
	nPayTimeOutTick = PAY_TIME_OUT_TICK;
	nPayTimeOutThreshold = PAY_TIME_OUT_TICK;
}

//是否到达排队时间
BOOL IsPayTimeOut(void)
{
	if(nPayTimeOutTick < PAY_TIME_OUT_TICK) return FALSE;

	return TRUE;
}



#define AT_TURN_TICK	1000
int nTurnTick = 1000;	//开机初始化为无

//排队时间复位
void ResetTurnTick(void)
{
	nTurnTick = 0;
}

//排队计时
void TickTurn(void)
{
	TaskMsg msg = {0};

	if(nTurnTick < AT_TURN_TICK)
	{
		nTurnTick += CAR_TICK;
		if(nTurnTick == AT_TURN_TICK)
		{
			msg.msg_id = Msg_At_Turn;
			Task_QPost(&BusinessMsgQ, &msg);
		}
	}
}

//是否到达排队时间
BOOL IsAtTurn(void)
{
	if(nTurnTick < AT_TURN_TICK) return FALSE;

	return TRUE;
}

#define DOWN_ALEX_TIME_GATE	200
int nDownAlexTick = 0;
void ResetDownAlexTick(void)
{
	nDownAlexTick = 0;
}

void TickDownAlex(void)
{
	if(nDownAlexTick <= DOWN_ALEX_TIME_GATE)
	{
		nDownAlexTick++;
	}
}

static void Car_Begin_Comming(CarInfo *pCar, int8 cmd)
{
	if(pCar == NULL) return;

	pCar->cmd = cmd;
	pCar->nScalerKgComing = Get_Static_Weight();
	pCar->nScalerKgDyncComing = Get_Dynamic_Weight();

	//单车上秤的判断只考虑秤台重量
	if((pCar->nScalerKgComing < SCALER_MIN_KG) || (pCar->nScalerKgDyncComing < SCALER_MIN_KG))
	{
		debug(Debug_Business, "单车触发, 秤台重量=%d\r\n", Get_Dynamic_Weight());
	}
	else
	{
		debug(Debug_Business, "跟车触发, 秤台重量=%d\r\n", pCar->nScalerKgComing);
	}
	FSM_Change_CarState(pCar, stCarComing);
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
	debug(Debug_Business, "**创建车辆\r\n");
	if(newCar != NULL) 
	{
		Car_Begin_Comming(newCar, cmd);
		CarQueue_Add_Tail(newCar);
	} 
	else 
	{
		debug(Debug_Error, "错误: 车辆队列满!\r\n");
	}
}

void FSM_CarFunction(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL))
		return;

	if(pCar->bCarIsDead == TRUE)
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

/**************状态机系统处理******************/
extern int IsAlexHold(void);
extern int Car_Confirm(CarInfo *pCar);

void Sys_Business(TaskMsg *msg)
{
	CarInfo *pCar = NULL;
	AlexRecoder *tmpAxle = NULL;
	static long lc_tick = 0;

	switch(msg->msg_id)
	{
		//前地感触发
		case Msg_GC1_Enter:
			debug(Debug_Business, ">>代替光幕触发, %ldms\r\n", Get_Sys_Tick());
			msg->msg_id = Msg_LC2_Enter;
		
		//光幕触发事件
		case Msg_LC2_Enter:	
			//记录光幕触发时间，待光幕释放时计算遮挡时间
			lc_tick = Get_Sys_Tick();
			
			//必须先处理前车轴数再清轴信息
			pCar = CarQueue_Get_Tail();
			FSM_CarFunction(pCar, msg);

			//一下不能清除轴，程序多线程运行，有可能算法线程加轴信号已发出在排队，轴也处于释放状态，
			//会将最后一个加轴信号丢失，产生误判，到时候莫名其妙丢轴，就算采集到波形仿真也很难重现
			//if(IsAlexHold() == 0)
			//{
			//	Clear_Axle_Recoder();
			//}

			//复位轮胎识别器
			TyreQueue_Init();
		
			//创建车辆
			Create_New_Car(NormalCarCmd);

			//超长车识别复位
			//LongCarReset();

			//多车报警
			More_Car_Alarm();
			break;

		//上秤端轴触发
		case Msg_PreAlex_Come:
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL) break;
			
			//最后一个车第一次来轴时，清除系统轴记录
			//这里主要只关注了进车，倒车时情况有些复杂，在倒车轴信号会特殊处理
			if(pCar->nAlexMsgCount == 0)
			{
				//尝试对前一个车进行轴数确认，轴数确认函数本身是防重复的
				Car_Confirm(pCar->pPrev);

				//复位轴记录
				Clear_Axle_Recoder();
			}
			
			//车辆处理来轴消息
			FSM_CarFunction(pCar, msg);

			break;

		//加减轴事件
		case Msg_PreAlex_Add:
		case Msg_PreAlex_Remove:
			
			//获取轴消息
			tmpAxle = (AlexRecoder *)msg->msg_ctx;
			if(tmpAxle == NULL) break;
			
			//从最后一个车开始
			pCar = CarQueue_Get_Tail();

			if(pCar == NULL) break;	//车都没有就不要处理轴信号了

			//当本车来轴数为零时表明信号是前车的
			if(pCar->nAlexMsgCount == 0)
			{
				//进轴时表明是前车信号
				if(msg->msg_id == Msg_PreAlex_Add)
				{
					//表明这个信号是上一个车漏掉的，切换至上一个车
					pCar = pCar->pPrev;
					debug(Debug_Business, "--前车加轴\r\n");

					if(pCar == NULL) return;
				}
				else
				{
					//退轴时表明是后退光幕距离远，来轴时没触发光幕，车辆还没创建
					//补充来轴消息
					pCar->nAlexMsgCount++;
					Clear_Axle_Recoder();
					//add_by_StreakMCU	2013-8-25
					//防止汽车反向行驶砸车 
					barriergate_open();
					//end_add_by_StreakMCU
				}
			}

			//滤除轴信号振荡信息，在轴综合判断里还有一次过滤
			if(pCar->nAxleNum > 0)
			{
				if(tmpAxle->AlexKgRef*10 < pCar->AxleInfo[pCar->nAxleNum-1].axle_peak)
				{
					debug(Debug_Business, "\r\n>轴振荡滤除cur:%d,ref:%d\r\n", tmpAxle->AlexKgRef, pCar->AxleInfo[pCar->nAxleNum-1].axle_peak);
						
					//清除误消息
					msg->msg_id = Msg_None;
					break;
				}
			}

			//保存轴信息
			Save_Axle_Info(tmpAxle);
			
			//车辆进行轴数处理
			FSM_CarFunction(pCar, msg);
			debug(Debug_Business, "**第%d轴,轴重:%d,轴峰值:%d,胎型:%s\r\n", pCar->nAxleNum, tmpAxle->AlexKg, tmpAxle->AlexKgRef, Get_Car_TryeType(tmpAxle->bDouble));
			debug(Debug_Business, "**轴速:%d\r\n", Get_CarSpeed());
			//如果不是最后一个车，最后一个轴已补上，可以确认轴信号了
			if(pCar != CarQueue_Get_Tail())
			{
				Car_Confirm(pCar);
			}

			break;
		
		//前地感释放
		case Msg_GC1_Leave:
			debug(Debug_Business, ">>代替光幕释放, %ldms\r\n", Get_Sys_Tick());
			msg->msg_id = Msg_LC2_Leave;	
			
		//光幕释放事件
		case Msg_LC2_Leave:
		case Msg_Cmd_Force:			
			//关道闸
			BarrierGateAction_After_LC2DownOff();
			
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL)
			{
				if(msg->msg_id == Msg_Cmd_Force) AbnormalAckInfo_To_SDK(ForceCarCmd, 0);
				break;
			}

			//保存命令号
			if(msg->msg_id == Msg_Cmd_Force)
			{
				pCar->cmd = ForceCarCmd;
				//车辆已经断尾
				if(pCar->nStatus > stCarComing)
				{
					AbnormalAckInfo_To_SDK(ForceCarCmd, pCar->CutOffState);
					break;
				}
				msg->msg_id = Msg_LC2_Leave;
			}
			
			if(Get_Sys_Tick() - lc_tick < 100) 
			{
				CarQueue_Car_Kill(pCar);
				debug(Debug_Business, "光幕遮挡时间<100ms, 干扰!\r\n");
				break;
			}
			FSM_CarFunction(pCar, msg);
			break;
			
		case Msg_At_Turn:
		case Msg_BackAlex_Come:	
		case Msg_GC2_Enter:
		case Msg_GC2_Leave:
			if(Get_FollowCarMode() == DevideAtOnce)
			{
				//该模式是忽略后地感与下秤轴识别器消息
				break;
			}
			else
			{
				//认为可能存在离秤缴费，后地感离收费亭远，之间至少能停下一个车
				pCar = CarQueue_Get_OnScaler_FirstCar();
				if(pCar == NULL) break;
				FSM_CarFunction(pCar, msg);
			}
			break;

		case Msg_BackAlex_Add:
			ResetDownAlexTick();
			//继续下一个case
		case Msg_BackAlex_Remove:

			if(Get_FollowCarMode() == DevideAtOnce) pCar = NULL;
			else pCar = CarQueue_Get_OnScaler_FirstCar();

			tmpAxle = (AlexRecoder *)msg->msg_ctx;

			//防止进轴误判为退轴
			if((msg->msg_id == Msg_BackAlex_Remove) && (nDownAlexTick < DOWN_ALEX_TIME_GATE))
			{
				msg->msg_id = Msg_BackAlex_Add;
				debug(Debug_Business, "不可能立即反向，强制修正为下轴进轴\r\n");
			}

			//滤除轴信号振荡信息
			if((pCar != NULL) && (tmpAxle!= NULL))
			{				
				//之前已经保存过轴信息了
				if(tmpAxle->AlexKgRef*10 < pCar->nDownAlexMaxKg)
				{
					debug(Debug_Business, "\r\n>下秤轴振荡滤除cur:%d,ref:%d\r\n", tmpAxle->AlexKgRef, pCar->nDownAlexMaxKg);
						
					//清除误消息
					msg->msg_id = Msg_None;
					break;
				}
			}

			//分发消息
			if(Get_FollowCarMode() == DevideAtOnce)
			{
				//该模式是忽略后地感与下秤轴识别器消息
				break;
			}
			else
			{
				//认为可能存在离秤缴费，后地感离收费亭远，之间至少能停下一个车
				pCar = CarQueue_Get_OnScaler_FirstCar();
				if(pCar == NULL) break;
				FSM_CarFunction(pCar, msg);
			}
			break;
		
		case Msg_Cmd_PlusSave:
			pCar = CarQueue_Get_Tail();
			if(pCar == NULL)
			{
				AbnormalAckInfo_To_SDK(LongCarCmd, 0);
				break;
			}

			FSM_CarFunction(pCar, msg);	
			break;

		case Msg_Cmd_PayNotify:
			pCar = CarQueue_Get_Head();
			if(pCar != NULL)
			{
				if(Get_FollowCarMode() == DevideAtOnce)
				{
					//CarQueue_Car_Kill(pCar);
					FSM_CarFunction(pCar, msg);
				}
				
				//冗余处理,防止队列异常
				else if(pCar->nStatus == stCarComing) 
				{
					if(pCar->pNext != NULL)
					{
						FSM_CarFunction(pCar->pNext, msg);
						CarQueue_Car_Kill(pCar);
					}	
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
			//超时计时处理
			TickTurn();

			//缴费超时处理
			TickPayTimeOut();

			//后轴识别器计时
			TickDownAlex();

			//调用超长车识别处理
			if(Get_LC_Status(2) == TRUE)
			{
				LongCarRecognize(Get_Static_Weight());
			}
			
			SysTimer_Business();
			Car_Business_Timer(msg);
			break;

		case Msg_WBAxle:
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
			if(B_Handle >= 0) SoftDog_Feed(B_Handle);
			
			//空秤定时器清零
			if(event_msg.msg_id != Msg_Tick)
			{
				Clear_ScalerEmpty_Count();
			}
			
			//打印事件消息
			MsgInfo_Print(&event_msg);

			//事务处理
		 	Sys_Business(&event_msg);

			//某些事件不打印车辆队列信息
			if((event_msg.msg_id == Msg_Tick)
				|| (event_msg.msg_id == Msg_None) 
				|| (event_msg.msg_id == Msg_GC1_Enter) 
				|| (event_msg.msg_id == Msg_GC1_Leave)				
				|| (event_msg.msg_id == Msg_PreAlex_Come)
				|| (event_msg.msg_id == Msg_PreAlex_Add)
				|| (event_msg.msg_id == Msg_PreAlex_Remove)
				|| (event_msg.msg_id == Msg_BackAlex_Come)
				|| (event_msg.msg_id == Msg_BackAlex_Add)
				|| (event_msg.msg_id == Msg_BackAlex_Remove)) 
			{
				continue;
			}
			
			//打印车辆队列信息
			CarQueue_Print_Info();
		}
	}
}

