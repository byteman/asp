#ifndef _JOB_USB_INIT_H_	
#define _JOB_USB_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"
	
	
#define	NO_U_DISC							0x00//(无U盘插入)
#define	LINK_U_DISC							0x01//(U盘已插入)
#define	GET_READY_U_DISC					0x02//(可以进行文件操作)
#define	EXTRACT_U_DISC						0x80//(U盘已拔出)
	
#define CMD_USB1_READ_STATE			0				//DeviceIoctl读取USB1状态
#define	CMD_USB1_READ_CAPACITY		1				//DeviceIOctl读取U盘容量，单位M
#define	CMD_USB1_REGISTER_FEED_DOG	2				//注册喂狗服务函数
	
/****************************************************************************
* 名	称：u8 JOB_USB1_init(u8 PRIO_t)
* 功	能：USB任务初始化
* 入口参数：u8 PRIO_t							优先级
* 出口参数：u8		是否成功   1成功  0失败
* 说	明：无
****************************************************************************/
u8 USB1Job_Init(u8 PRIO_t);	 	//USB任务初始化

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄并创建USB1任务
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				无用
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：删除USB1任务
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备读操作，读USB生产厂商名
* 入口参数：HANDLE IDH		设备句柄
						u8* Buffer		存放数据地址							
						u32 len				希望读取的数据个数		
						u32* Reallen	实际读取的数据个数		
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
* 功	能：设备控制，读USB状态，读U盘容量
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数							返回值
CMD_USB1_READ_STATE				无用							0x00(无U盘插入)
																						0x01(U盘已插入)
																						0x02(可以进行文件操作)
																						0x80(U盘已拔出)
CMD_USB1_READ_CAPACITY		无用							U盘容量，单位M

****************************************************************************/	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif



