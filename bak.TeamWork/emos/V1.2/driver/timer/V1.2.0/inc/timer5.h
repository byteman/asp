#ifndef _TIM5_INIT_H_	
#define _TIM5_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"

#define Mode_1US				0							//1微秒
#define Mode_10US				1							//10微秒
#define Mode_100US			2							//100微秒
#define Mode_500US			3							//500微秒

#define TIM5_CMDClearCnt			0				//清空计数器
#define TIM5_CMDTimOpen			1				//开启定时器启用中断响应
#define TIM5_CMDTimClose			2				//关闭定时器关闭中断响应
#define TIM5_CMDSetCntFlag		3				//设置计数开关
#define TIM5_CMDSetCaller		4				//设置回调函数

typedef void(*TIM5_fun)(void); 			//函数指针
	
/****************************************************************************
* 名	称：u8 TIM5_Config(TIM5_fun Cfun,u16 TimeUS,u8 mode)
* 功	能：定时器配置
* 入口参数：TIM5_fun Cfun		回调函数
						u16 TimeUS	需要定时的单位级别	微秒单位
						u8 mode			模式 1 10 100 500微秒
* 出口参数：u8	设置是否成功		1成功 0失败
* 说	明：无
****************************************************************************/
u8 TIM5_Config(TIM5_fun Cfun,u16 TimeUS,u8 mode);	 	//定时器初始化

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				无用
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备读操作
* 入口参数：HANDLE IDH		设备句柄
						u8* Buffer		存放数据地址					无用		
						u32 len				希望读取的数据个数		无用
						u32* Reallen	实际读取的数据个数		获得时长计时值
						u32 lParam		参数									无用
* 出口参数：s32						读取操作是否成功			无用
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备写操作
* 入口参数：HANDLE IDH		设备句柄							无用
						u8* Buffer		写数据源地址					无用
						u32 len				希望写入的数据个数		无用
						u32* Reallen	实际写入的数据个数		无用
						u32 lParam		参数									无用
* 出口参数：s32						写入操作是否成功			无用
* 说	明：
****************************************************************************/	

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备控制
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数							返回值
TIM7_CMDClearCnt					无用							无用
TIM7_CMDTimOpen						无用							无用
TIM7_CMDTimClose					无用							无用
TIM7_CMDSetCntFlag				1开0关						无用
TIM7_CMDSetCaller					函数地址					无用
****************************************************************************/	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
#include "driver.h"
#include "tim7_init.h"

//回调函数1
void TIM7_CallerTest1(void)
{
	int i = 0;
	i += 3;
	return;
}

//回调函数2
void TIM7_CallerTest2(void)
{
	int i = 0;
	i += 3;
	return;
}

	u32 cnt = 0;																	//获取计数值
	HVL_ERR_CODE err = HVL_NO_ERR;								//错误状态
	
	HANDLE TIM7_Dev = 0;													//设备句柄
	
	TIM7_Config(TIM7_CallerTest1,10,Mode_500US);	//500微秒中断一次	10次激活一个中断响应
	
	TIM7_Dev = DeviceOpen(CHAR_TIM7,0,&err);			//设备打开
	if(err != HVL_NO_ERR)
		while(1);
	
	if(DeviceIoctl(TIM7_Dev,TIM7_CMDSetCntFlag,1) == 0)		//设置计数开关
		while(1);
		
	if(DeviceIoctl(TIM7_Dev,TIM7_CMDTimOpen,0) == 0)			//开启定时器启用中断响应
		while(1);
	
	SysTimeDly(5000);																			//5S

	if(DeviceIoctl(TIM7_Dev,TIM7_CMDSetCaller,(u32)&TIM7_CallerTest2) == 0)			//切换回调函数
		while(1);
	
	SysTimeDly(5000);																			//5S
	
	if(DeviceIoctl(TIM7_Dev,TIM7_CMDTimClose,0) == 0)			//开启定时器启用中断响应
		while(1);
	
	if(CharDeviceRead(TIM7_Dev,0,0,&cnt,0) == 0)					//获取计数值
		while(1);
	
	if(DeviceIoctl(TIM7_Dev,TIM7_CMDClearCnt,0) == 0)			//复位计数器
		while(1);
*/
