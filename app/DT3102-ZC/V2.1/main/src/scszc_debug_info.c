#include "includes.h"
#include "app_msg.h"
#include "task_timer.h"
#include "sys_param.h"
#include "car_business.h"
#include "wet_algorim.h"
#include "weight_param.h"
#include "axle_business.h"
#include "overtime_queue.h"
#include "net_business_thread.h"
#include "scszc_debug_info.h"
#include "device_info.h"
#include "adjob.h"
#include "app.h"

//消息说明
static const char* strMsgArr[] = 
{
	"MsgNone",				//msgNone = 0,
	"MsgReset",			//msgReset,
	"MsgRefresh",				//msgRefresh
	"地感1触发",		//msgInCoilEnter, //输入地感触发
	"地感1释放",		//msgInCoilLeave,
	"地感2触发",		
	"地感2释放",		
	"光幕1触发",			
	"光幕1释放",		
	"光幕2触发",		
	"光幕2释放",	
	"来轴",	
	"加轴",			//msgInAlexAdd,
	"减轴",			//msgInAlexRemove,
	"后轴",			//msgOutAlexEnter,
	"重新采集",
	"长车分段",
	"缴费通知",			//msgCmdDelCar,
	"强制采集",
	"MsgTick",			//msgTick,
	"弯板信号"
};

//车辆状态说明
static char* strCarStateArr[] = 
{
	"NoCar",				//stCarNone
	"CarComing",			//stCarComing
	"CarInScaler",			//stCarInScaler
	"CarWaitPay",			//stCarWaitPay
	"LongCar",				//stLongCarInScaler, //长车在称台上，头部已经触发后地感
	"CarLeaving",			//stCarLeaving
	"CarFarPay",			//stCarFarPay
	"CarDead"				//stDead,		
};

static char *strCarLevel[] = {
	"levelNone",
	"LevelAxleWet",
	"LevelMultiCar",
	"LevelAutoLongCar",
	"LevelSingleCarBy",
	"LevelSingleCarDown",
	"LevelLongCarSegs",
	"LevelSingleCar"
};

__align(8) static char strInfo[128];
static char* pStrInfo = (char*)strInfo;

void CarQueue_Print_Info(void)
{
	CarInfo* pIter = NULL;
	int nStrIndex = 0;
	int i = 0;
	
	//获取车辆数信息
	nStrIndex += snprintf(pStrInfo + nStrIndex, 127-nStrIndex, "T%ld,P%ld,W%ld\r\n", CarQueue_Get_Count(), CarQueue_Get_CanPayCount(), Get_Static_Weight());
	//获取轴信息
	nStrIndex += snprintf(pStrInfo + nStrIndex, 127-nStrIndex, "A0%d,AU%d,AD%d\r\n", (int)gWetPar.PreAxle2ZeroAD, gWetPar.PreAxleUpWet, gWetPar.PreAxleDownWet);
	pStrInfo[nStrIndex] = 0;
	debug(Debug_Business, pStrInfo);
	RDelay(10);
	
	i = 1;
	pIter = CarQueue_Get_Head();
	while(pIter != NULL)
	{		
		debug(Debug_Business, "I%d,%11s,AX%2d,%5dkg,(%2d),%s\r\n", i, 
			Get_Car_StatusName(pIter->nStatus), pIter->nAxleNum, pIter->nBestKg, pIter->bWetVaild, Get_Car_LevelName(pIter->WetLevel));
		RDelay(5);
		pIter = pIter->pNext;
		i++;
	}
	debug(Debug_Business, "\r\n");
}

void MsgInfo_Print(TaskMsg *pMsg)
{
	uint8 id = 0;

	if(pMsg == NULL) return;
		
 	if(pMsg->msg_id == Msg_Tick) return;
		
	id = pMsg->msg_id - Msg_Business_Base;
	if(id >= sizeof(strMsgArr)/sizeof(strMsgArr[0]))		 //防止数组越界
		return;
		
	//事件信息
	debug(Debug_Business, "\r\n>>%s, %ldms\r\n", strMsgArr[id], Get_Sys_Tick());
	RDelay(10);

    //不打印队列信息的事件
	if((pMsg->msg_id == Msg_GC1_Enter)	|| 
		(pMsg->msg_id == Msg_GC1_Leave) || 
		(pMsg->msg_id == Msg_None) ||
		(pMsg->msg_id == Msg_BackAlex) ||
		(pMsg->msg_id == Msg_PreAlex_Come) ||
		(pMsg->msg_id == Msg_WBAxle)) 
		return;
	
	CarQueue_Print_Info();
}

char *Get_Car_StatusName(int32 id)
{
	if(id >= sizeof(strCarStateArr)/sizeof(strCarStateArr[0]))	
		id = 0;
		
	return strCarStateArr[id];
}

char *Get_Car_LevelName(uint8 id)
{
	if(id > 0)
		id = id - LevelAxleWet + 1;

	if(id >= sizeof(strCarLevel)/sizeof(strCarLevel[0]))	
		id = 0;	

	return strCarLevel[id];
}


//=========================================================================
//串口调试命令

//模拟一个车
static int Do_Simulate_Car(char *arg)
{
	char *buf = arg;
	uint32 wet;
	uint8 axle;

	if(arg == NULL) return -1;

	//长度不对
	if(strlen(buf) < 4) return -1;
	
	axle = *buf - 0x30;
	//轴数不对
	if((axle > 6) || (axle < 2)) return -1;
	
	buf++;
	wet = atoi(buf);
	//重量不正常
	if((wet < 500) || (wet > 200000)) return -1;
	
	Send_Simulate_CarInfo(axle, wet, NormalCarCmd);
	return 0;
}

//模拟倒车
static int Do_Simulate_BackCar(char *arg)
{
	Send_Simulate_CarInfo(2, 1000, BackCarCmd);

	return 0;
}

//手动置零
static int Do_Scaler_Set_Zero(char *arg)
{
	Scaler_Set_Zero();
	return 0;
}

//打印外设信息
static int Do_Print_DeviceInfo(char *arg)
{
	char *prompt = "设备状态:\r\n";
	int ADChipErr = 0;
	char buf[20] = {0};

	Usart1_WriteData(prompt, strlen(prompt));

	if(Get_LC_ErrStatus(2))   //判断光幕状态,双秤台只是用后光幕
	{
		snprintf(buf,19, "%s", "1.光幕正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "1.光幕故障\r\n");
	}
	Usart1_WriteData(buf,strlen(buf));
	
	if(Get_GC_ErrStatus(1))//判断地感状态，双秤台只是用前地感
	{
		snprintf(buf,19, "%s", "2.前地感正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "2.前地感故障\r\n");
	}
	Usart1_WriteData(buf,strlen(buf));

	if(Get_GC_ErrStatus(2))//判断地感状态，双秤台只是用前地感
	{
		snprintf(buf,19, "%s", "3.后地感正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "3.后地感故障\r\n");
	}
	Usart1_WriteData(buf,strlen(buf));

	if(Get_AD_ErrStatus(1))
	{
		snprintf(buf,19, "%s", "4.传感器1异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "4.传感器1正常\r\n");
	}	
	Usart1_WriteData(buf,strlen(buf));

	if(Get_AD_ErrStatus(2))
	{
		snprintf(buf,19, "%s", "5.传感器2异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "5.传感器2正常\r\n");
	}	
	Usart1_WriteData(buf,strlen(buf));

	if(Get_AD_ErrStatus(3))
	{
		snprintf(buf,19, "%s", "6.传感器3异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "6.传感器3正常\r\n");
	}	
	Usart1_WriteData(buf,strlen(buf));

	if(Get_AD_ErrStatus(4))
	{
		snprintf(buf,19, "%s", "7.传感器4异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "7.传感器4正常\r\n");
	}
	Usart1_WriteData(buf,strlen(buf));

	if(Get_Tire_ErrStatus())//判断轮胎识别器状态
	{
		snprintf(buf,19, "%s", "8.轮胎Sensor正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "8.轮胎Sensor异常\r\n");
	}	
	Usart1_WriteData(buf,strlen(buf));

	memset(buf, 0, 20);
	ADChipErr = Get_ADChip_ErrStatus();
	if(ADChipErr > 0)
	{
 		if(ADChipErr & ERROR_AD1) snprintf(buf, 15, "1 ");
 		if(ADChipErr & ERROR_AD2) snprintf(buf+2,15-2,"2 ");
 		if(ADChipErr & ERROR_AD3) snprintf(buf+2,15-4,"3 ");
 		if(ADChipErr & ERROR_AD4) snprintf(buf+2,15-6,"4 ");
 		snprintf(buf,19, "9.AD %s路异常\r\n", buf);
	}
	else if(ADChipErr == 0)
	{
		snprintf(buf,19, "%s", "9.AD正常\r\n");
	}	
	else
	{
		;
	}
	Usart1_WriteData(buf,strlen(buf));

	return 0;
}

static int Do_Get_Version(char *arg)
{
	char buf[20] = {0};
	
	snprintf(buf, 19, "DT3101-V%d.%d.%d\r\n", AppVersion1, AppVersion2, AppVersion3);
	Usart1_WriteData( buf, strlen(buf));

	return 0;
}

//注册串口调试命令
void SCSZC_Debug_Cmd_Init(void)
{
	Register_Debug_Cmd("car", Do_Simulate_Car, "模拟一个车, e.g.car23000 2轴3T", TRUE);
	Register_Debug_Cmd("backcar", Do_Simulate_BackCar, "模拟一个倒车", FALSE);
	Register_Debug_Cmd("setzero", Do_Scaler_Set_Zero, "秤台手动置零", FALSE);
	Register_Debug_Cmd("print", Do_Print_DeviceInfo, "打印秤台及外设信息", FALSE);
	Register_Debug_Cmd("version", Do_Get_Version, "获取程序版本", FALSE);
}

