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

static OvertimeData t_carinfo = {{0}};

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



//超速重量修正函数
static void Weight_Dynamic_Modify(CarInfo *pCar)
{	
	if(pCar == NULL) return;
	
	if(pCar->speed <= 50)
		pCar->nBestKg = (float)pCar->nBestKg * (float)gWetPar.Speed_0k  / 1000.0f;
	else if(pCar->speed > 50 && pCar->speed <= 100) 
		pCar->nBestKg = (float)pCar->nBestKg * (float)gWetPar.Speed_5k  / 1000.0f;
	else if(pCar->speed > 100 && pCar->speed <= 150) 
		pCar->nBestKg = (float)pCar->nBestKg * (float)(gWetPar.Speed_10k) / 1000.0f;
	else if(pCar->speed > 150 && pCar->speed <= 200) 
		pCar->nBestKg = (float)pCar->nBestKg * (float)(gWetPar.Speed_15k) / 1000.0f;
	else if(pCar->speed > 200 && pCar->speed <= 250) 
		pCar->nBestKg = (float)pCar->nBestKg * (float)(gWetPar.Speed_20k) / 1000.0f;
	else if(pCar->speed > 250 && pCar->speed <= 300) 
		pCar->nBestKg = (float)pCar->nBestKg * (float)(gWetPar.Speed_25k) / 1000.0f;
	else 
		pCar->nBestKg = (float)pCar->nBestKg * (float)(gWetPar.Speed_30k) / 1000.0f;
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
		pCar->AxleInfo[i].axle_wet = tmp;
	
	pCar->AxleInfo[pCar->nAxleNum-1].axle_wet = pCar->nBestKg - i*tmp;
}

//车辆重量,轴数异常修正
static void Modify_Abnormal_CarInfo(CarInfo *pCar)
{
	static uint8 cnt = 1;

	if(pCar == NULL) return;
	
	cnt++;
	if(cnt > 6) cnt = 1;

	if(pCar->nBestKg < 850) 		//车辆重量小于850kg, 取随机重量1050~1800
		pCar->nBestKg = 900 + cnt * 150;
	
#if 0
	if(pCar->nBestKg < 7000) { 	//经验判断, 3轴以上的车不存在小于7吨的情况
		if(pCar->nAxleNum > 2) {
			pCar->nAxleNum = 2;
			i = 2;
		}
	}
#endif

	if(pCar->nAxleNum < 2) {		//修正轴数小于2的情况，根据重量取默认轴数
		if(pCar->nBestKg < 8000) 
			pCar->nAxleNum = 2;
		else
			pCar->nAxleNum = 6;
	}	

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

	for(i = 0; i < pCar->nAxleNum; i++)
		sum += pCar->AxleInfo[i].axle_wet;
	
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

//待发送车辆信息入队
static void Send_CarInfo(CarInfo *pCar)
{
	uint8 i = 0;
	uint8 TireOK = 0;
	TaskMsg msg = {0};
	GuiMsgInfo gui_msg = {0};
	char datebuf[30] = {0};
	int8 orig_axle = pCar->nAxleNum;
	int32 orig_wet = pCar->nBestKg;
	CSysTime *time = Get_System_Time();

	if(pCar == NULL) return;
	if(pCar->bResponse == 1) return;

	pCar->speed = Get_CarSpeed();
	debug(Debug_Business, "speed: %d.%dkm/h, Axlelength: %d\r\n", pCar->speed/10, pCar->speed%10, gWetPar.AxleLength);

	memset(&t_carinfo, 0, sizeof(OvertimeData));

	//异常车辆信息纠正
	if((pCar->cmd != LongCarCmd) || (pCar->cmd != BackCarCmd))
		Modify_Abnormal_CarInfo(pCar);

	//判断是否超限
	if(orig_wet > (int)gWetPar.MaxWet)	t_carinfo.ScalerOverWetFlag = 1;
	
	t_carinfo.cmd = pCar->cmd;

	//判断是否超载
	Charge_Car_OverWeight(pCar, &t_carinfo);

	debug(Debug_Business, "\r\n>>上送车辆信息, %ldms\r\n",Get_Sys_Tick());
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

		//根据速度补偿车辆重量
		Weight_Dynamic_Modify(pCar);

		//车辆重量分度值处理
		pCar->nBestKg = WetDrvProc(pCar->nBestKg, gWetPar.StaticDivValue);
		
		if(pCar->cmd == LongCarCmd) 
		{
			t_carinfo.LongCarSegs  = pCar->nLongCarSegs;
			t_carinfo.LongCarReady = pCar->bLongCarReady;

			//长车轴数异常保护
			if(pCar->nAxleNum == 0) pCar->nAxleNum = 2;
		}
		
		//计算轴重
		Calc_Car_AxleWet(pCar);

		t_carinfo.TotalWet = pCar->nBestKg;			//总重
		t_carinfo.AxleNum  = pCar->nAxleNum;		//轴数
		t_carinfo.AxleGroupNum = pCar->nAxleNum;	//轴组数
		t_carinfo.CarStatus = pCar->CutOffState;	//车辆状态

		debug(Debug_Business, "*Post CarInfo{wet=%d,axle=%d,speed=%d,%s,%s}\r\n", 	    	
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
		
		//后面的轴型
		//当轮轴识别器正常的时候，按轮轴识别器结果处理单双轮
		//if(Get_Tire_ErrStatus() && (TireOK == 1))
		//{
		//	//完全正确情况下使用识别出的单双轮信号
		//	for(i = 1; i < pCar->nAxleNum; i++)
		//	{
		//		t_carinfo.AxleType[i] = pCar->AxleInfo[i].axle_type + 1;
		//	}
		//}

		//
		//else
		{	
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
				t_carinfo.AxleType[2] = 2; 
				t_carinfo.AxleType[3] = 2; 
				t_carinfo.AxleType[4] = 2; 
				t_carinfo.AxleType[5] = 2; 
			}
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
			
			if(t_carinfo.AxleLen[i] <= 0)
			{
				t_carinfo.AxleLen[i] = 20;
			}
		}
		
		//发送次数清零
		t_carinfo.SendNum = 0;
		
		//加速度取默认值
		t_carinfo.speeda = 0;
		t_carinfo.speed = pCar->speed;
		
		//业务线程消息号
		msg.msg_id = Msg_CarInfo_Ready;
		
		//GUI线程消息号
		gui_msg.ID = WM_CARIN;
		
		debug(Debug_Business, "*CarQueueLen=%d; seri=%d, orig_wet=%d, orig_axle=%d!\r\n",	
			CarQueue_Get_Count(), OverQueue_GetSeriID(), orig_wet, orig_axle);
		//打印轴重
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			debug(Debug_Business, "*AxleWet%d: %d; ", i, pCar->AxleInfo[i].axle_wet);
		}
		debug(Debug_Business, "\r\n");
		
		//打印实际胎型
		debug(Debug_Business, "*OrigTyretype: \r\n");
		for(i = 0; i < pCar->nAxleNum; i++)
		{
			debug(Debug_Business, "%d, %d\r\n", pCar->AxleInfo[i].axle_type + 1, pCar->AxleInfo[i].axle_len);
		}
		debug(Debug_Business, "\r\n");

		//打印胎型
		debug(Debug_Business, "*SendTyretype: \r\n");
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
		debug(Debug_Business, "*Send BackCarInfo!\r\n\r\n");
	}	

	t_carinfo.SendMod = SendActiveMod;

	//车辆信息入队待发送数据队列
	if(OverQueue_Enqueue(&t_carinfo) == 0)
	{
		//入队成功,置车辆已发送标志(入队后由OverQueue管理,CarQueue认为该车已上送)
		pCar->bResponse = 1;

		//给业务数据管理线程发送消息
		Task_QPost(&DataManagerMsgQ,  &msg);

		//车辆信息已发送,打印车辆队列信息
		if(pCar->cmd != BackCarCmd)
		{
			debug(Debug_Business, "\r\n>>软件收尾\r\n");
			CarQueue_Print_Info();
		}
	
		//向主当前窗口发送加载消息
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

//跟车重量计算
static void Calc_FollowCar_Wet(CarInfo *pCar)
{
	int32 wet = 0, axlewet = 0;	
	uint8 i = 0;
	
	if(pCar == NULL) return;
	
	if(pCar->nAxleNum == 6) 
	{
		if((!pCar->bWetVaild) || (pCar->nBestKg < 8000)) 
		{
			pCar->nBestKg = Get_Static_Weight();
			pCar->WetLevel = LevelSingleCarStable;
			debug(Debug_Business, ">FollowCar, Axles=6, Get ScalerWet=%d\r\n", pCar->nBestKg);
		}
		return;
	}

	//已经保存重量,看是否有机会更新重量
	if(pCar->bWetVaild) 
	{
		if(CarQueue_Get_OnScaler_Count() == 1) 
		{
			if(Get_Static_Weight() > 850) 
			{
				pCar->nBestKg = Get_Static_Weight();
				pCar->WetLevel = LevelSingleCarBeginDown;
				debug(Debug_Business, ">FollowCar Get ScalerWet:%d,tick=%ld", pCar->nBestKg,Get_Sys_Tick());
			}
		}
		return;
	}			

	//计算轴重
	for(i = 0; i < pCar->nAxleNum; i++)
		axlewet += pCar->AxleInfo[i].axle_wet;

	wet = pCar->nScalerKgDown - pCar->nScalerKgComing;	//光幕触发释放前后秤台静态重量差值
//	if(wet + 500 > axlewet) 
	if(wet > ((float)axlewet * 0.9f)) 
	{
		pCar->nBestKg = wet;
		pCar->WetLevel = LevelMultiCar;
	} 
	else 
	{
		pCar->nBestKg = axlewet;
		pCar->WetLevel = LevelAxleWet;
	}
	debug(Debug_Business, ">跟车计重,AxleWet=%d,UpDownWet=%d,tick=%ld", axlewet, wet, Get_Sys_Tick());
}

//跟车处理
//后车触发光幕，或车辆触发后地感，保存车辆信息
static void Save_FollowCar_Info(CarInfo *pCar)
{	
	if(pCar == NULL) return;

	if(pCar->bResponse == 1) return;
	
	debug(Debug_Business, "save follocar info!\r\n");
	if(pCar->nScalerKgDown == 0)
		pCar->nScalerKgDown = Get_Static_Weight();

	Calc_FollowCar_Wet(pCar);

	//触发地感保存后清除延时计重信息
	if(!pCar->bWetVaild) 
	{
		pCar->bWetVaild = TRUE;
		pCar->bWetTimeFlag = FALSE;
		pCar->nWetKeepTime = 0;
		//pCar->WetFlag = NoneWetFlag;	
	}
	
	Send_CarInfo(pCar);
}

static void LongCar_Segs_Business(CarInfo *pCar)
{
	if(pCar == NULL) return;

	pCar->nAxleNum = Apply_LongCar_Axle(pCar);
	if(pCar->nAxleNum <= 0) 
	{
		if(!pCar->bLongCarReady) 
		{
			CarQueue_Car_Kill(pCar);
			return;
		} 
	} 
	else 
	{
		pCar->nBestKg = Get_Static_Weight();
		pCar->cmd = LongCarCmd;
		pCar->WetLevel = LevelLongCar; 
		pCar->bWetVaild = TRUE;
	}
	Send_CarInfo(pCar);
	
	if(!pCar->bLongCarReady) 
	{
		pCar->nStatus = stCarLeaving;
		CarQueue_Car_Kill(pCar);			//前面分段信息上送后认为其已缴费离开，立即删掉
	}
}

//车辆取重量计时
static void Car_GetWet_TimerBusiness(CarInfo *pCar)
{
    BOOL flag = FALSE;
    static int preMaxWet = 0;
    
	if(pCar == NULL) return;
	
	if(pCar->bWetTimeFlag) 
	{
	    pCar->nWetKeepTime += CAR_TICK;
	    switch(pCar->WetFlag) 
	    {  
    		case FollowWetFlag:                   //跟车延时计重     
    			if(pCar->nWetKeepTime >= CAR_WET_TICK) 
    			{
    				pCar->nScalerKgDown = Get_Static_Weight();
    				Calc_FollowCar_Wet(pCar); 
    				pCar->WetLevel = LevelMultiCar;
    				pCar->bWetVaild = TRUE;
					//Send_CarInfo(pCar);
    				flag = TRUE;
    			}
    			break;
    		 
    		case SingleWetFlag:                //单车稳定计重
			case SingleUnstableFlag:			//单车未稳定计重															 
				preMaxWet = Get_Static_MaxWeight();
    			
    			if(pCar->nWetKeepTime >= CAR_WET_TICK) 
    			{
    				if(Get_Static_Weight() < preMaxWet - 500) 
    				{
    					pCar->nBestKg = preMaxWet;
						debug(Debug_Business, "Get max ScalerWet:%d!\r\n",preMaxWet);
    				} 
    				else 
    				{
    					pCar->nBestKg= Get_Static_Weight();
						debug(Debug_Business, "Get current ScalerWet:%d! MaxWet:%d\r\n",
							pCar->nBestKg,preMaxWet);
					}
    				pCar->WetLevel = LevelSingleCarStable;
    				pCar->bWetVaild = TRUE;
    				preMaxWet = 0;
					Clear_ScalerMaxWeight();
    				Send_CarInfo(pCar);
    				flag = TRUE;
    			}
    			break;
    		default:
    		    break;
    	}
    	
    	if(flag) 
    	{
    	    pCar->bWetTimeFlag = FALSE;
    		pCar->nWetKeepTime = 0;
    		//pCar->WetFlag = NoneWetFlag;
    	}
	}//end if(pCar->bWetTimeFlag)
}

//车辆断尾后延时对轴数进行判断
static int Car_CutOffState_Fun(CarInfo *pCar)
{
	int32 naxle = 0;
	int ret = 0;
	OvertimeData *pLastCar = NULL;

	if(pCar == NULL) return 2;

	pCar->bAxleValid = TRUE;
	naxle = Apply_Car_Axle(pCar);
	if(naxle > 0) 
	{ 							
		pCar->nAxleNum = naxle;	//正常上车
	} 
	else if(naxle < 0 ) 
	{
		debug(Debug_Business, "BackCar, TotalCarNumber=%d\r\n", CarQueue_Get_Count());
		CarQueue_Car_Kill(pCar);		//倒车
		pCar->cmd = BackCarCmd;
	
		if(pCar->pPrev != NULL) 		//删除前面的车应当大致匹配轴数
		{
			if(naxle + pCar->pPrev->nAxleNum <= 2)
			{
				pLastCar = OverQueue_Get_Tail();
				if(pLastCar != NULL)
				{
					//车辆数据还未上送，且队尾不是倒车信息删掉
					if((pLastCar->cmd != BackCarCmd) && (pLastCar->bSend == 0))
					{		
						OverQueue_Remove_Tail();
						debug(Debug_Business, "OverQueue阻塞,不发送倒车信息!\r\n");
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
		ret = 1;
	} 
	else	
	{		
		debug(Debug_Business, "外部干扰!\r\n");
		CarQueue_Car_Kill(pCar);		//外部干扰	
		ret = 2;
	}			

	return ret;
}

//车辆到达状态响应外部事件处理函数
static void Car_Business_CarComState(CarInfo *pCar, TaskMsg *pMsg)
{	
	AlexRecoder *tmpAxle = NULL;
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			FSM_Change_CarState(pCar, stLongCar);
			LongCar_Segs_Business(pCar);
			break;
			
		case Msg_LC2_Leave:
			pCar->bWetTimeFlag = TRUE;
			pCar->CutOffState = 1;
//			if(pCar->WetFlag != SingleWetFlag)	   //??
//				pCar->WetFlag = FollowWetFlag;		
			FSM_Change_CarState(pCar, stCarInScaler);
			break;

		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			Car_GetWet_TimerBusiness(pCar);
			break;

		case Msg_PreAlex_Come:
			if(pCar->pNext == NULL)
			{
				if(pCar->nAxleNum == 0)
				{
					//记录初始重量
					pCar->nScalerKgComing = Get_Static_Weight();
					if(pCar->nScalerKgComing < 500) 
					{		
						pCar->WetFlag = SingleWetFlag;		//新车未入队，置单车标志
						Clear_ScalerMaxWeight();		
					}						
				}
			}
			break;

		case Msg_PreAlex_Add:
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= 6))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}

				pCar->nAxleNum++;
			}
			break;

		case Msg_PreAlex_Remove:
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				pCar->nAxleNum--;
			}
			break;

		case Msg_GC2_Enter:
		case Msg_BackAlex:	
			if(Get_LongCar_Mode() == AutoLongCarMode) 
			{		
				if(CarQueue_Get_OnScaler_Count() == 1 &&
				    pCar->nStatus == stCarComing) 
				{
					if(pMsg->msg_id == Msg_BackAlex)
						pCar->nBackAxleNum++;
					
					pCar->nLongCarHalfWet = Get_Static_Weight();
					pCar->nLongCarHalfAxle = Apply_LongCar_Axle(pCar);
					
//					Clear_Axle_Recoder();   //如果后地感闪变会异常
					FSM_Change_CarState(pCar, stLongCar);
				}
			}
			break;

		default:
			break;
	}
}

//车辆完全上秤状态响应外部事件处理函数
static void Car_Business_CarInScalerState(CarInfo *pCar, TaskMsg *pMsg)
{
	AlexRecoder *tmpAxle = NULL;
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			if(!pCar->bAxleValid)
			{
				if(pCar->nStatusKeepTime >= CAR_WET_TICK - 10)
				{
					if(Car_CutOffState_Fun(pCar) != 0)
						break;
				}
			}
				
			Car_GetWet_TimerBusiness(pCar);
			break;

		case Msg_LC2_Enter:
			if(!pCar->bAxleValid)
			{
				if(Car_CutOffState_Fun(pCar) != 0)
					break;
			}
				
			if(pCar->WetFlag == SingleWetFlag) 
			{
				//pCar->WetFlag = SingleUnstableFlag;
				pCar->WetLevel = LevelSingleCarBy;
			} 
			else if((pCar->WetFlag == FollowWetFlag) ) //&& (pCar->nAxleNum > 0) && (pCar->cmd != BackCarCmd)) 
			{
				if(!pCar->bWetVaild)
					Save_FollowCar_Info(pCar);
				else if(pCar->bResponse == 0)
					Send_CarInfo(pCar);
			}
		    break;
		    
        case Msg_BackAlex:
		case Msg_GC2_Enter:
			//车辆断尾后60ms内不可能下秤,防止后地感闪变
			//if(pCar->nStatusKeepTime < 60) break;

			if(!pCar->bAxleValid)
			{
				if(Car_CutOffState_Fun(pCar) != 0)
					break;
			}

		    if(pCar->WetFlag == SingleWetFlag) 	
		    {
				//pCar->WetFlag = SingleUnstableFlag;
				pCar->WetLevel = LevelSingleCarBeginDown;
			} 
			else if((pCar->WetFlag == FollowWetFlag)) //&& (pCar->nAxleNum > 0) && (pCar->nBestKg!= BackCarCmd))
			{
				if(!pCar->bWetVaild)
					Save_FollowCar_Info(pCar);
				else if(pCar->bResponse == 0)
					Send_CarInfo(pCar);
			}

			if(pMsg->msg_id == Msg_BackAlex)
				pCar->nBackAxleNum++;
#ifdef _SIMULATION_NOPAY_
			FSM_Change_CarState(pCar, stCarLeaving);
#else
			FSM_Change_CarState(pCar, stCarWaitPay);
#endif
			break;

		case Msg_Cmd_PayNotify:
		    FSM_Change_CarState(pCar, stCarLeaving);
			break;

		case Msg_PreAlex_Add:
			if(pCar->bWetVaild) break;

			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= 6))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}
				pCar->nAxleNum++;
			}
			break;

		case Msg_PreAlex_Remove:
			if(pCar->bWetVaild) break;
			
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				pCar->nAxleNum--;
			}
			break;

		default:
			break;
	}		
}

//长车状态响应外部事件处理函数
static void Car_Business_LongCarState(CarInfo *pCar, TaskMsg *pMsg)
{
	int32 sum = 0, i = 0;
	AlexRecoder *tmpAxle = NULL; 
	
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			break;

		case Msg_Cmd_PlusSave:
			pCar->nLongCarSegs = pMsg->msg_ctx;
			LongCar_Segs_Business(pCar);
			break;
 
		case Msg_GC2_Leave: 
			//后地感释放,光幕遮挡,调整车辆状态
			if(Get_LC_Status(2))
			{
				if(pCar->pNext == NULL)
				{
					//后面没车，认为是地感误信号
					FSM_Change_CarState(pCar, stCarComing);
				}
				else
				{
					FSM_Change_CarState(pCar, stCarFarPay);
				}
			}
			else
			{
				FSM_Change_CarState(pCar, stCarFarPay);
			}

#ifdef _SIMULATION_NOPAY_
			CarQueue_Car_Kill(pCar);
#endif
			break;

		//长车缴费通知事件
		case Msg_Cmd_PayNotify:
		    FSM_Change_CarState(pCar, stCarLeaving);
			break;

		case Msg_LC2_Leave:
			if(Get_LongCar_Mode() == AutoLongCarMode) 
			{	
				//长车自动称重模式
				pCar->nAxleNum = Apply_LongCar_Axle(pCar);
				if((pCar->nAxleNum - pCar->nLongCarHalfAxle) <= 0) 
				{
					pCar->nBestKg = pCar->nLongCarHalfWet;
				}
				else
				{
					sum = 0;
					for(i = 0; i < pCar->nAxleNum; i++)
					{
						sum += pCar->AxleInfo[i].axle_wet;
					}

					pCar->nBestKg = sum;
				}
				
				pCar->CutOffState = 1;
				pCar->bWetVaild = TRUE;
				pCar->WetLevel = LevelAutoLongCar;
				debug(Debug_Business, "AutoLongCar CutOff!\r\n");
				Send_CarInfo(pCar);
			} 
			else 
			{
				pCar->nAxleNum = Apply_LongCar_Axle(pCar);
				if(pCar->nAxleNum <= 0) 
				{
					if(!pCar->bLongCarReady) 
					{
						CarQueue_Car_Kill(pCar);
						break;
					} 
				} 
				pCar->bLongCarReady = TRUE;
				pCar->CutOffState = 1;
				debug(Debug_Business, "HandleLongCar CutOff!\r\n");
			}
			break;
			
		case Msg_PreAlex_Add:
			if(pCar->bWetVaild) break;

			//不记录轴信息会导致长车按轴重取重变轻
			tmpAxle = (AlexRecoder *)pMsg->msg_ctx;
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				if(tmpAxle != NULL)
				{
					if((pCar->nAxleNum >= 0) && (pCar->nAxleNum <= 6))
					{
						pCar->AxleInfo[pCar->nAxleNum].axle_wet = tmpAxle->AlexKg;
						pCar->AxleInfo[pCar->nAxleNum].axle_type = tmpAxle->bDouble;
					}
				}
				pCar->nAxleNum++;
			}
			break;
		
		case Msg_PreAlex_Remove:
			if(pCar->nAxleNum < MAX_AXLE_NUM)
			{
				pCar->nAxleNum--;
			}
			break;

		case Msg_BackAlex:
			pCar->nBackAxleNum++;
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
                FSM_Change_CarState(pCar, stCarFarPay);
            break;
            
		default:
			break;
	}	
}

static void Car_DealDoubleNextCar(CarInfo *pCar)
{
	if(pCar == NULL) return;
	if(pCar->pNext == NULL) return;

				
	if((pCar->pNext->WetFlag == FollowWetFlag) ) //&& (pCar->pNext->nAxleNum > 0)&& (pCar->pNext->cmd != BackCarCmd))
	{
		if(pCar->pNext->nStatus == stCarInScaler)
		{
			if(pCar->pNext->bWetVaild == 0)
			{
				pCar->pNext->WetFlag = SingleWetFlag;
				pCar->pNext->nScalerKgComing = 0;
			}
			else
			{
				if(pCar->pNext->bResponse == 0)
				{
					Save_FollowCar_Info(pCar->pNext);		//开启用于标定
				}
			}
		}
		else if(pCar->pNext->nStatus == stCarComing)
		{
			pCar->pNext->WetFlag = SingleWetFlag;
			pCar->pNext->nScalerKgComing = 0;
		}	
	}

}

//车辆等待收费状态响应外部事件处理函数
static void Car_Business_CarWaitPayState(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			Car_GetWet_TimerBusiness(pCar);
			break;

		case Msg_Cmd_PayNotify:
		    FSM_Change_CarState(pCar, stCarLeaving);
			break;

		case Msg_BackAlex:
			pCar->nBackAxleNum++;
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
                FSM_Change_CarState(pCar, stCarFarPay);
            break;

       	case Msg_GC2_Leave:
			//大于2轴的车不可能离秤缴费
			//if(pCar->nAxleNum > 2) break;
			
       		FSM_Change_CarState(pCar, stCarFarPay);
       	
       		if(pCar->pNext != NULL)
       		{
				Car_DealDoubleNextCar(pCar);
			}
			
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
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;
			Car_GetWet_TimerBusiness(pCar);
			break;

		case Msg_Cmd_PayNotify:
			CarQueue_Car_Kill(pCar);
			break;

		default:
			break;
	}
}


//车辆缴费离开状态响应外部事件处理函数
static void Car_Business_CarLeave(CarInfo *pCar, TaskMsg *pMsg)
{
	if((pCar == NULL) || (pMsg == NULL)) return;
	
	switch(pMsg->msg_id) 
	{
		case Msg_Tick:
			pCar->nStatusKeepTime += CAR_TICK;
			pCar->nLiveTime += CAR_TICK;	
#ifdef _SIMULATION_NOPAY_
			Car_GetWet_TimerBusiness(pCar);
#endif
			break;

		case Msg_GC2_Leave: 
       		if(pCar->pNext != NULL)
       		{
				Car_DealDoubleNextCar(pCar);
			}

			CarQueue_Car_Kill(pCar);
			break;

		case Msg_BackAlex:
			pCar->nBackAxleNum++;
			if(pCar->nBackAxleNum >= pCar->nAxleNum)
                CarQueue_Car_Kill(pCar);
			break;					//fatal err;

        case Msg_Cmd_PayNotify:
			CarQueue_Car_Kill(pCar);
			if(pCar->pNext != NULL) 
			{
				FSM_Change_CarState(pCar->pNext, stCarLeaving);
				debug(Debug_Business, "First car is already Leave, maybe the second car pay!\r\n");
			}
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

struct _BusinessArray {
	uint8 state;
	CarBusinessFun fun;
};

static const struct _BusinessArray BusinessArray[] = {
	{stCarNone, 	Car_Business_None},
	{stCarComing, 	Car_Business_CarComState},
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

	if(pCar->nStatus == state) 
		return;

	if(state > stCarFarPay)
	   	return;

	debug(Debug_Business, "CarState change : %s -> %s\r\n", Get_Car_StatusName(pCar->nStatus), Get_Car_StatusName(state));
	pCar->nPreStatus = pCar->nStatus;
	pCar->nStatus = state;
	pCar->nStatusKeepTime = 0;
	pCar->bStateChanged = TRUE;
	pCar->CarFSMFun = BusinessArray[state].fun;
}

void Car_Business_RepeatCmd(CarInfo *pCar, TaskMsg *msg) 
{
	if(pCar != NULL) 
	{
		pCar->nBestKg = Get_Static_Weight();
		Send_CarInfo(pCar);
	} 
	else 
	{
		if(Get_Static_Weight() > 500) 
		{
			pCar = CarQueue_Create_Car();
			if(pCar != NULL) 
			{
				pCar->nBestKg = Get_Static_Weight();
				pCar->cmd = RepeatCarCmd;
				Send_CarInfo(pCar);
			}
		}
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
	}
}

