#include "includes.h"
#include "car_queue.h"
#include "app_msg.h"
#include "car_business.h"
#include "overtime_queue.h"
#include "axle_business.h"
#include "sys_config.h"
#include "weight_param.h"
#include "wet_algorim.h"
#include "app_rtc.h"
#include "guiwindows.h"
#include "task_timer.h"
#include "speed_check.h"
#include "device_info.h"
#include "task_timer.h"
#include "scszc_debug_info.h"
#include "Form_Main.h"
#include "debug_info.h"

extern void ResetTurnTick(void);
extern void ResetPayTimeOutTick(void);
extern void SetPayTimeOutThreshold(int tick);
extern void ClosePayTimeOut(void);
extern void FSM_CarFunction(CarInfo *pCar, TaskMsg *pMsg);

int nCarWetMaxTick = 2000;
static OvertimeData t_carinfo = {{0}};

//调整车辆排队缴费超时时间
static void Set_Car_PayOutTick(CarInfo *pCar)
{
	if(pCar == NULL) return;

	//设置默认超时时间10秒
	SetPayTimeOutThreshold(10000);

	//根据轴数和重量调整
	if(pCar->nAxleNum <= 2)
	{
		if(pCar->nBestKg < 8000) SetPayTimeOutThreshold(8000);
		if(pCar->nBestKg < 3000) SetPayTimeOutThreshold(3000);
	}
}

//判断是否单车
BOOL IsSingleCarComming(CarInfo *pCar)
{
	if(pCar == NULL) return TRUE;

	if(pCar->nScalerKgComing < SCALER_MIN_KG) return TRUE;
	if(pCar->nScalerKgDyncComing < SCALER_MIN_KG) return TRUE;

	return FALSE;
}

//设置车辆最佳重量
void CarSetBestKg(CarInfo *pCar, int nKg, unsigned char nLevel)
{
	if(pCar == NULL) return;

	//防止下秤分车出错
	if(pCar->WetLevel == LevelMultiCar)
	{
		if(nKg > ((float)pCar->nBestKg * 1.25f))
		{
			debug(Debug_Business, "单车整秤超重:%d, %d\r\n", nKg, pCar->nBestKg);
			return;
		}
	}

	//只有当新置信度等级高于老等级时才更新重量
	if(nLevel > pCar->WetLevel)
	{
		pCar->nBestKg = nKg;
		pCar->WetLevel = nLevel;

		if(pCar->nBestKg < 850) pCar->nBestKg = 850;
	}
}

//当前车离开后车还未断尾时,判断4轴以上大车是否满足单车条件,不用考虑轴重
static BOOL Charge_FollowToSingle_Business(CarInfo *pCar, CarKgLevel level)
{
	if(pCar == NULL) return FALSE;

	if(pCar->WetLevel == LevelMultiCar)
	{
		if((pCar->nAxleNum >= 4) && (pCar->bFollowToSingle == 1))
		{
			//4轴以上的车, 如果车辆在前车离开时车辆未断尾且
			//前车离开后再上秤的轴数大于等于2
			//则认为后车满足单车条件,取整秤重量
			if((pCar->nAxleNum - pCar->nFollowAxleNum) >= 2)
			{
				pCar->nBestKg = Get_Static_MaxWeight();
				if(pCar->nBestKg < 850) pCar->nBestKg = 850;

				pCar->WetLevel = level;
				debug(Debug_Business, "跟车切换为单车\r\n");
				return TRUE;
			}
		}
	}

	return FALSE;
}

//根据轴数判断车辆超载
static void Charge_Car_OverWeight(CarInfo *pCar, OvertimeData *tCar)
{
	if(pCar == NULL) return;
	if(tCar == NULL) return;
	
	switch(pCar->nAxleNum) 
	{
		case 2:
			if(pCar->nBestKg > (int)gWetPar.Max2AxleWet)
				tCar->OverWetFlag = 1;
			break;

		case 3:
			if(pCar->nBestKg > (int)gWetPar.Max3AxleWet)
				tCar->OverWetFlag = 1;
			break;

		case 4:
			if(pCar->nBestKg > (int)gWetPar.Max4AxleWet)
				tCar->OverWetFlag = 1;
			break;

		case 5:
			if(pCar->nBestKg > (int)gWetPar.Max5AxleWet)
				tCar->OverWetFlag = 1;
			break;

		case 6:
			if(pCar->nBestKg > (int)gWetPar.Max6AxleWet)
				tCar->OverWetFlag = 1;
			break;

		default: 
			break;
	}
}

static int32 WetDrvProc(int32 Wet, u8 Drv)
{
	long nStep;

	if(Drv == 0) Drv = 1;
	
	//分度值处理
	if(Wet >= 0)
	{
		nStep = (long)((2 * Wet + Drv) / (2 * Drv));
	}
	else
	{
		nStep = (long)((2 * Wet - Drv) / (2 * Drv));
	}
	
	return (int32)(Drv * nStep);
}

#define CARSPEED_POINT_COUNT 7

//超速重量修正函数
static void Weight_Dynamic_Modify(CarInfo *pCar)
{	
	static const int CarSPoint[CARSPEED_POINT_COUNT] = {0, 50, 100, 150, 200, 250, 300};
	static float CarK[CARSPEED_POINT_COUNT];
	float wet = 0.0f, k = 0.0f;
	int i = 0;
	
	if(pCar == NULL) return;

	pCar->speed = Get_CarSpeed();

	CarK[0] = (float)gWetPar.Speed_0k  / 1000.0f;
	CarK[1] = (float)gWetPar.Speed_5k  / 1000.0f;
	CarK[2] = (float)gWetPar.Speed_10k  / 1000.0f;
	CarK[3] = (float)gWetPar.Speed_15k  / 1000.0f;
	CarK[4] = (float)gWetPar.Speed_20k  / 1000.0f;
	CarK[5] = (float)gWetPar.Speed_25k  / 1000.0f;
	CarK[6] = (float)gWetPar.Speed_30k  / 1000.0f;

	do 
	{
		if(pCar->speed <= CarSPoint[0])
		{
			wet = (float)pCar->nBestKg * CarK[0];
			break;
		}

		if(pCar->speed > CarSPoint[6])
		{
			wet = (float)pCar->nBestKg * CarK[6];
			break;
		}
		
		for(i = 1; i < CARSPEED_POINT_COUNT; i++)
		{
			if(pCar->speed < CarSPoint[i])
			{
				k = CarK[i-1] + (CarK[i]-CarK[i-1])*(pCar->speed-CarSPoint[i-1])/(float)(CarSPoint[i]-CarSPoint[i-1]);
				wet = k * (float)pCar->nBestKg;
				break;
			}
		}
	}while(0);

	//速度修正系数异常保护
	if((wet < 10.0f) || ((int)wet > (5*pCar->nBestKg))) return;

	pCar->nBestKg = (int)wet;
}  

//修正轴重异常的情况,取平均轴重
static void Modify_Abnormal_AxleWeight(CarInfo *pCar)
{
	int i = 0;
	float tmp = 0.0f;

	if(pCar == NULL) return;
	if(pCar->nAxleNum < 1) return;

	tmp = pCar->nBestKg / pCar->nAxleNum;

	//分度值处理
	tmp = WetDrvProc(tmp, gWetPar.StaticDivValue);
	if(tmp > gWetPar.StaticDivValue) tmp -= gWetPar.StaticDivValue;
		
	for(i = 0; i < pCar->nAxleNum - 1; i++) 
	{
		pCar->AxleInfo[i].axle_wet = tmp;
	}

	pCar->AxleInfo[pCar->nAxleNum-1].axle_wet = pCar->nBestKg - i*tmp;
}

//车辆重量,轴数异常修正
static void Modify_Abnormal_CarInfo(CarInfo *pCar)
{
	static uint8 cnt = 1;

	if(pCar == NULL) return;
	if((pCar->bAckErr == 1) 		||
		(pCar->cmd == LongCarCmd)	|| 
		(pCar->cmd == BackCarCmd)) return;
	
	cnt++;
	if(cnt > 6) cnt = 1;

	if(pCar->nBestKg < 850)
	{
		//车辆重量小于850kg, 取随机重量1050~1800
		pCar->nBestKg = 900 + cnt * 150;
	}
	
	if(pCar->nAxleNum < 2)  pCar->nAxleNum = 2;
	if(pCar->nAxleNum > 6) pCar->nAxleNum = 6;
}

//根据比例计算轴重, 车辆信息上送时取
static void Calc_Car_AxleWet(CarInfo *pCar)
{
	int32 sum = 0;
	float div = 0.0f;
	uint8 i = 0;

	if(pCar == NULL) return;
	if(pCar->nAxleNum < 1) return;

	sum = 0;
	for(i = 0; i < pCar->nAxleNum; i++)
	{
		sum += pCar->AxleInfo[i].axle_wet;
	}
	debug(Debug_Business, "原始轴重:%d, tick=%ld\r\n", sum, Get_Sys_Tick());
	
	if(sum <= 0) 
	{
		//计算平均轴重
		Modify_Abnormal_AxleWeight(pCar);
		return;
	}
		
	div = (float)pCar->nBestKg / (float)sum;
	sum = 0;
	for(i = 0; i < pCar->nAxleNum - 1; i++) 
	{
		pCar->AxleInfo[i].axle_wet = div * pCar->AxleInfo[i].axle_wet;

		//分度值处理
		pCar->AxleInfo[i].axle_wet = WetDrvProc(pCar->AxleInfo[i].axle_wet, gWetPar.StaticDivValue);
		//分度值补偿
		if(pCar->AxleInfo[i].axle_wet > gWetPar.StaticDivValue) 
			pCar->AxleInfo[i].axle_wet -= gWetPar.StaticDivValue;
			
		sum += pCar->AxleInfo[i].axle_wet;
	}
	
	if(sum > pCar->nBestKg) 
	{
		Modify_Abnormal_AxleWeight(pCar);
		return;
	}
	
	pCar->AxleInfo[pCar->nAxleNum-1].axle_wet = pCar->nBestKg - sum;
}

//待发送车辆信息入队(本身防车辆重入)
static void Send_CarInfo(CarInfo *pCar)
{
	uint8 i = 0;
	uint8 TireOK = 0;
	TaskMsg msg = {0};
	GuiMsgInfo gui_msg = {0};
	char datebuf[30] = {0};
	int32 orig_wet = pCar->nBestKg;
	CSysTime *time = Get_System_Time();

	if(pCar == NULL) return;
	if(pCar->bResponse == 1) return;

	pCar->speed = Get_CarSpeed();
	debug(Debug_Business, "车速: %d.%dkm/h\r\n", pCar->speed/10, pCar->speed%10);

	memset(&t_carinfo, 0, sizeof(OvertimeData));

	//异常车辆信息纠正
	Modify_Abnormal_CarInfo(pCar);

	//判断是否超限
	if(orig_wet > (int)gWetPar.MaxWet)	t_carinfo.ScalerOverWetFlag = 1;
	
	t_carinfo.cmd = pCar->cmd;

	//判断是否超载
	Charge_Car_OverWeight(pCar, &t_carinfo);

	debug(Debug_Business, "\r\n上送车辆信息, %ldms\r\n", Get_Sys_Tick());
	if(pCar->cmd != BackCarCmd) 
	{
		t_carinfo.year = pCar->year = time->year;
		t_carinfo.mon  = pCar->mon  = time->mon;
		t_carinfo.day  = pCar->day  = time->day;
		t_carinfo.hour = pCar->hour = time->hour;
		t_carinfo.min	 = pCar->min  = time->min;
		t_carinfo.sec  = pCar->sec  = time->sec;

		snprintf(datebuf, 29, "*%d-%02d-%02d %02d:%02d:%02d", pCar->year, pCar->mon, pCar->day, 
			pCar->hour, pCar->min, pCar->sec);
		debug(Debug_Business, "%s\r\n", datebuf);

		//车辆重量分度值处理
		pCar->nBestKg = WetDrvProc(pCar->nBestKg, gWetPar.StaticDivValue);

		//业务线程消息号
		msg.msg_id = Msg_CarInfo_Ready;
		//GUI线程消息号
		gui_msg.ID = WM_CARIN;
		
		//超长车分段
		if(pCar->cmd == LongCarCmd) 
		{
			t_carinfo.LongCarSegs  = pCar->nLongCarSegs;
		}
		//车辆断尾状态
		t_carinfo.CarStatus = pCar->CutOffState;	

		//sdk主动请求异常数据
		if((pCar->bAckErr == 1) || (pCar->nAxleNum <= 0)) goto send_end;
		
		//计算轴重
		Calc_Car_AxleWet(pCar);

		t_carinfo.TotalWet = pCar->nBestKg;			//总重
		t_carinfo.AxleNum  = pCar->nAxleNum;		//轴数
		t_carinfo.AxleGroupNum = pCar->nAxleNum;	//轴组数

		debug(Debug_Business, "上传重量{wet=%d,axle=%d,speed=%d,%s,%s}\r\n", 	    	
				pCar->nBestKg, pCar->nAxleNum, pCar->speed, 
				Get_Car_LevelName(pCar->WetLevel), Get_Car_StatusName(pCar->nStatus));

		//=====================轴重=====================
		for(i = 0; i < t_carinfo.AxleNum; i++) 
		{
			t_carinfo.AxleWet[i] = pCar->AxleInfo[i].axle_wet;
		}

		//=====================轴型=====================
		//初步统计轮轴识别器工作是否正常
		//只有在所有轴通过时，轮轴识别器能输出有效信号，才认为其工作是正常的
		TireOK = 1;
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			if((pCar->AxleInfo[i].axle_type != 0) && (pCar->AxleInfo[i].axle_type != 1))
			{
				TireOK = 0;
				break;
			}
		}

		//第一胎必然是单胎
		t_carinfo.AxleType[0] = 1;
		
		//轮胎识别器存在未识别情况，认为其信号不可靠
		//以下每种车型分别处理
		//处理两轴车********************************************************
		if(pCar->nAxleNum <= 2)
		{
			if(t_carinfo.TotalWet > 5000)
			{
				t_carinfo.AxleType[1] = 2;
			}
			else
			{
				t_carinfo.AxleType[1] = 1; 
			}
		}

		//处理3轴车*********************************************************
		else if(pCar->nAxleNum == 3)
		{
			//第二轴根据轴距来识别
			if(pCar->AxleInfo[0].axle_len < pCar->AxleInfo[1].axle_len)
			{
				t_carinfo.AxleType[1] = 1;
			}
			else
			{
				t_carinfo.AxleType[1] = 2; 
			}

			//第三轴根据重量判断
			if(t_carinfo.TotalWet < 4500)
			{
				//第三轴默认为双轮
				t_carinfo.AxleType[2] = 1; 
			}
			else
			{
				//第三轴默认为双轮
				t_carinfo.AxleType[2] = 2; 
			}
		}

		//处理四轴车*********************************************************
		else if(pCar->nAxleNum == 4)
		{
			//默认为1122
			t_carinfo.AxleType[1] = 1; 
			t_carinfo.AxleType[2] = 2; 
			t_carinfo.AxleType[3] = 2; 
		}

		//处理5轴车*********************************************************
		else if(pCar->nAxleNum == 5)
		{
			//第二轴根据轴距来识别
			if(pCar->AxleInfo[0].axle_len < pCar->AxleInfo[1].axle_len)
			{
				t_carinfo.AxleType[1] = 1;
			}
			else
			{
				t_carinfo.AxleType[1] = 2; 
			}

			//第三轴默认为双轮
			t_carinfo.AxleType[2] = 2; 
			t_carinfo.AxleType[3] = 2; 
			t_carinfo.AxleType[4] = 2; 
		}

		//处理6轴车*********************************************************
		else if(pCar->nAxleNum >= 6)
		{
			//第二轴根据轴距来识别
			if(pCar->AxleInfo[0].axle_len < pCar->AxleInfo[1].axle_len)
			{
				t_carinfo.AxleType[1] = 1;
			}
			else
			{
				t_carinfo.AxleType[1] = 2; 
			}

			//第三轴默认为双轮
			for(i = 2; i < pCar->nAxleNum; i++) t_carinfo.AxleType[i] = 2;
		}



		//最后补充硬识别
		if(Get_Tire_ErrStatus() && (TireOK == 1))
		{
			//第二轴很随机，按硬轴识别
			//完全正确情况下使用第二轴硬识别参数
			t_carinfo.AxleType[1] = pCar->AxleInfo[1].axle_type + 1;

			//如果是4轴车，补充第三轴识别
			if(pCar->nAxleNum == 4)
			{
				t_carinfo.AxleType[2] = pCar->AxleInfo[2].axle_type + 1;
			}
		}

		//===============================================
		//轴组类型与轴型一致
		for(i = 0; i < t_carinfo.AxleNum; i++)
		{
			t_carinfo.AxleGroupType[i] = t_carinfo.AxleType[i];
		}

		//=====================轴间距=====================
		for(i = 0;i < t_carinfo.AxleNum - 1;i++) 	
		{
			if((pCar->AxleInfo[i].axle_len / 5) > 30000)
			{
				t_carinfo.AxleLen[i] = 30000;
			}
			else
			{
				t_carinfo.AxleLen[i] = pCar->AxleInfo[i].axle_len/5;
			}
			
			if(t_carinfo.AxleLen[i] < 100)
			{
				t_carinfo.AxleLen[i] = 100;		//权值0.01
			}
		}
		
		//发送次数清零
		t_carinfo.SendNum = 0;
		
		//加速度取默认值
		t_carinfo.speeda = 0;
		t_carinfo.speed = pCar->speed;
		
		//打印轴重
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			debug(Debug_Business, "轴重%d: %d\r\n", i, pCar->AxleInfo[i].axle_wet);
		}
		debug(Debug_Business, "\r\n");
		
		//打印实际胎型
		debug(Debug_Business, "原胎型轴距: \r\n");
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			debug(Debug_Business, "%d, %d\r\n", pCar->AxleInfo[i].axle_type + 1, pCar->AxleInfo[i].axle_len);
		}
		debug(Debug_Business, "\r\n");

		//打印胎型
		debug(Debug_Business, "传胎型轴距: \r\n");
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			debug(Debug_Business, "%d, %d\r\n", t_carinfo.AxleType[i], t_carinfo.AxleLen[i]);
		}
		debug(Debug_Business, "\r\n");
	}
	else 
	{
		gui_msg.ID = WM_CARBACK;
		msg.msg_id = Msg_BackCarInfo;
		debug(Debug_Business, "发送倒车消息!\r\n\r\n");
	}	

send_end:
	t_carinfo.SendMod = SendActiveMod;

	//车辆信息入队待发送数据队列
#ifdef _SIMULATION_NOPAY_
	if(1)
#else
	if(OverQueue_Enqueue(&t_carinfo) == 0)
#endif
	{
		//入队成功,置车辆已发送标志(入队后由OverQueue管理,CarQueue认为该车已上送)
		pCar->bResponse = 1;

		//给业务数据管理线程发送消息
		Task_QPost(&DataManagerMsgQ,  &msg);

		//向主当前窗口发送加载消息,轴数为0的不显示
		if(t_carinfo.AxleNum > 0)
		{
			if(g_pCurWindow != &gWD_Main)
			{
				memcpy(&gMainCarInfo, &t_carinfo, sizeof(OvertimeData));
			}
			else
			{
				gui_msg.pWindow = &gWD_Main;
				gui_msg.wParam = (u32)&t_carinfo;
				GuiMsgQueuePost(&gui_msg);
			}
		}
	}
}

//跟车重量计算
static void Calc_FollowCar_Wet(CarInfo *pCar)
{
	int32 wet = 0, axlewet = 0;	
	uint8 i = 0;
	
	if(pCar == NULL) return;

	//计算轴重
	axlewet = 0;
	for(i = 0; i < pCar->nAxleNum; i++)
	{
		axlewet += pCar->AxleInfo[i].axle_wet;
	}

	//计算秤台重量差
	wet = pCar->nScalerKgIn - pCar->nScalerKgComing;	//光幕触发释放前后秤台静态重量差值

	//比较然后取较大值
	if((wet > ((float)axlewet / 1.03f)) || ((wet>3000) && ((wet+500) > axlewet)))
	{
		CarSetBestKg(pCar, wet, LevelMultiCar);
	}
	else 
	{
		CarSetBestKg(pCar, axlewet, LevelMultiCar);
	}

//	debug(Debug_Business, ">跟车计重, 轴重和=%d, 整秤差=%d, tick=%ld\r\n", axlewet, wet, Get_Sys_Tick());

	//处理六轴车，可能秤台上只能停下一个六轴车
	if((pCar->nAxleNum >= 6) && ((pCar->pPrev == NULL) || ((pCar->pPrev != NULL) && (pCar->pPrev->nAxleNum > 2)))) 
	{
		pCar->nBestKg = Get_Static_MaxWeight();
		if(pCar->nBestKg < 850) pCar->nBestKg = 850;
		pCar->WetLevel = LevelSingleCarStable;

		debug(Debug_Business, ">6轴跟车, 轴重和=%d, 总重=%d, tick=%ld\r\n", axlewet, pCar->nBestKg, Get_Sys_Tick());
	}
	else
	{
		debug(Debug_Business, ">跟车计重, 轴重和=%d, 整秤差=%d, tick=%ld\r\n", axlewet, wet, Get_Sys_Tick());
	}
}

//超长车重量计算
static void Calc_LongCar_Wet(CarInfo *pCar)
{
	int32 wet = 0, axlewet = 0;	
	uint8 i = 0;
	
	if(pCar == NULL) return;

	//计算轴重
	axlewet = 0;
	for(i = 0; i < pCar->nAxleNum; i++)
	{
		axlewet += pCar->AxleInfo[i].axle_wet;
	}

	//判断是否能够满足分段取重量的条件，没有用下秤地感时无法分段采集
	if((Get_FollowCarMode() != DevideAtOnce) && (pCar->nLongCarHalfAxle != 0) && (pCar->nBackAxleNum == pCar->nLongCarHalfAxle))
	{
		//前后两段重量相加
		wet = pCar->nLongCarHalfWet + Get_Static_Weight();
		
		//比较然后取较大值
		if((wet > ((float)axlewet / 1.12f)) && (wet < ((float)axlewet * 1.25f))) 
		{
			CarSetBestKg(pCar, wet, LevelAutoLongCar);
		} 
		else 
		{
			CarSetBestKg(pCar, axlewet, LevelAutoLongCar);
		}
		//假超长车处理
		if((TRUE == IsSingleCarComming(pCar))&&(wet > axlewet))
		{
			pCar->nBestKg = wet;
		}
		debug(Debug_Business, ">超长车计重, 轴重和=%d, 分段和=%d, tick=%ld\r\n", axlewet, wet, Get_Sys_Tick());
	}
	else
	{
		//计算秤台重量差
		wet = Get_Static_MaxWeight();	//超长车去秤台最大值视为整车最优值

		//比较然后取较大值
		if((wet > ((float)axlewet / 1.03f)) && (wet < ((float)axlewet * 1.25f))) 
		{
			CarSetBestKg(pCar, wet, LevelAutoLongCar);
		} 
		else 
		{
			CarSetBestKg(pCar, axlewet, LevelAutoLongCar);
		}
		
		//假超长车处理
		if((TRUE == IsSingleCarComming(pCar))&&(wet > axlewet))
		{
			pCar->nBestKg = wet;
		}

		if((pCar->nAxleNum - pCar->nLongCarHalfAxle) == 0)
		{			
			pCar->nBestKg = (pCar->nLongCarHalfWet > wet) ? pCar->nLongCarHalfWet : wet;
		}

		debug(Debug_Business, ">超长车计重, 轴重和=%d, 最大值=%d, tick=%ld\r\n", axlewet, wet, Get_Sys_Tick());
	}
}


static void LongCar_Segs_Business(CarInfo *pCar)
{	
	int num = 0;
	
	if(pCar == NULL) return;

	num = Apply_LongCar_Axle(pCar);
	if(num <= pCar->nLongCarSegAxle)
	{	
		//发送异常数据
		AbnormalAckInfo_To_SDK(LongCarCmd, pCar->CutOffState);
		if(pCar->CutOffState == 1)
		{
			//光幕断尾,倒车,删掉
			if(pCar->nAxleNum <= 0) CarQueue_Car_Kill(pCar);
		}
		return;
	}
	//计算轴数增加量
	pCar->nAxleNum = num - pCar->nLongCarSegAxle;
	debug(Debug_Business, "第%d分段,分段轴数%d\r\n", pCar->nLongCarSegs, pCar->nAxleNum);
	
	pCar->bResponse = 0;
	pCar->nBestKg = Get_Static_Weight();
	pCar->cmd = LongCarCmd;
	pCar->WetLevel = LevelLongCar; 
	pCar->bWetVaild = TRUE;

	//上送分段信息
	Send_CarInfo(pCar);
	//保存车辆实际的轴数, 还原轴数
	pCar->nLongCarSegAxle = num;
	pCar->nAxleNum = num;
}

//车辆断尾后延时对轴数进行判断（防车辆重复确认轴数）
int Car_Confirm(CarInfo *pCar)
{
	int32 nAxleCount = 0;	
	OvertimeData *pLastCar = NULL;

	if(pCar == NULL) return 2;

	if(pCar->bCarIsDead == TRUE) return 2;

	//防止重复确认
	if(pCar->bConfirm == TRUE) return 0;

	//判断车辆是否收到过轴信息
	if(pCar->nAlexMsgCount == 0)
	{
		//表明是干扰信号
		debug(Debug_Business, "外部干扰!\r\n");
		CarQueue_Car_Kill(pCar);
		return 2;
	}
	
	//计算车辆轴数
	nAxleCount = Apply_Car_Axle(pCar);

	//打印轴数信息
	debug(Debug_Business, "**轴数处理,总轴数=%d\r\n", nAxleCount);
	
	if(nAxleCount > 0) 
	{
		//正常上车
		pCar->nAxleNum = nAxleCount;
		pCar->bConfirm = TRUE;
		return 0;
	}
	else if(nAxleCount < 0 ) 
	{
		//倒车
		debug(Debug_Business, "倒车! \r\n");
		CarQueue_Car_Kill(pCar);		
		pCar->cmd = BackCarCmd;
	
		//删除前面的车应当大致匹配轴数
		if(pCar->pPrev != NULL)
		{
			if(nAxleCount + pCar->pPrev->nAxleNum <= 2)
			{
				pLastCar = OverQueue_Get_Tail();
				if(pLastCar != NULL)
				{
					//车辆数据还未上送，且队尾不是倒车信息删掉
					if((pLastCar->cmd != BackCarCmd) && (pLastCar->bSend == 0))
					{		
						OverQueue_Remove_Tail();
						debug(Debug_Business, "通信队列阻塞,不发送倒车信息!\r\n");
					} 
					else
					{
						pCar->cmd = BackCarCmd;
						pCar->nAxleNum = 2;
						Send_CarInfo(pCar);				//发送倒车信息		
					}
				}
				else
				{
					pCar->cmd = BackCarCmd;
					pCar->nAxleNum = 2;
					Send_CarInfo(pCar);				//发送倒车信息	
				}
				CarQueue_Car_Kill(pCar->pPrev);
			}	
		}
		else
		{
			//在广西版流程中，收费电脑一旦读取了数据，就会发生删车命令
			//所以在倒车时，前面的车已经删除了
			pCar->cmd = BackCarCmd;
			pCar->nAxleNum = 2;
			Send_CarInfo(pCar);				//发送倒车信息		
		}
		return 1;
	} 
	else	
	{		
		debug(Debug_Business, "外部干扰!\r\n");
		CarQueue_Car_Kill(pCar);		//外部干扰	
		return 2;
	}			
}

//超时取重量
void Car_Confirm_Wet(CarInfo *pCar)
{
	if(pCar == NULL) return;

	if((pCar->bWetVaild == FALSE) && (pCar->WetLevel >= LevelMultiCar))
	{
		debug(Debug_Business, "超时取重量:%d\r\n", pCar->nBestKg);

		pCar->bWetVaild = TRUE;

		Send_CarInfo(pCar);
	}
}


//==========================================以下是状态响应函数=====================================

//车辆到达状态响应外部事件处理函数
static void Car_Business_CarComState(CarInfo *pCar, TaskMsg *pMsg)
{	
	AlexRecoder *tmpAxle = NULL;
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;

		//轴到来消息
		case Msg_PreAlex_Come:
			if(pCar->nAlexMsgCount == 0)
			{
				//记录初始重量
				pCar->nScalerKgComing = Get_Static_Weight();
				pCar->nScalerKgDyncComing = Get_Dynamic_Weight();
				if((pCar->nScalerKgComing < SCALER_MIN_KG) || (pCar->nScalerKgDyncComing < SCALER_MIN_KG))
				{
					debug(Debug_Business, "单车触秤, 秤台重量=%d\r\n", Get_Dynamic_Weight());
				}

				//超长车判断复位
				LongCarReset();
			}
			pCar->nAlexMsgCount++;
			break;

		//加轴
		case Msg_PreAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= gWetPar.nMaxAxleNum))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_peak = tmpAxle->AlexKgRef;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}


				//超长车判断复位
				if((pCar->nAxleNum == 0) && (pCar->nAlexMsgCount == 1))
				{
					LongCarReset();
				}

				//记录轴数
				pCar->nAxleNum++;
			}
	
			Clear_ScalerMaxWeight();
			break;

		//减轴
		case Msg_PreAlex_Remove:
			pCar->nAxleNum--;
			Clear_ScalerMaxWeight();
			break;
	
		//光幕释放
		case Msg_LC2_Leave:
			pCar->CutOffState = 1;

			//进stCarEnter状态前清除秤台最大值记录以重新计数
			//Clear_ScalerMaxWeight();
		

			//切换到stCarEnter状态
			FSM_Change_CarState(pCar, stCarEnter);
			break;

		//触发后地感
		case Msg_GC2_Enter:
			//超长车处理
			if(Get_LongCar_Mode() == AutoLongCarMode) 
			{
				//至少要有一个轴才算是超长车吧
				if(pCar->nAlexMsgCount <= 0) break;

				if(CarQueue_Get_OnScaler_Count() == 1)
				{
					//记录当前半个车的重量和轴数
					pCar->nLongCarHalfWet = Get_Static_Weight();
					pCar->nLongCarHalfAxle = Apply_LongCar_Axle(pCar);
					
					//进超长车状态前清除秤台最大值记录以重新计数
					Clear_ScalerMaxWeight();

					//切换至超长车状态
					FSM_Change_CarState(pCar, stLongCar);
				}
			}
			break;

		//后轴信号
		case Msg_BackAlex_Come:	
			if(Get_LongCar_Mode() == AutoLongCarMode) 
			{
				//至少要有一个轴才算是超长车吧
				if(pCar->nAlexMsgCount <= 0) break;

				if(CarQueue_Get_OnScaler_Count() == 1) 
				{			
					//记录当前半个车的重量和轴数
					pCar->nLongCarHalfWet = Get_Static_Weight();
					pCar->nLongCarHalfAxle = Apply_LongCar_Axle(pCar);
					
					//切换至超长车状态
					FSM_Change_CarState(pCar, stLongCar);
				}
			}
			break;

		//分段采集
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;

		case Msg_At_Turn:
			//记录车辆跟车时的轴数
			pCar->nFollowAxleNum = pCar->nAxleNum;
			pCar->bFollowToSingle = 1;
			break;

		default:
			break;
	}
}

int GetDyncAvgValue(CarInfo *pCar)
{
	//默认1200kg的小车
	if(pCar == NULL) return 1200;

	if(pCar->bDyncMinOk == FALSE)
	{
		return pCar->fDyncMaxKg;
	}

	if((pCar->fDyncMaxKg - pCar->fDyncMinKg)*1/8 > (pCar->fDyncMaxLastKg - pCar->fDyncMinKg))
	{
		return pCar->fDyncMaxKg;
	}

	return pCar->fDyncAvgKg;
}

void Car_Dync_Kg_Update(CarInfo *pCar)
{
	if(pCar == NULL) return;

	if(pCar->nStatusKeepTime <= 850)
	{
		//获取第一区间的最大值
		if(pCar->fDyncMaxKg <= Get_Static_Weight())
		{
			pCar->fDyncMaxKg = Get_Static_Weight();
		}		
		
		pCar->fDyncMinKg = pCar->fDyncMaxKg;
		pCar->fDyncAvgKg = 0.0f;
		pCar->fDyncSumKg = 0.0f;
		pCar->fDyncMaxLastKg = 0.0f;
		pCar->nDyncKgIndex = FALSE;
		pCar->nDyncKgCount = FALSE;
		pCar->bDyncMaxOk = FALSE;
		pCar->bDyncMinOk = FALSE;
		pCar->bDyncValid = FALSE;
		pCar->bDyncRaise = FALSE;
		return;
	}

	if((pCar->fDyncMaxKg <= Get_Static_Weight()) && (pCar->bDyncMaxOk == FALSE))
	{
		if(pCar->fDyncMaxKg < Get_Static_Weight())
		{
			pCar->fDyncMaxKg = Get_Static_Weight();
			pCar->fDyncMinKg = pCar->fDyncMaxKg;
			pCar->fDyncSumKg = 0.0f;
			pCar->fDyncAvgKg = pCar->fDyncMaxKg * 0.999f;
			pCar->nDyncKgIndex = 0;
			pCar->nDyncKgCount = 0;
		}
		else
		{
			pCar->fDyncSumKg += Get_Static_Weight();
			pCar->nDyncKgIndex++;
		}
	}
	else
	{		
		if(pCar->bDyncMinOk == FALSE)
		{
			pCar->bDyncMaxOk = TRUE;

			if(pCar->fDyncMinKg >= Get_Static_Weight())
			{
				pCar->fDyncMinKg = Get_Static_Weight();
				pCar->fDyncSumKg += Get_Static_Weight();
				pCar->nDyncKgIndex++;				
			}
			else
			{
				//pCar->fDyncAvgKg = (float)(pCar->fDyncSumKg) / pCar->nDyncKgIndex;	
				pCar->fDyncAvgKg = (pCar->fDyncMaxKg + pCar->fDyncMinKg) / 2;
				pCar->fDyncMaxLastKg = pCar->fDyncMinKg;
				pCar->bDyncMinOk = TRUE;
			}	
		}
	}

	//记录出现最小值之后，后面的最大值来判断波形
	if(pCar->bDyncMinOk == TRUE)
	{
		if(pCar->fDyncMaxLastKg <= Get_Static_Weight())
		{
			pCar->fDyncMaxLastKg = Get_Static_Weight();
		}	
	}
}

//车辆进入状态
extern int IsAlexHold(void);
extern BOOL IsAtTurn(void);
static void Car_Business_CarEnterState(CarInfo *pCar, TaskMsg *pMsg)
{
	//int CarWet = 0;
	int nCarMaxWet = 0;
	//int poise = 0;
	AlexRecoder *tmpAxle = NULL;
	TaskMsg msg = {0};
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id)
	{
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;

			//在到达设定时间之前求值
			if(pCar->nStatusKeepTime <= nCarWetMaxTick)
			{
				Car_Dync_Kg_Update(pCar);
			}

			if(pCar->nStatusKeepTime >= nCarWetMaxTick)
			{
				//判断轴识别器是否已经释放
				if(IsAlexHold() > 0)
				{
					nCarWetMaxTick += 50;
					break;
				}

				//复位到默认值，两个车同时在Enter状态时可能有问题
				nCarWetMaxTick = 2000;

				//打印车辆确认消息
				CarComfirm_Print();

				//分车处理，判断是否为正常上车、干扰或倒车，结果为0表示正常上车
				if(Car_Confirm(pCar) != 0) 
				{
					//刷新队列信息
					msg.msg_id = Msg_Refresh;
					Task_QPost(&BusinessMsgQ, &msg);
					break;
				}

				if(IsSingleCarComming(pCar))
				{
					//超长车处理
					if((IsLongCar() == TRUE) && (Get_LongCar_Mode() == AutoLongCarMode))
					{
    					pCar->nScalerKgIn = Get_Static_Weight();
    					Calc_LongCar_Wet(pCar);
					}

					//单车处理
					else
					{
						//单车上秤取值
						pCar->nScalerKgIn = Get_Static_Weight();
						nCarMaxWet = Get_Static_MaxWeight();
						//单车重量限制
						if(nCarMaxWet < 400)
						{
							debug(Debug_Business, "车辆最值:%d,太小删掉!\r\n", nCarMaxWet);
							CarQueue_Car_Kill(pCar);
							break;
						}	
#if 1
						//CarSetBestKg(pCar, nCarMaxWet, LevelSingleCarStable);
						//debug(Debug_Business, "取单车最值:%d\r\n", nCarMaxWet);
		
						
						CarSetBestKg(pCar, GetDyncAvgValue(pCar), LevelSingleCarStable);
						debug(Debug_Business, "取单车平衡值:%d\r\n", (int)(GetDyncAvgValue(pCar)));
#else						
						//根据车辆重量增大砝码
						poise = 500;
						if(nCarMaxWet > 30000) poise = 1000;
						
						if(Get_Static_Weight() < nCarMaxWet - poise) 
						{
						CarSetBestKg(pCar, nCarMaxWet, LevelSingleCarStable);

						debug(Debug_Business, "取单车最值:%d\r\n", nCarMaxWet);
						} 
						else 
						{
						CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarStable);

						debug(Debug_Business, "取单车当前:%d  最值:%d\r\n", pCar->nBestKg, nCarMaxWet);
						}
#endif
						//根据速度补偿车辆重量
						Weight_Dynamic_Modify(pCar);
					}

					pCar->bWetVaild = TRUE;
				}
				else
				{
					//获取进入后的秤台重量
					pCar->nScalerKgIn = Get_Static_Weight();
					
					//防止光幕收尾后前车已经开始下秤
					nCarMaxWet = Get_Static_MaxWeight();
					if(nCarMaxWet > pCar->nScalerKgIn + 50)
					{
						pCar->nScalerKgIn = nCarMaxWet;
					}

					//计算跟车重量
    				Calc_FollowCar_Wet(pCar);

					//在下秤分车模式下判断是否有单车可能
					if(Get_FollowCarMode() != DevideAtOnce)
					{
						//判断是否有机会取单车重量
						if((pCar->pPrev == NULL) || ((pCar->pPrev != NULL) && (pCar->pPrev->nStatus == stCarFarPay)))
						{
							//单车在秤上，要求前车至少离开了一秒钟以上
							if(IsAtTurn() == TRUE)
							{	
								if(Charge_FollowToSingle_Business(pCar, LevelSingleCarBeginDown) == FALSE)
								{
									CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarBeginDown);
								}
								pCar->bWetVaild = TRUE;

								debug(Debug_Business, "取单车重量:%d\r\n", pCar->nBestKg);
							}
						}
					}
    				
					//当跟车取值采取即刻取值方式时，立即上传重量
					if(Get_FollowCarMode() == DevideAtOnce)
					{
						pCar->bWetVaild = TRUE;
					}
					else
					{
						//如果小车取值为立即上传
						if(Get_LittleFollowCarMode() == DevideAtOnce)
						{
							if(pCar->nBestKg < 3000)
							{
								if(pCar->pPrev != NULL)
								{
									//需要判断前车已经上传数据
									if(pCar->pPrev->bResponse == 1) pCar->bWetVaild = TRUE;
								}
								else
								{
									pCar->bWetVaild = TRUE;
								}
							}
						}
					}
				}

				//切换到下一个状态
				if(pCar->nNextStatus == stCarWaitPay)
				{
					//该情况可能是由于还没有到达1200ms后地感就被触发

					//在非上秤分车模式下，判断前面的车是否已经离开了
					if((pCar->bWetVaild != TRUE) && (Get_FollowCarMode() != DevideAtOnce))
					{
						if(CarQueue_Car_Is_Head(pCar))
						{
							pCar->nScalerKgIn = Get_Static_Weight();
							nCarMaxWet = Get_Static_MaxWeight();
							if(nCarMaxWet > pCar->nScalerKgIn + 50)
							{
								pCar->nScalerKgIn = nCarMaxWet;
							}
						
							if(Charge_FollowToSingle_Business(pCar, LevelSingleCarBeginDown) == FALSE)
							{
								CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarBeginDown);
							}
							debug(Debug_Business, "取单车重量:%d\r\n", nCarMaxWet);
						}
					}

					//转入待缴费模式前一定要上传重量了
					pCar->bWetVaild = TRUE;

					//切换到指定状态
					pCar->nNextStatus = stCarNone;

#ifdef _SIMULATION_NOPAY_
					FSM_Change_CarState(pCar, stCarLeaving);
#else
					FSM_Change_CarState(pCar, stCarWaitPay);
#endif
				}
				else
				{
					FSM_Change_CarState(pCar, stCarInScaler);
				}

				//打印车辆消息
				CarQueue_Print_Info();

				//上传车辆信息
				if(pCar->bWetVaild == TRUE)
				{
    				Send_CarInfo(pCar);
				}
			}
			break;

		//轴到来消息
		case Msg_PreAlex_Come:
			if(pCar->nAlexMsgCount == 0)
			{
				//记录初始重量
				pCar->nScalerKgComing = Get_Static_Weight();
				pCar->nScalerKgDyncComing = Get_Dynamic_Weight();
				if((pCar->nScalerKgComing < SCALER_MIN_KG) || (pCar->nScalerKgDyncComing < SCALER_MIN_KG))
				{
					debug(Debug_Business, "单车触秤, 秤台重量=%d\r\n", Get_Dynamic_Weight());
				}					
			}
			pCar->nAlexMsgCount++;
			break;

		//加轴
		case Msg_PreAlex_Add:
			
			//必须要有来轴消息才进行后续处理，防止是前一个车漏掉的轴信号
			if(pCar->nAlexMsgCount == 0) break;

			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= gWetPar.nMaxAxleNum))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_peak = tmpAxle->AlexKgRef;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}

				pCar->nAxleNum++;
			}
			
			Clear_ScalerMaxWeight();
			break;

		//减轴
		case Msg_PreAlex_Remove:

			//必须要有来轴消息才进行后续处理，防止是前一个车漏掉的轴信号
			if(pCar->nAlexMsgCount == 0) break;

			pCar->nAxleNum--;
			Clear_ScalerMaxWeight();
			break;

		//后车触发光幕
		case Msg_LC2_Enter:

			//确认车辆轴数
			//这里不再确认，有可能最后一个轴信号还没释放
			//系统轴记录清除时刻改到了后一个车第一次来轴时
			//Car_Confirm(pCar);
			
			//暂时不改变状态，等到1200ms后自然转变
		    break;
		
		//后轴信号
        case Msg_BackAlex_Come:
			//指示1200ms定时后转向待缴费状态
			pCar->nNextStatus = stCarWaitPay;
			break;

		//下秤地感触发
		case Msg_GC2_Enter:
			//指示1200ms定时后转向待缴费状态
			pCar->nNextStatus = stCarWaitPay;
			break;

		//后轴信号
        case Msg_BackAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			pCar->nDownAlexMaxKg = tmpAxle->AlexKgRef;

			pCar->nBackAxleNum++;
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
			{
				//如果还在进入状态下后轴就走完了，真是叫没天理
                //FSM_Change_CarState(pCar, stCarFarPay);
			}
			break;

		//下秤地感释放（异常信号）
		case Msg_GC2_Leave:
			//后地感触发信号有可能是干扰信号
			pCar->nNextStatus = stCarNone;
			break;

		//分段采集
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;

		default:
			break;
	}		
}

//车辆完全上秤状态响应外部事件处理函数
static void Car_Business_CarInScalerState(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;
  
		//下秤轴触发或后地感触发
		case Msg_BackAlex_Come:
		case Msg_GC2_Enter:
			//如果该车是最后一辆车并且重量还没确认，则可以取到单车重量
			if(pCar->bWetVaild == FALSE)
			{
				if((pCar->pNext != NULL) && (pCar->pNext->nAlexMsgCount > 0))
				{
					debug(Debug_Business, "取跟车重量:%d\r\n", pCar->nBestKg);
				}
				else
				{
					//单车在秤上
					if(Charge_FollowToSingle_Business(pCar, LevelSingleCarBeginDown) == FALSE)
					{
						CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarBeginDown);
					}
					debug(Debug_Business, "取单车下秤重量:%d\r\n", pCar->nBestKg);
				}

				//不管取哪种重量方式，都必须置为有效并上传
				pCar->bWetVaild = TRUE;
				Send_CarInfo(pCar);
			}

#ifdef _SIMULATION_NOPAY_
			FSM_Change_CarState(pCar, stCarLeaving);
#else
			FSM_Change_CarState(pCar, stCarWaitPay);
#endif
			break;

		//排队时间到
		case Msg_At_Turn:
			//如果该车是最后一辆车并且重量还没确认，则可以取到单车重量
			if(pCar->bWetVaild == FALSE)
			{
				if((pCar->pNext != NULL) && (pCar->pNext->nAlexMsgCount > 0))
				{
					debug(Debug_Business, "取跟车重量:%d\r\n", pCar->nBestKg);
				}
				else
				{
					//单车在秤上
					if(Charge_FollowToSingle_Business(pCar, LevelSingleCarBeginDown) == FALSE)
					{
						CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarBeginDown);
					}
					debug(Debug_Business, "取单车排队重量:%d\r\n", pCar->nBestKg);
				}

				//不管取哪种重量方式，都必须置为有效并上传
				pCar->bWetVaild = TRUE;
				Send_CarInfo(pCar);
			}			
			
			break;

		//后车触发光幕
		case Msg_LC2_Enter:
			if(pCar->bWetVaild == FALSE)
			{
				//如果该车是第一辆车，可以取秤台重量作为车辆重量
				if(CarQueue_Car_Is_Head(pCar) || ((pCar->pPrev != NULL) && (pCar->pPrev->nStatus == stCarFarPay)))
				{
					if(Charge_FollowToSingle_Business(pCar, LevelSingleCarBy) == FALSE)
					{
						CarSetBestKg(pCar, Get_Static_Weight(), LevelSingleCarBy);
					}
					debug(Debug_Business, "取单车未稳重量:%d!\r\n", pCar->nBestKg);
					pCar->bWetVaild = TRUE;
					Send_CarInfo(pCar);					
				}
				else
				{
					debug(Debug_Business, "取跟车重量:%d!\r\n",  pCar->nBestKg);
					pCar->bWetVaild = TRUE;
					Send_CarInfo(pCar);
				}
			}
		  break;

		//缴费通知
		case Msg_Cmd_PayNotify:
			if(Get_FollowCarMode() == DevideAtOnce)
			{
				CarQueue_Car_Kill(pCar);
			}
			else
			{
				Set_Car_PayOutTick(pCar);
				ResetPayTimeOutTick();

				FSM_Change_CarState(pCar, stCarLeaving);
			}
			break;

		//分段采集
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;

		default:
			break;
	}
}

//车辆等待收费状态响应外部事件处理函数
static void Car_Business_CarWaitPayState(CarInfo *pCar, TaskMsg *pMsg)
{
	AlexRecoder *tmpAxle = NULL;

	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;

		//缴费通知
		case Msg_Cmd_PayNotify:
			if(Get_FollowCarMode() == DevideAtOnce)
			{
				CarQueue_Car_Kill(pCar);
			}
			else
			{
				Set_Car_PayOutTick(pCar);
				ResetPayTimeOutTick();

				FSM_Change_CarState(pCar, stCarLeaving);
			}
			break;

		//后地感释放
       	case Msg_GC2_Leave:
			//if(pCar->nBackAxleNum > 1)	//有可能是前一个车离开后地感的信号
			if(pCar->nBackAxleNum > (int8)(pCar->nAxleNum/2))
			{
				ResetTurnTick();
       			FSM_Change_CarState(pCar, stCarFarPay); 
			}
			break;
		
		//下秤进轴
		case Msg_BackAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			pCar->nDownAlexMaxKg = tmpAxle->AlexKgRef;

			pCar->nBackAxleNum++;
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
			{
				ResetTurnTick();
                FSM_Change_CarState(pCar, stCarFarPay);
			}
            break;

		//下秤退轴
		case Msg_BackAlex_Remove:
			if(pCar->nBackAxleNum > 0) pCar->nBackAxleNum--;

			if(pCar->nBackAxleNum == 0)
			{
                FSM_Change_CarState(pCar, stCarInScaler);
			}
            break;   

        //分段采集
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;

		default:
			break;
	}
}

//车辆缴费离开状态响应外部事件处理函数
static void Car_Business_CarLeave(CarInfo *pCar, TaskMsg *pMsg)
{
	AlexRecoder *tmpAxle = NULL;

	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;	
			break;

		//后地感
		case Msg_GC2_Leave: 
			
			//这个时候有可能是后地感闪烁，有可能是下秤轴数数错，谁的置信度高？

#if(1)//信了后地感

			//如果后地感置信度高pCar->nBackAxleNum可能数多，提前删车了
			if(pCar->nBackAxleNum > (int8)(pCar->nAxleNum/2))
			{
				ResetTurnTick();
				ClosePayTimeOut();
				CarQueue_Car_Kill(pCar);
			}
			else
			{
				//误动作，是前一个车的轴没走完
				pCar->nBackAxleNum = 0;
			}

#else
			//如果轴识别器置信度高
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
			{
				ResetTurnTick();
				ClosePayTimeOut();
				CarQueue_Car_Kill(pCar);
			}
#endif
			break;

		//下秤进轴
		case Msg_BackAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			pCar->nDownAlexMaxKg = tmpAxle->AlexKgRef;

			pCar->nBackAxleNum++;
			ResetPayTimeOutTick();
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
			{
			   ResetTurnTick();
			   ClosePayTimeOut();
               CarQueue_Car_Kill(pCar);
			}
            break;    

		//下秤退轴
		case Msg_BackAlex_Remove:
#ifndef _SIMULATION_NOPAY_			
			//离开状态的车已缴费，退轴信号认为是进轴被误判
			pCar->nBackAxleNum++;
			ResetPayTimeOutTick();
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
			{
				ResetTurnTick();
				ClosePayTimeOut();
				CarQueue_Car_Kill(pCar);
			}
#else
			if(pCar->nBackAxleNum > 0) pCar->nBackAxleNum--;
			if(pCar->nBackAxleNum == 0)
			{
		       FSM_Change_CarState(pCar, stCarInScaler);
			}
#endif
		  break;  

		//缴费通知
        case Msg_Cmd_PayNotify:
			//后地感故障的异常响应
			ResetTurnTick();
			CarQueue_Car_Kill(pCar);
			
			//让后车来处理
			if(pCar->pNext != NULL) 
			{
				debug(Debug_Business, "后地感异常, 本车已离开, 缴费通知给后车!\r\n");

				//让后面一个车响应缴费消息，一般来说，此时后面一个车处于秤中状态
				FSM_CarFunction(pCar->pNext, pMsg);
			}
			break;

		//分段采集
		case Msg_Cmd_PlusSave:
			AbnormalAckInfo_To_SDK(LongCarCmd, pCar->CutOffState);
			break;

		default:
			break;
	}
}

//车辆离秤缴费状态响应外部事件处理函数
static void Car_Business_CarFarPay(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;

		//缴费通知
		case Msg_Cmd_PayNotify:
			CarQueue_Car_Kill(pCar);
			break;

		//分段采集
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;

		default:
			break;
	}
}


//长车状态响应外部事件处理函数
static void Car_Business_LongCarState(CarInfo *pCar, TaskMsg *pMsg)
{
	AlexRecoder *tmpAxle = NULL; 
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		//节拍
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;

		//光幕释放
		case Msg_LC2_Leave:
			if(Get_LongCar_Mode() == AutoLongCarMode) 
			{
				//长车自动称重模式
				pCar->nScalerKgIn = Get_Static_Weight();
				
				//算轴数
				pCar->nAxleNum = Apply_LongCar_Axle(pCar);
				if(pCar->nAxleNum <= 0)
				{
					debug(Debug_Business, "超长车轴数低于0, 视为倒车!\r\n");
					CarQueue_Car_Kill(pCar);
					break;
				}
				pCar->CutOffState = 1;

				//算超长车重量
    			Calc_LongCar_Wet(pCar);
				pCar->bWetVaild = TRUE;
    				
				//立即上传重量
				Send_CarInfo(pCar);

				//切换至待缴费状态
#ifdef _SIMULATION_NOPAY_
				FSM_Change_CarState(pCar, stCarLeaving);
#else
				FSM_Change_CarState(pCar, stCarWaitPay);
#endif
			} 
			else
			{
				debug(Debug_Business, "分段超长车断尾!\r\n");
				pCar->nAxleNum = Apply_LongCar_Axle(pCar);
				if(pCar->nAxleNum <= 0) 
				{
					CarQueue_Car_Kill(pCar);
					break;
				}

				pCar->CutOffState = 1;
			}
			break;
		
		//加轴
		case Msg_PreAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= gWetPar.nMaxAxleNum))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_peak = tmpAxle->AlexKgRef;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}

				pCar->nAxleNum++;
			}
			break;

		//减轴
		case Msg_PreAlex_Remove:
			pCar->nAxleNum--;
			break;

		//后轴识别器
		case Msg_BackAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			pCar->nDownAlexMaxKg = tmpAxle->AlexKgRef;

			pCar->nBackAxleNum++;
			//if(pCar->nBackAxleNum >= pCar->nAxleNum)
			//{
			//	ResetTurnTick();
            //    FSM_Change_CarState(pCar, stCarFarPay);
			//}
            break;

		//下秤退轴
		case Msg_BackAlex_Remove:
			if(pCar->nBackAxleNum > 0) pCar->nBackAxleNum--;

			if(Get_LongCar_Mode() == AutoLongCarMode)
			{
				if(pCar->nBackAxleNum == 0)
				{
					FSM_Change_CarState(pCar, stCarComing);
				}
			}
            break;    

		//长车缴费通知事件（长车自动模式下异常, 手动模式下正常）
		case Msg_Cmd_PayNotify:
		    FSM_Change_CarState(pCar, stCarLeaving);
			break;

		//地感释放消息（异常时才能收到）
		case Msg_GC2_Leave: 
			//后地感释放,光幕一定该是遮挡的
			if(Get_LC_Status(2))
			{
				//超长车状态下光幕必然是遮挡的
				if(pCar->pNext == NULL)
				{
					//后面没车，认为是地感误信号
					FSM_Change_CarState(pCar, stCarComing);

					debug(Debug_Business, "!!异常超长车, 后地感释放\r\n");
				}
				else
				{
					//未知异常，后面有车
					FSM_Change_CarState(pCar, stCarComing);

					debug(Debug_Business, "!!异常超长车, 不可能后面有车\r\n");
				}
			}
			else
			{
				//未知异常，光幕没遮挡
				FSM_Change_CarState(pCar, stCarComing);

				debug(Debug_Business, "!!异常超长车, 不可能光幕未遮挡\r\n");
			}

#ifdef _SIMULATION_NOPAY_
			CarQueue_Car_Kill(pCar);
#endif
			break;

		//分段采集(未审核)
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;
            
		default:
			break;
	}	
}





//empty function
static void Car_Business_None(CarInfo *pCar, TaskMsg *pMsg)
{
	return;
}

struct _BusinessArray 
{
	uint8 state;
	CarBusinessFun fun;
};

static const struct _BusinessArray BusinessArray[] = 
{
	{stCarNone, 	Car_Business_None},
	{stCarComing, 	Car_Business_CarComState},
	{stCarEnter, 	Car_Business_CarEnterState},	
	{stCarInScaler, Car_Business_CarInScalerState},
	{stCarWaitPay,  Car_Business_CarWaitPayState},
	{stLongCar,		Car_Business_LongCarState},
	{stCarLeaving,	Car_Business_CarLeave},
	{stCarFarPay,	Car_Business_CarFarPay}
};

void FSM_Change_CarState(CarInfo *pCar, int32 state)
{
	if(pCar == NULL)
		return;

	if(pCar->bCarIsDead == TRUE)
		return;

	if(pCar->nStatus == state) 
		return;

	if(state > stCarFarPay)
	   	return;

	pCar->nPreStatus = pCar->nStatus;
	pCar->nStatus = state;
	pCar->nStatusKeepTime = 0;
	pCar->CarFSMFun = BusinessArray[state].fun;
}

//重复采集
void Car_Business_RepeatCmd(CarInfo *pCar, TaskMsg *msg) 
{
	CarInfo *tCar = pCar;
	
	if(tCar != NULL) 
	{
		if(tCar->nStatus == stCarLeaving)
		{
			if(tCar->pNext == NULL)
			{
				//队列只有一个车且已经交费
				AbnormalAckInfo_To_SDK(RepeatCarCmd, 0);
				return;
			}
			tCar = tCar->pNext;
		}
		
		if(tCar->nStatus <= stCarEnter)
		{
			AbnormalAckInfo_To_SDK(RepeatCarCmd, 0);
			return;
		}
		tCar->nBestKg = Get_Static_Weight();
		tCar->bResponse = 0;
		tCar->cmd = RepeatCarCmd;
		//这里重点测试，Send_CarInfo是防重复的
		Send_CarInfo(tCar);
	} 
	else 
	{
		AbnormalAckInfo_To_SDK(RepeatCarCmd, 0);
	}
}

//发送一个模拟的车辆信息
void Send_Simulate_CarInfo(uint8 axle, int32 wet, int cmd)
{
	CarInfo *pCar = NULL;
	
	pCar = CarQueue_Create_Car();
	if(pCar != NULL) 
	{
		pCar->nBestKg = wet;
		pCar->nAxleNum = axle;
		pCar->cmd = cmd;
		pCar->speed = 33;
		
		Send_CarInfo(pCar);
		pCar->bUse = FALSE;		//删除车辆
	}
}

//SDK主动采集命令下队列无车或是车辆已经断尾
void AbnormalAckInfo_To_SDK(uint8 cmd, uint8 CutOffState)
{
	CarInfo *pCar = NULL;

	debug(Debug_Business, "AckCmd:%d采集异常,CutOffState:%d!\r\n", cmd, CutOffState);
	pCar = CarQueue_Create_Car();
	if(pCar != NULL) 
	{
		pCar->nAxleNum = 0;
		pCar->cmd = cmd;
		pCar->bAckErr = 1;
		pCar->CutOffState = CutOffState;
		
		Send_CarInfo(pCar);
		pCar->bUse = FALSE;		//删除车辆
	}
}


//超长车判断
#define LONG_CAR_MAX_COUNT		20
static int32 nLongCarWetArr[LONG_CAR_MAX_COUNT];
static int32 nLongCarWetInit = 0;
static BOOL bIsLongCar = 0;
static int32 nLongCarMax;
static int32 nLongCarMin;
static int32 nLongCarMaxPos;
static int32 nLongCarMinPos;
static int32 nLongCarSum;
static int32 nLongCarAvg;
static int32 nLongCarCounter;
void LongCarRecognize(int32 nWet)	//调用频率，100Hz
{
	int i;
	
	nLongCarSum += nWet;
	nLongCarCounter++;
	if(nLongCarCounter < (100 / LONG_CAR_MAX_COUNT)) return;

	nLongCarAvg = nLongCarSum / nLongCarCounter;
	nLongCarSum = 0;
	nLongCarCounter = 0;

	//初始化
	if(nLongCarWetInit == 0)
	{
		for(i = 0; i < LONG_CAR_MAX_COUNT; i++)
		{
			nLongCarWetArr[i] = nLongCarAvg;
		}
		
		bIsLongCar = 0;
		nLongCarWetInit = 1;
	}
	
	//移位
	else
	{
		for(i = 0; i < LONG_CAR_MAX_COUNT-1; i++)
		{
			nLongCarWetArr[i] = nLongCarWetArr[i+1];
		}	
		nLongCarWetArr[LONG_CAR_MAX_COUNT-1] = nLongCarAvg;
	}
	
	//获取最大值最小值位置
	nLongCarMax = -100000;
	nLongCarMin = 150000;
	for(i = 0; i < LONG_CAR_MAX_COUNT; i++)
	{
		//最大值
		if(nLongCarWetArr[i] > nLongCarMax)
		{
			nLongCarMax = nLongCarWetArr[i];
			nLongCarMaxPos = i;
		}
		
		//最小值
		if(nLongCarWetArr[i] < nLongCarMin)
		{
			nLongCarMin = nLongCarWetArr[i];
			nLongCarMinPos = i;
		}	
	}
	
	//判断下降趋势
	if(nLongCarMaxPos < nLongCarMinPos)
	{
		//判断下降幅度超过2.6吨
		if((nLongCarMax - nLongCarMin) > 2600)
		{
			bIsLongCar = 1;
		}
	}
}

BOOL IsLongCar(void)
{
	return bIsLongCar;	
	//return FALSE;
}

void LongCarReset(void)
{
	nLongCarSum = 0;
	nLongCarCounter = 0;
	bIsLongCar = 0;
	nLongCarWetInit = 0;
}


