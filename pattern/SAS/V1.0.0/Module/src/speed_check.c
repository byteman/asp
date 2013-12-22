#include "includes.h"
#include "device_info.h"
#include "speed_check.h"

static int16 CarSpeed = 66;
static uint16 length = 1500;				//光幕地感距离mm

/* 速度计算函数 */
void Check_CarSpeed (void)
{
#if 0
	static uint16 nCount_2ms = 0;						//用于计算2ms定时次数
	static BOOL carCome = FALSE;						//来车标志,局部变量
	static BOOL keep = FALSE;

	//获取前地感线圈或者前光幕触发状态	
	if(TRUE == Get_GC_Status(1) || TRUE == Get_LC_Status(1))		
	{
		if(keep) {
			carCome = TRUE;								//前地感被触发,置来车标志
			nCount_2ms++;								//开始2ms计数
		}															
	}
	else														//前地感没有被触发
	{
		keep = TRUE;
		if(carCome)			//光幕触发前地感还未触发后光幕，清数据
		{
			carCome = FALSE;
			nCount_2ms = 0;
		}
	}
	
	if(Get_LC_Status(2) && (carCome))					//后光幕触发
	{
		carCome = FALSE;								//清除来车标志
		keep = FALSE;
		//计时太小不计算速度
		if(nCount_2ms > 10)
			CarSpeed = 72*(length/nCount_2ms);			//7.2*length/cnt (km/h)计算车辆速度
		nCount_2ms = 0;									//2ms计数清零
	}
#endif
}

/* 速度获取函数 */
uint16 Get_CarSpeed(void)
{
	if(CarSpeed < 20)
		CarSpeed = 20;

	if(CarSpeed > 800)
		CarSpeed = 300;

	return CarSpeed;
}

void Set_Speed_Length(uint16 len)
{
	length = len;
//	Param_Write();
}

uint16 Get_Speed_Length(void)
{
	return length;
}

void Set_CarSpeed(int32 speed)
{
	CarSpeed = speed;
}


