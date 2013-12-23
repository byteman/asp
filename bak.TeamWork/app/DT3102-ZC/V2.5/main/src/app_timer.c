#include "app_timer.h"
#include "app_msg.h"
#include "device_info.h"
#include "dt3102_io.h"
#include "barriergate_ctl.h"

typedef struct {
	u8 countTick1:3;	//连续触发次数计数
	u8 countTick2:3;	//连续释放次数计数
	u8 jump:1;	//保存光幕上一次的状态 0-释放，1-触发
}SavedStatus;


//2ms timer task routine
void TaskTimer_2ms(void)
{
	static int LC2_Err_Count = 0;
	static SavedStatus LC2_status={0};
	static SavedStatus GC2_status={0};
	static uint8 cnt = 0;
	TaskMsg msg = {0};
	
	
	//读取上一次设备信息
	u32 oldDev=Get_Device_Status();
	DeviceStatus newDev={0};
	//外设信息保存地方
	sLight_State_TypeDef		lc2_status={0};	//后光幕
	sLand1_State_TypeDef		gc1_status={0};	//前地感
	sLand2_State_TypeDef 		gc2_status={0};	//后地感
	sAxisType1_State_TypeDef 	axle1_status={0};//胎型识别器-单双轮、故障、信号

	//读取光幕、地感、轮轴识别器外设状态
	lc2_status = fRead_Light2State();
	gc1_status = fRead_Land1State();
	gc2_status = fRead_Land2State();
	axle1_status = fRead_AxisType1State();

	newDev.bitValue.LC2_err		= lc2_status.Bit.ErrFlag;
	newDev.bitValue.LC2_status	= lc2_status.Bit.EventFlag;
	newDev.bitValue.GC1_err		= gc1_status.Bit.ErrFlag;
	newDev.bitValue.GC1_status	= gc1_status.Bit.EventFlag;
	newDev.bitValue.GC2_err		= gc2_status.Bit.ErrFlag;
	newDev.bitValue.GC2_status	= gc2_status.Bit.EventFlag;
	newDev.bitValue.tireSen_err	 = axle1_status.Bit.ErrFlag;
	newDev.bitValue.single_double= axle1_status.Bit.SingleOrBoth;
	newDev.bitValue.tireSen_Signal = axle1_status.Bit.EventFlag;
	
	//更新全局设备信息
	Updata_Device_Status(newDev);
	
	//比较两次的设备状态变化，发送对应消息：光幕2/前地1/后地感2---触发和释放
	//3-lc  5-gc1  7-gc2     ( 1010 1000=0xa8)
	/*------------------lc 2-----------------*/
	if(LC2_status.jump > newDev.bitValue.LC2_status)	//lc信号有跳变 trigger--release
	{
		if(++LC2_status.countTick1 > 5)		//超过5次(10ms)为有效信号
		{
			//控制OUT输出
			IoLo(OUT4);			
			
			//发送消息
			msg.msg_id=Msg_LC2_Leave;
			Task_QPost(&BusinessMsgQ, &msg);
			LC2_status.countTick1=0;
			LC2_status.jump=0;
		}		
	}
	else if(LC2_status.jump < newDev.bitValue.LC2_status)	//lc信号有跳变 release--trigger
	{
		if(++LC2_status.countTick2 > 5)		//超过5次(10ms)为有效信号
		{
			//控制OUT输出
			IoHi(OUT4);
			
			//发送消息
			msg.msg_id=Msg_LC2_Enter;
			Task_QPost(&BusinessMsgQ,&msg);
			LC2_status.countTick2=0;
			LC2_status.jump=1;
		}
	}
	else
	{
		LC2_status.countTick1=LC2_status.countTick2=0;
	}
	
	//光幕故障记录
	if(newDev.bitValue.LC2_err > 0)
	{
		if(LC2_Err_Count < 10000) LC2_Err_Count++;
	}
	else
	{
		LC2_Err_Count = 0;
	}
	
	/*---------------gc 1----------------*/
	if(LC2_Err_Count > 5000)//光幕保持10秒故障信号以上，使用前地感
	{
		if((oldDev & 0x20) > (newDev.value & 0x20))	//ground 1 sensor release
		{
			msg.msg_id=Msg_GC1_Leave;
			Task_QPost(&BusinessMsgQ,&msg);
		}
		else if((oldDev & 0x20) < (newDev.value & 0x20))	//ground 1 sensor trigger
		{
			msg.msg_id=Msg_GC1_Enter;
			Task_QPost(&BusinessMsgQ,&msg);
		}
	}
	
	/*---------------gc 2----------------*/
	if(GC2_status.jump > newDev.bitValue.GC2_status)	//ground 2 sensor release
	{
		if(++GC2_status.countTick1 > 4)		//超过4次(8ms)为有效信号
		{
			msg.msg_id=Msg_GC2_Leave;
			Task_QPost(&BusinessMsgQ,&msg);
			GC2_status.countTick1 = 0;
			GC2_status.jump = 0;
		}
	}
	else if(GC2_status.jump < newDev.bitValue.GC2_status)	//ground 2 sensor trigger
	{
		if(++GC2_status.countTick2 > 4)		//超过4次(8ms)为有效信号
		{
			msg.msg_id=Msg_GC2_Enter;
			Task_QPost(&BusinessMsgQ,&msg);
			GC2_status.countTick2 = 0;
			GC2_status.jump = 1;
		}
	}
	else
	{
		GC2_status.countTick1 = 0;
		GC2_status.countTick2 = 0;
	}

	//给业务线程发送10ms时钟
	if(++cnt >= 5)
	{
		cnt = 0;
		
		//2013-8-20 道闸控制
		TrafficSignal_Business();
		
		msg.msg_id=Msg_Tick;
		Task_QPost(&BusinessMsgQ, &msg);
	}
#if 0
	//调用速度计算模块
	if(Get_System_Mode()<=SCS_ZC_3)
	{
		Check_CarSpeed();
	}
#endif
}

