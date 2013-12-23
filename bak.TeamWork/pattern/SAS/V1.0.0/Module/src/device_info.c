#include "device_info.h"



static volatile DeviceStatus curStatus={0};
static volatile int ADChipStatus = 0;

/****************************************
* 说	明：更新所有设备状态信息到静态变量，
* 已按《称重控制端与SDK通讯协议》排列好
****************************************/
//2ms定时器更新外设状态
void Updata_Device_Status(DeviceStatus newStatus)
{
	curStatus.value =(newStatus.value & 0x3ff) | (curStatus.value & 0xfffff800);
}

void Updata_HW_Status(DeviceStatus newStatus)
{
	curStatus.value =(newStatus.value & 0x030) | (curStatus.value & 0xffffffcf);
}

//更新轴识别器状态
void Update_AxleDevice_Status(DeviceStatus status)
{
	curStatus.value =(status.value) | (curStatus.value & 0xff8fffff);
}

//更新车辆信息上送标志位
void Update_CarInfo_SendFlag(DeviceStatus status)
{
	curStatus.value =(status.value) | (curStatus.value & 0xff7fffff);
}


//算法线程更新AD故障状态
//chanel 1-4 AD通道编号,status:0-正常,1为故障
BOOL Updata_AD_Status(u8 chanel,u8 status)
{
	if(chanel > 3 || status > 1)
		return FALSE;
		
	switch(chanel)
	{
		case 0:curStatus.bitValue.ad1_err = status;
			break;
		case 1:curStatus.bitValue.ad2_err = status;
			break;
		case 2:curStatus.bitValue.ad3_err = status;
			break;
		case 3:curStatus.bitValue.ad4_err = status;
			break;
		default:return TRUE;
	}
	return TRUE;
}


//算法线程更新ad芯片状态
void UpDate_ADChip_Status(int status)
{
	ADChipStatus = status & 0x0f;
}

int Get_ADChip_ErrStatus(void)
{
	return ADChipStatus;
}

/****************************************
 *参	数：no可为1(前光幕)或2(后光幕)
 * 返回值：true触发，false释放
 * 说	明：获取光幕触发状态
 ****************************************/
BOOL Get_LC_Status(u8 no)
{
	if(no==1)
	{
		return curStatus.bitValue.LC1_status==1;
	}
		
	else 
	{
		return curStatus.bitValue.LC2_status==1;
	}
}

/****************************************
 * 参	数：no可为1(前光幕)或2(后光幕)
 * 返回值：true正常，false故障
 * 说	明：获取光幕出错状态
 ****************************************/
BOOL Get_LC_ErrStatus(u8 no)
{
	if(no==1)
	{
		return curStatus.bitValue.LC1_err==0;
	}
	else 
	{
		return curStatus.bitValue.LC2_err==0;
	}
}

/****************************************
 * 参	数：no可为1-4路通道
 * 返回值：true正常 false故障
 * 说	明：获取AD通道故障状态
 ***************************************/
BOOL Get_AD_ErrStatus(u8 no)
{
	if((no < 1) || (no > 4)) return FALSE;
	
	if(no == 1)
		return curStatus.bitValue.ad1_err==1;
	else if(no == 2)
		return curStatus.bitValue.ad2_err==1;
	else if(no == 3)
		return curStatus.bitValue.ad3_err==1;
	else if(no == 4)
		return curStatus.bitValue.ad4_err==1;
	else
		;

	return FALSE;
}

/****************************************
 * 参	数：no可为1(前轴识别器)或2(后轴识别器)
 * 返回值：true有压力 false无压力
 * 说	明：获取轴识别器触发状态
 ***************************************/
BOOL Get_Axle_Status(u8 no)
{
	//other
	return TRUE;
}
/****************************************
 * 参	数：无
 * 返回值：true正常 false故障
 * 说	明：获取胎型识别器出错状态
 ***************************************/
BOOL Get_Tire_ErrStatus(void)
{
	return curStatus.bitValue.tireSen_err == 0;
}

//获取胎型识别器的信号有无状态，true-有信号，false-无信号
BOOL Get_Device_TyreFlag(void)
{
	return curStatus.bitValue.tireSen_Signal;
}

//获取胎型识别器的单双轮状态，true-双轮，false-单轮
BOOL Get_Tire_Status(void)
{
	return curStatus.bitValue.single_double==1;
}


/****************************************
 * 参	数：no可为1(前地感)或2(后地感)
 * 返回值：true触发 false释放
 * 说	明：获取地感线圈触发状态 
 ****************************************/
BOOL Get_GC_Status(u8 no)
{
	if(no==1)
	{
		return curStatus.bitValue.GC1_status==1;
	}
	else 
	{
		return curStatus.bitValue.GC2_status==1;
	}
}

/****************************************
 * 参	数：no可为1(前地感)或2(后地感)
 * 返回值：true正常 false故障
 * 说	明：获取地感线圈出错状态 
 ****************************************/
BOOL Get_GC_ErrStatus(u8 no)
{
	if(no==1)
	{
		return curStatus.bitValue.GC1_err==0;
	}
	else 
	{
		return curStatus.bitValue.GC2_err==0;
	}
}


/****************************************
* 返回值：已按《称重控制端与SDK通讯协议》排列好的两字节数据
* 说	明：获取所有设备状态信息
****************************************/
u32 Get_Device_Status(void)
{
	return curStatus.value;
}



