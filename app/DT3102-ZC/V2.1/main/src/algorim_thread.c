#include "includes.h"
#include "task_def.h"
#include "task_timer.h"
#include "adjob.h"
#include "weight_param.h"
#include "device_info.h"
#include "wet_algorim.h"
#include "app_beep.h"
#include "io.h"
#include "sys_config.h"
#include "data_store_module.h"
#include "softdog.h"
#include "axletype_business.h"
#include "car_queue.h"
#include "scszc_debug_info.h"

static HANDLE ad_handle;

static int A_Handle;

void Algorim_Thread(void *arg)
{
	uint8 err;
	long adbuf[4] = {0};
//	static uint8 nFreqDiv = 0;
	sAxisType1_State_TypeDef axle1_status={0};//胎型识别器-单双轮、故障、信号

	System_Config_Init();			//系统参数与FRAM初始化
	Weight_Param_Init();			//称重参数初始化
	init_flash();					//dataflash初始化	
	Beep_Initial();					//平台创建蜂鸣器任务
	Beep_Voice(2, 10);		  		//开机鸣叫2次 100ms	
	CarQueue_Init();
	SCSZC_Debug_Cmd_Init();
	A_Handle = Register_SoftDog(300, SOFTDOG_RUN, "算法线程");

	Weight_Init();

	if(ADJOB_init(ADJobPrio) == 0) {
		debug(Debug_None,"ERROR:cs5532 init failed\r\n");
		SysTaskSuspend(SYS_PRIO_SELF);
	}
	
	ad_handle = DeviceOpen(CHAR_AD,0,&err);			//打开AD采样任务
	if(err != HVL_NO_ERR) {
		debug(Debug_Error, "Error: open cs5532 failed!\r\n");
		while(1);
	}
	
	while(1) 
	{
		if(CharDeviceRead(ad_handle,(u8*)adbuf,16) != 16)
				continue;

		UpDate_ADChip_Status(DeviceIoctl(ad_handle, AD_CMDCheck, 0));

		if(A_Handle >= 0) SoftDog_Feed(A_Handle);
		
		Weight_Business(adbuf);

		//轮胎识别器处理
		axle1_status = fRead_AxisType1State();
		AxleType_Business(&axle1_status);
		

#if 0				 
		//在上秤过程中总重处理
		nFreqDiv++;
		if(nFreqDiv >= 16) {
			nFreqDiv = 0;
			//光幕要处于触发状态，计算动态下秤重量
			if(Get_LC_Status(2))	
				NewCarKgCal_Srv(Get_Dynamic_Weight());	
		}
#endif
	}
}
