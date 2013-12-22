#include "beep_thread.h"
#include "beep_init.h"
#include "driver.h"
  
/****************************************************************************/
//为了不引用 Beep_init.h
extern void Beep(u32 count,u16 time);			//蜂鸣器鸣叫
extern void SetBeepFlag(u8 on);						//设置蜂鸣器开关
extern u8 LookBeepFlag(void);							//查看蜂鸣器开关
extern void Beep_Config(void);						//蜂鸣器初始化

/****************************************************************************/

static INT8U BeepJob_PRIO = 0;			//任务优先级
static SYS_EVENT* BeepJobEvent = (SYS_EVENT*)0;										//鸣叫消息队列
static u8 BeepJob_pDisc[] = "LuoHuaiXiang_BeepJob\r\n";						//设备描述字符					必须static

//任务堆栈大小
#define BeepJobStackSize 128	  														//BEEPJOB任务堆栈大小

//任务堆栈
__align(8) static SYS_STK 		BeepJobStack[BeepJobStackSize];			//BEEPJOB任务堆栈

/****************************************************************************
* 名	称：void BeepJob_Default(void *nouse)
* 功	能：默认BeepJob
* 入口参数：无
* 出口参数：无
* 说	明：无			
****************************************************************************/
void BeepJob_Default(void *nouse)
{
	u8 err = 0;	
	void* pMsg;														//消息带来的值地址
	
	if(LookBeepFlag() == 0)								//如果蜂鸣器处于关闭状态
		SetBeepFlag(1);											//打开蜂鸣器

	while(1)
	{
		pMsg = SysQPend(BeepJobEvent, 0, &err);	//等待消息
		if(err != SYS_NO_ERR)	//正常		接收到的消息队列状态
			continue;
		else
			Beep(((BEEP_STRUCTURE*)pMsg)->count,((BEEP_STRUCTURE*)pMsg)->time);										//鸣叫
	}
}

/****************************************************************************
* 名	称：static s8 BEEPJOB_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 BEEPJOB_Open(u32 lParam)
{
	INT8U TaskStatus = 0;	//创建任务SysTaskCreate返回值    	SYS_NO_ERR(0x00成功)	SYS_PRIO_EXIST(0x28优先级相同) 

	TaskStatus |= SysTaskCreate(BeepJob_Default,						//任务函数
										(void *)0,														//任务参数
										&BeepJobStack[BeepJobStackSize - 1],	//任务堆栈
										BeepJob_PRIO);												//任务优先级
 	if(TaskStatus != SYS_NO_ERR)
		return 1;																							//打开失败
	else
		return 0;																							//打开成功
}

/****************************************************************************
* 名	称：static s8 BEEPJOB_Close(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 BEEPJOB_Close(u32 lParam)
{
	if(SysTaskDel(BeepJob_PRIO) == SYS_ERR_NONE)				//蜂鸣器任务
		return 0;
	return 1;
}

/****************************************************************************
* 名	称：static s32 BEEPJOB_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 BEEPJOB_Read(u8* buffer,u32 len) 
{
	return 0;						
}

/****************************************************************************
* 名	称：static s32 BEEPJOB_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：立即发音
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						BEEP_STRUCTURE
* 出口参数：s32						写入操作是否成功	1成功 0失败			(失败为信号量出错)
* 说	明：
typedef struct
{
	u32 count;				//发声次数
	u16 time;					//每次发声时间，单位毫秒
}BEEP_STRUCTURE;
****************************************************************************/
static s32 BEEPJOB_Write(u8* buffer,u32 len) 
{
	return 0;
}

/****************************************************************************
* 名	称：static s32 BEEPJOB_Ioctl(u32 cmd, u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 BEEPJOB_Ioctl(u32 cmd, u32 lParam)
{
	switch(cmd)
	{
		case BEEPCMD_SETONOFF:
		{
			if(lParam > 0)
				SetBeepFlag(1);
			else
				SetBeepFlag(0);
		}
		case BEEPCMD_GETONOFF:
			return (u32)LookBeepFlag();
		case BEEPCMD_ON:
		{
				if(BeepJobEvent != NULL) 										//如果创建了消息队列信号量
				{
					SysQPost(BeepJobEvent, (void*)lParam);	//发送消息队列
					return 0;
				}
				break;
		}
	}
	return 1;
}

/****************************************************************************
* 名	称：u8 BEEPJOB_Init(u8 PRIO_t,SYS_EVENT* JOBEVENT)
* 功	能：BeepJOB初始化
* 入口参数：u8 PRIO_t								任务优先级
						SYS_EVENT* JOBEVENT			鸣叫消息队列
* 出口参数：u8 					是否成功	1成功	0失败
* 说	明：无
****************************************************************************/
u8 BEEPJOB_Init(u8 PRIO_t,SYS_EVENT* JOBEVENT)
{
	//注册用变量
	DEV_REG beepjob = 		//设备注册信息表						不必static		
	{
		CHAR_BEEPJOB,				//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		BeepJob_pDisc,			//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)BEEPJOB_Open,		//设备打开函数指针
		(u32*)BEEPJOB_Close, 	//设备关闭函数指针
		(u32*)BEEPJOB_Read,		//字符读函数
		(u32*)BEEPJOB_Write,	//字符写函数
		(u32*)BEEPJOB_Ioctl		//控制函数
	};		
	
	Beep_Config();																			//蜂鸣器初始化
	BeepJob_PRIO = PRIO_t;															//优先级保存
	BeepJobEvent = JOBEVENT;														//消息队列保存
	if(DeviceInstall(&beepjob) != HVL_NO_ERR)						//注册设备
		return 0;	
	
	return 1;
}
