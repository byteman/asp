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
#include "sys_config.h"

//消息说明
static const char* strMsgArr[] = 
{
	"无效",				//msgNone = 0,
	"复位",			//msgReset,
	"刷新",				//msgRefresh
	"地感1触发",		//msgInCoilEnter, //输入地感触发
	"地感1释放",		//msgInCoilLeave,
	"地感2触发",		
	"地感2释放",		
	"光幕1触发",			
	"光幕1释放",		
	"光幕2触发",		
	"光幕2释放",	
	"来轴",
	"进轴",					//msgInAlexAdd,
	"退轴",					//msgInAlexRemove,
	"后轴来轴",					//msgOutAlexEnter,
	"后轴进轴",					//msgOutAlexEnter,
	"后轴退轴",					//msgOutAlexEnter,
	"前车离开",
	"重新采集",
	"长车分段",
	"缴费通知",			//msgCmdDelCar,
	"强制采集",
	"节拍",			//msgTick,
	"弯板信号"
};

//车辆状态说明
static const char* strCarStateArr[] = 
{
	"无效",				//stCarNone
	"到来",				//stCarComing
	"进入",				//stCarEnter
	"秤中",				//stCarInScaler
	"缴费",				//stCarWaitPay
	"长车",				//stLongCarInScaler, //长车在称台上，头部已经触发后地感
	"离开",				//stCarLeaving
	"离秤缴费",			//stCarFarPay
	"删除"				//stDead,
};

static const char *strCarLevel[] = 
{
	"无效",
	"跟车",
	"长车自动",
	"单车跟随",
	"单车下秤",
	"长车分段",
	"单车"
};

static const char *strTryeType[] = 
{
	"单轮",
	"双轮",
	"无效"
};

__align(8) static char strInfo[128];
static char* pStrInfo = (char*)strInfo;

void CarQueue_Print_Info(void)
{
	CarInfo* pIter = NULL;
	int nStrIndex = 0;
	int i = 0;
	
	//获取车辆数信息
	nStrIndex += snprintf(pStrInfo + nStrIndex, 127-nStrIndex, ">>队列信息\r\nT%ld,P%ld,W%ld\r\n", CarQueue_Get_Count(), CarQueue_Get_CanPayCount(), Get_Static_Weight());
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

void CarComfirm_Print(void)
{
	debug(Debug_Business, "\r\n>>车辆确认, %ldms\r\n",  Get_Sys_Tick());
}

void MsgInfo_Print(TaskMsg *pMsg)
{
	uint8 id = 0;

	if(pMsg == NULL) return;
		
 	if(pMsg->msg_id == Msg_Tick) return;
	if(pMsg->msg_id == Msg_Refresh) return;
	
	if(Get_FollowCarMode() == DevideAtOnce)
	{
		if((pMsg->msg_id == Msg_BackAlex_Come)	||
			(pMsg->msg_id == Msg_BackAlex_Add) ||
			(pMsg->msg_id == Msg_BackAlex_Remove) ||
			(pMsg->msg_id == Msg_At_Turn))
		{
			return;
		}
	}
	

	id = pMsg->msg_id - Msg_Business_Base;
	if(id >= sizeof(strMsgArr)/sizeof(strMsgArr[0]))		 //防止数组越界
		return;
		
	//事件信息
	debug(Debug_Business, "\r\n>>%s, %ldms\r\n", strMsgArr[id], Get_Sys_Tick());
	RDelay(10);
}

const char *Get_Car_StatusName(int32 id)
{
	if(id >= sizeof(strCarStateArr)/sizeof(strCarStateArr[0]))	
		id = 0;
		
	return strCarStateArr[id];
}

const char *Get_Car_LevelName(uint8 id)
{
	if(id > 0)
		id = id - LevelMultiCar + 1;

	if(id >= sizeof(strCarLevel)/sizeof(strCarLevel[0]))	
		id = 0;	

	return strCarLevel[id];
}

const char *Get_Car_TryeType(uint8 nCount)
{
	if(nCount > 2) nCount = 2;

	return strTryeType[nCount];
}

//=========================================================================
//串口调试命令

//模拟一个车
static int Do_Simulate_Car(char *arg, DebugSend send)
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
static int Do_Simulate_BackCar(char *arg, DebugSend send)
{
	Send_Simulate_CarInfo(2, 1000, BackCarCmd);

	return 0;
}

//手动置零
static int Do_Scaler_Set_Zero(char *arg, DebugSend send)
{
	Scaler_Set_Zero(FALSE);
	return 0;
}

//打印外设信息
static int Do_Print_DeviceInfo(char *arg, DebugSend send)
{
	char *prompt = "设备状态:\r\n";
	int ADChipErr = 0;
	char buf[20] = {0};

	if(send == NULL) return 1;

	send(prompt, strlen(prompt));

	if(Get_LC_ErrStatus(2))   //判断光幕状态,双秤台只是用后光幕
	{
		snprintf(buf,19, "%s", "1.光幕正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "1.光幕故障\r\n");
	}
	send(buf,strlen(buf));
	
	if(Get_GC_ErrStatus(1))//判断地感状态，双秤台只是用前地感
	{
		snprintf(buf,19, "%s", "2.前地感正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "2.前地感故障\r\n");
	}
	send(buf,strlen(buf));

	if(Get_GC_ErrStatus(2))//判断地感状态，双秤台只是用前地感
	{
		snprintf(buf,19, "%s", "3.后地感正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "3.后地感故障\r\n");
	}
	send(buf,strlen(buf));

	if(Get_AD_ErrStatus(1))
	{
		snprintf(buf,19, "%s", "4.传感器1异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "4.传感器1正常\r\n");
	}	
	send(buf,strlen(buf));

	if(Get_AD_ErrStatus(2))
	{
		snprintf(buf,19, "%s", "5.传感器2异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "5.传感器2正常\r\n");
	}	
	send(buf,strlen(buf));

	if(Get_AD_ErrStatus(3))
	{
		snprintf(buf,19, "%s", "6.传感器3异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "6.传感器3正常\r\n");
	}	
	send(buf,strlen(buf));

	if(Get_AD_ErrStatus(4))
	{
		snprintf(buf,19, "%s", "7.传感器4异常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "7.传感器4正常\r\n");
	}
	send(buf,strlen(buf));

	if(Get_Tire_ErrStatus())//判断轮胎识别器状态
	{
		snprintf(buf,19, "%s", "8.轮胎Sensor正常\r\n");
	}
	else 
	{
		snprintf(buf,19, "%s", "8.轮胎Sensor异常\r\n");
	}	
	send(buf,strlen(buf));

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
	send(buf,strlen(buf));

	return 0;
}

static int Do_Get_Version(char *arg, DebugSend send)
{
	static char buf[64] = {0};

	if(send == NULL) return 1;
	
	snprintf(buf, 63, "程序版本:DT3101-V%d.%d.%d\r\n", AppVersion1, AppVersion2, AppVersion3);
	send( buf, strlen(buf));

	send("\r\n3102称重参数:\r\n", 15); 
	snprintf(buf, 63, "PowerZero:%d,ZeroTruckRang:%d,ZeroTick:%d\r\n", 
		gWetPar.PowerZero, gWetPar.ZeroTruckRang, gWetPar.ZeroTruckTick);
	send(buf, strlen(buf));

	snprintf(buf, 63, "Scaler_K:%f,AD3_K:%f,AD4_K:%f\r\n", 
		gWetPar.Scaler_K,gWetPar.PreAxle1_K,gWetPar.PreAxle2_K);
	send(buf, strlen(buf));

	snprintf(buf, 63, "Scaler_zero:%d,AD3_zero:%d,AD4_zero:%d\r\n", 
		(int)gWetPar.ScalerZeroAD, (int)gWetPar.PreAxle1ZeroAD, (int)gWetPar.PreAxle2ZeroAD);
	send(buf, strlen(buf));

	snprintf(buf, 63, "前轴上阈值:%d,前轴下阈值:%d\r\n", 
		gWetPar.PreAxleUpWet, gWetPar.PreAxleDownWet);
	send(buf, strlen(buf));

	snprintf(buf, 63, "后轴上阈值:%d,后轴下阈值:%d\r\n",
		gWetPar.BackAxleUpWet, gWetPar.BackAxleDownWet);
	send(buf, strlen(buf));
	
	return 0;
}

static int Do_Set_FollowMode(char *arg, DebugSend send)
{
	uint8 level = 0;
	char tmp;
	char buf[25] = {0};
	
	if(arg == NULL) return -1;
	if(send == NULL) return -1;

	tmp = Get_FollowCarMode();
	level = *arg - 0x30;

	snprintf(buf, 24, "OrigMod:%d,RevMod:%d\r\n", tmp, level);
	send(buf, strlen(buf));
	
	if(level > DevideAtDown) return -1;
	Set_FollowCarMode(level);

	return 0;
}

static int Do_Set_ScalerK(char *arg, DebugSend send)
{
	float k = 0.0f;

	k = atof(arg);

	return Set_ScalerStatic_K(k);
}

static int Do_Set_AD3K(char *arg, DebugSend send)
{
	float k = 0.0f;

	k = atof(arg);

	return Set_3Axle_Static_K(k);
}


//注册串口调试命令
void SCSZC_Debug_Cmd_Init(void)
{
	Register_Debug_Cmd("car", Do_Simulate_Car, "模拟一个车, e.g.car23000 2轴3T", TRUE);
	Register_Debug_Cmd("backcar", Do_Simulate_BackCar, "模拟一个倒车", FALSE);
	Register_Debug_Cmd("setzero", Do_Scaler_Set_Zero, "秤台手动置零", FALSE);
	Register_Debug_Cmd("print", Do_Print_DeviceInfo, "打印秤台及外设信息", FALSE);
	Register_Debug_Cmd("version", Do_Get_Version, "获取程序版本和称重参数", FALSE);
	Register_Debug_Cmd("setfmode", Do_Set_FollowMode, "设置跟车选项: 0上秤分车1下秤分车", TRUE);
	Register_Debug_Cmd("setscalerk", Do_Set_ScalerK,	"设置整秤K系数", TRUE);
	Register_Debug_Cmd("setad3k", Do_Set_AD3K,	"设置AD3_K系数", TRUE);
}

