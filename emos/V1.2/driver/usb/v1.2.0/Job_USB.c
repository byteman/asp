
#include "Job_USB.h"
#include "driver.h"


#include "usb_conf.h"
#include "usb_core.h"
#include "usbh_core.h"
#include "usbh_msc_core.h"

typedef void (*fFeedDog)(void);									//保存应用层注册的喂狗服务函数
fFeedDog pfFeedDog = NULL;
/** @defgroup USBH_USR_MAIN_Private_Variables
* @{
*/
__ALIGN_BEGIN USB_OTG_CORE_HANDLE      USB_OTG_Core __ALIGN_END;
__ALIGN_BEGIN USBH_HOST                USB_Host __ALIGN_END;

//源于usbh_usr.h
extern  USBH_Usr_cb_TypeDef USR_cb;
extern 	s32	fUSB1_StateRead(void);
extern	s32	fUSB1_CapacityReturn(void);


//定义USB任务堆栈大小
#define 	JOB_USB1_STACK_SIZE			256

//任务堆栈
__align(8) static SYS_STK 		Stack_JobUSB[JOB_USB1_STACK_SIZE];		//JOB USB1任务堆栈
static u8 JobUSB1_PRIO = 0;												//任务优先级

static u8 sJOB_USB1_pDisc[] = "USB1\r\n";								//设备描述字符		必须static

/****************************************************************************
* 名	称：USB1Thread
* 功	能：USB1线程
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
static void USB1Thread(void *nouse)
{
	for(;;)
	{
		/* Host Task handler */
		USBH_Process(&USB_OTG_Core, &USB_Host);
		SysTimeDly(10);
	}
}
/****************************************************************************
* 名	称：static s8 USB1JOB_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 USB1JOB_Open(u32 lParam)
{
	INT8U TaskStatus = 0;	//创建任务SysTaskCreate返回值    	SYS_NO_ERR(0x00成功)	SYS_PRIO_EXIST(0x28优先级相同) 

	TaskStatus |= SysTaskCreate(USB1Thread,											//任务函数
										(void *)0,									//任务参数
										&Stack_JobUSB[JOB_USB1_STACK_SIZE - 1],		//任务堆栈
										JobUSB1_PRIO);								//任务优先级
 	if(TaskStatus != SYS_NO_ERR)
		return 1;																	//打开失败
	else
		return 0;																	//打开成功
}
/****************************************************************************
* 名	称：static s8 USB1JOB_Close(u32 lParam)
* 功	能：删除USB1任务
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 USB1JOB_Close(u32 lParam)
{
	if( SYS_NO_ERR != SysTaskDel(JobUSB1_PRIO) )
	{return 1;}
	else
	{return 0;}
		
}
extern char sProductString[20];
/****************************************************************************
* 名	称：static s32 USB1JOB_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：阻塞应用层任务
****************************************************************************/
static s32 USB1JOB_Read(u32 offset , u8* buffer,u32 len,u32* Reallen,u32 lParam) 
{
	u8 i;
	*Reallen = len;
	if(len>20)
	{
		len = 20;
		*Reallen = 20;
	}
	for( i=0 ; i<len ; i++ )
	{
		(buffer[i]) = sProductString[i];
	}
	return 1;						
}
/****************************************************************************
* 名	称：static s32 USB1JOB_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：写
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：
****************************************************************************/
static s32 USB1JOB_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
{
	return 0;
}
/****************************************************************************
* 名	称：static s32 USB1JOB_Ioctl(u32 cmd, u32 lParam)
* 功	能：控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 USB1JOB_Ioctl(u32 cmd, u32 lParam)
{
	switch(cmd){
		case 	CMD_USB1_READ_STATE:{
				return fUSB1_StateRead();}
		case	CMD_USB1_READ_CAPACITY:{
				return	fUSB1_CapacityReturn();}
		case	CMD_USB1_REGISTER_FEED_DOG:{
				pfFeedDog = (fFeedDog)lParam;
		}
	}
	return 1;
}
/****************************************************************************
* 名	称：u8 USB1Job_Init(u8 PRIO_t)
* 功	能：USB1任务初始化
* 入口参数：u8 PRIO_t							优先级
* 出口参数：u8		是否成功   1成功  0失败
* 说	明：无
****************************************************************************/
u8 USB1Job_Init(u8 PRIO_t)
{	
	//注册用变量
	DEV_REG USBjob = 		  	//设备注册信息表		不必static		
	{
		BLOCK_USB1,				//设备ID号
		0,  					//是否共享			0:不可共享使用, 1:可共享使用
		0, 						//对于共享设备最大打开次数
		1,						//最大读次数
		1,						//最大写次数
		sJOB_USB1_pDisc,		//驱动描述			自定义
		20120001,				//驱动版本			自定义
		(u32*)USB1JOB_Open,		//设备打开函数指针
		(u32*)USB1JOB_Close, 	//设备关闭函数指针
		(u32*)USB1JOB_Read,		//字符读函数
		(u32*)USB1JOB_Write,	//字符写函数
		(u32*)USB1JOB_Ioctl		//控制函数
	};	
	
	JobUSB1_PRIO = PRIO_t;		//任务优先级保存
	
	//USB初始化
	USBH_Init(&USB_OTG_Core, 
						USB_OTG_FS_CORE_ID,
						&USB_Host,
						&USBH_MSC_cb, 
						&USR_cb);	

	if(DeviceInstall(&USBjob) != HVL_NO_ERR)	//注册设备
		return 0;	
	
	return 1;
}





