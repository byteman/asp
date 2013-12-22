#ifndef _DOG_INIT_H_
#define _DOG_INIT_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"				

#define DOGCMD_FEED			0		//喂狗
	
void FeedDog(void); //直接调用喂狗函数
/****************************************************************************
* 名	称：u8 Dog_Init(void)
* 功	能：看门狗初始化
* 入口参数：无
* 出口参数：u8	初始化是否成功	1成功 0失败
* 说	明：无
****************************************************************************/
u8 Dog_Init(void);

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				任务优先级
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：创建蜂鸣器任务
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：删除蜂鸣器任务
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
DOGCMD_FEED								无用							无用
****************************************************************************/	

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	HANDLE DOG_HANDLE;												//设备句柄
	
	USART1_Config(&usart1cfg);								//串口初始化
	Set_PrintfPort(1);												//设置打印串口
	printf("测试开始\r\n");
	SysTimeDly(10);
	
	Dog_Init();		//看门狗初始化		初始化后1.6秒内需要立即喂狗
	
	DOG_HANDLE	= DeviceOpen(CHAR_DOG,0,&err);	//打开看门狗设备
	if(err != HVL_NO_ERR)
		while(1);
	DeviceIoctl(DOG_HANDLE,DOGCMD_FEED,0);			//喂狗
	
	while(1)
	{
		printf("秒计数 %d \r\n",cnt++);
		DeviceIoctl(DOG_HANDLE,DOGCMD_FEED,0);		//喂狗
		SysTimeDly(140);			//1.6S  必须喂狗，否则复位
	}
*/
