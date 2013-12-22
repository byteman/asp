#ifndef _DEVICE_INFO_H
#define _DEVICE_INFO_H

#include "includes.h"

/*****
设备命名规定：
光幕1：上秤方向第一个光幕，该光幕一般用于测速，可选。
光幕2：上秤方向第二个光幕，该光幕用于车辆分离。
地感线圈1：上秤台方向的地感线圈
地感线圈2：下秤台方向的地感线圈

BIT0: 
0--正常
1--光幕1故障
BIT 1: 
0--光幕1未挡住
1--光幕1挡住
BIT 2: 
0--正常
1--光幕2故障
BIT 3: 
0—光幕2未挡住
1—光幕2挡住
BIT 4：
0--正常
1--地感线圈1故障
BIT 5：
0--地感线圈1处于释放状态
1--地感线圈1处于触发状态
BIT 6：
0--正常
1--地感线圈2故障
BIT 7：
0--地感线圈2处于释放状态
1--地感线圈2处于触发状态
BIT8: 
0—	正常
1—	轮胎识别器故障 
BIT9: 
0-	单轮
1-	双轮
BIT10-BIT15：为其他外设保留

BIT16：第1路AD故障
0-	正常
1-	故障
BIT 17：第2路AD故障
0：正常
1：故障
BIT 18：第3路AD故障
0：正常
1：故障
BIT 19：第4路AD故障
0：正常
1：故障
BIT 20-BIT31 ：为将来的AD故障保留
**********/
typedef struct {
	u8 ad1_err:1;		//第1路AD故障
	u8 ad2_err:1;
	u8 ad3_err:1;
	u8 ad4_err:1;
}ADStatus;
//其中1、2为上称方向的顺序 
typedef union {
	struct {
		u8 LC1_err:1;		//光幕1  0-normal status,1-err status
		u8 LC1_status:1;	//	 0-release,1-trigger
		u8 LC2_err:1;		//光幕2 分车光幕故障
		u8 LC2_status:1;	//分车光幕状态	
		u8 GC1_err:1;		//前地感		
		u8 GC1_status:1;	
		u8 GC2_err:1;		//后地感
		u8 GC2_status:1;
		
		u8 tireSen_err:1;		//轮胎识别器故障
		u8 single_double:1;	//单双轮   0-signal,1-double
		u8 tireSen_Signal:1;  //轮胎识别器信号 0 无 1 有
		u8 ud1:5;			//备用 6位
		
		u8 ad1_err:1;		//第1路AD故障
		u8 ad2_err:1;
		u8 ad3_err:1;
		u8 ad4_err:1;
		u8 axle11_valid:1;		//前轴第一路有效标志
		u8 axle12_valid:1;
		u8 axle2_valid:1;			//后轴
		u8 carinfo_send_flag:1;		//车辆发送标志
		u8 ud2:8;			//备用 4位		 
	}bitValue;

	u32 value;
}DeviceStatus;	//外设状态信息 



//设置继电器的状态 PE0-3 低电平时输出口为高电平
u8 Set_Relay_Status(u8 no,BOOL status);

BOOL Get_LC_Status(u8 no);
// 参	数：no可为1(前光幕)或2(后光幕)
// 返回值：true触发，false释放
// 说	明：获取光幕触发状态

BOOL Get_LC_ErrStatus(u8 no);
// 参	数：no可为1(前光幕)或2(后光幕)
// 返回值：true正常，false故障
// 说	明：获取光幕出错状态

BOOL Get_Tire_Status(void);
// 参	数：无
// 返回值：true单轮 false双轮
// 说	明：获取胎识别器单双状态

BOOL Get_Tire_ErrStatus(void);
// 参	数：无
// 返回值：true正常 false故障
// 说	明：获取胎识别器出错状态

BOOL Get_GC_Status(u8 no);
// 参	数：no可为1(前地感)或2(后地感)
// 返回值：true触发 false释放
// 说	明：获取地感线圈触发状态 

BOOL Get_GC_ErrStatus(u8 no);
// 参	数：no可为1(前地感)或2(后地感)
// 返回值：true正常 false故障
// 说	明：获取地感线圈出错状态 


u32 Get_Device_Status(void);
// 返回值：已按《称重控制端与SDK通讯协议》排列好的两字节数据
// 说	明：获取所有设备状态信息

void Updata_Device_Status(DeviceStatus newStatus);
// 说	明：更新所有设备状态信息到静态变量，已按《称重控制端与SDK通讯协议》排列好

//更新红外对射信号，借用前地感
void Updata_HW_Status(DeviceStatus newStatus);

//更新轴识别器状态, 调试使用
void Update_AxleDevice_Status(DeviceStatus status);
//更新车辆发送标志,上位机调试使用
void Update_CarInfo_SendFlag(DeviceStatus status);


BOOL Updata_AD_Status(u8 chanel,u8 status);
BOOL Get_AD_ErrStatus(u8 no);

//算法线程更新AD故障状态
//chanel 1-4 AD通道编号,status:0-正常,1为故障

void UpDate_ADChip_Status(int status);
int Get_ADChip_ErrStatus(void);


#endif		//_DEVICE_INFO_H
