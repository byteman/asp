#include "adjob.h"
#include "driver.h"
#include "String.h"

#include "ADInterface.h"
#include "spi_cs5532A.h"
#include "spi_cs5532B.h"

static const u8 ADJob_pDisc[] = "ADJob\r\n";					

static SYS_EVENT* ADTimeEvent = (SYS_EVENT*)0;							//采样时间达成信号量
static HANDLE TIM6_Dev = 0;												//设备句柄

//TIM6头文件内容搬移
typedef void(*TIM6_fun)(void); 											//函数指针
extern u8 TIM6_Config(TIM6_fun Cfun,u16 TimeUS,u8 mode);	 	//定时器初始化
#define TIM6_CMDTimOpen				1				//开启定时器启用中断响应
#define TIM6_CMDTimClose			2				//关闭定时器关闭中断响应
#define Mode_10US					1				//10微秒

static u8 u8ErrContAD1 = 0;							//保存AD错误次数
static u8 u8ErrContAD2 = 0;
static u8 u8ErrContAD3 = 0;
static u8 u8ErrContAD4 = 0;
static u8 u8StateAD	= 0;							//保存AD状态

static u8 u8ResetTimCntAD1 = 0;						//用于重新初始化AD通道的计时变量(16次--20ms)
static u8 u8ResetTimCntAD2 = 0;
static u8 u8ResetTimCntAD3 = 0;
static u8 u8ResetTimCntAD4 = 0;

#define	RE_INIT_AD_COUNT	0x07					//AD错误该次数后从新启动

/****************************************************************************
* 名	称：static s8 ADJOB_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8				打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 ADJOB_Open(u32 lParam)
{
	if(DeviceIoctl(TIM6_Dev,TIM6_CMDTimOpen,0) == 0)			//开启定时器启用中断响应
		return 1;												//失败	
	else
		return 0;												//成功
}

/****************************************************************************
* 名	称：static s8 ADJOB_Close(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8				打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 ADJOB_Close(u32 lParam)
{
	if(DeviceIoctl(TIM6_Dev,TIM6_CMDTimClose,0) == 0)			//关闭定时器启用中断响应
		return 1;				//失败
	return 0;					//成功
}

/****************************************************************************
* 名	称：s32 ADJOB_Read(u8* buffer,u32 len) 
* 功	能：读
* 入口参数：u8* buffer		接收缓冲区			保存AD值(大于等于16字节)
			u32 len			需要接收的长度		需要大于等于16
* 出口参数：s32				操作是否成功		1成功	-1，-2失败
* 说	明：阻塞应用层任务
****************************************************************************/
static s32 ADJOB_Read(u8* buffer,u32 len) 
{
	u8 err = 0;	
	SysSemPend(ADTimeEvent,0,&err);				//等待定时时间消息
 	if(err != SYS_NO_ERR)					
 		return -1;		
	
	if(len < 16)								//如果输入参数有误
		return -2;	

	//AD1
	if( 0 == u8ResetTimCntAD1 )					//判断AD1是否工作正常
	{
		if(CS5532A1_Flag())						//判断AD1是否转换好
		{				
			u8StateAD &= ~ERROR_AD1;
			u8ErrContAD1 = 0;
			*((long*)buffer) = CS5532Ax_GetValue();//获取AD值
		}
		else
		{//AD未准备好，错误次数加1，如果错误次数达到需要重新初始化的次数
			if( 0 == (++u8ErrContAD1&RE_INIT_AD_COUNT) )
			{
				u8ResetTimCntAD1 = 1;		
				fStepConfgAD_Ax(CS5532A1 , 1);	//重新初始化AD1的第一步--复位,进入命令模式
				if(u8ErrContAD1>10){
					u8ErrContAD1 = 0;
					u8StateAD |= ERROR_AD1;
				}
			}
		}
	}
	else 
	{//复位计数器加1
		u8ResetTimCntAD1++;
		if( 16 == u8ResetTimCntAD1 )			//20ms延时到
		{
			fStepConfgAD_Ax(CS5532A1 , 2);		//重新初始化AD1的第二步--配置
			u8ResetTimCntAD1 = 0;
		}	
	}
	//AD2
	if( 0 ==u8ResetTimCntAD2 )
	{
		if(CS5532A2_Flag())
		{
			u8StateAD &= ~ERROR_AD2;
			u8ErrContAD2 = 0;
			*((long*)(buffer+4)) = CS5532Ax_GetValue();
		}
		else
		{
			if( 0 == (++u8ErrContAD2&RE_INIT_AD_COUNT) )
			{
				u8ResetTimCntAD2 = 1;
				fStepConfgAD_Ax(CS5532A2 , 1);		//重新初始化AD2的第一步--复位
				if(u8ErrContAD2>10){
					u8ErrContAD2 = 0;
					u8StateAD |= ERROR_AD2;
				}
			}
		}
	}
	else
	{
		u8ResetTimCntAD2++;
		if( 16 == u8ResetTimCntAD2 )				//20ms延时到
		{
			fStepConfgAD_Ax(CS5532A2 , 2);			//重新初始化AD2的第二步--配置
			u8ResetTimCntAD2 = 0;
		}
	}

	//AD3
	if( 0 == u8ResetTimCntAD3 )
	{
		if(CS5532B1_Flag())
		{				
			u8StateAD &= ~ERROR_AD3;
			u8ErrContAD3 = 0;
			*((long*)(buffer+8)) = CS5532Bx_GetValue();
		}
		else
		{
			if( 0 == (++u8ErrContAD3&RE_INIT_AD_COUNT) )
			{
				u8ResetTimCntAD3 = 1;		
				fStepConfgAD_Bx(CS5532B1 , 1);	//重新初始化AD1的第一步--复位
				if(u8ErrContAD3>10){
					u8ErrContAD3 = 0;
					u8StateAD |= ERROR_AD3;
				}
			}
		}
	}
	else 
	{
		u8ResetTimCntAD3++;
		if( 16 == u8ResetTimCntAD3 )			//20ms延时到
		{
			fStepConfgAD_Bx(CS5532B1 , 2);		//重新初始化AD1的第二步--配置
			u8ResetTimCntAD3 = 0;
		}
	}
	//AD4
	if( 0 ==u8ResetTimCntAD4 )
	{
		if(CS5532B2_Flag())
		{
			u8StateAD &= ~ERROR_AD4;
			u8ErrContAD4 = 0;
			*((long*)(buffer+12)) = CS5532Bx_GetValue();
		}
		else
		{
			if( 0 == (++u8ErrContAD4&RE_INIT_AD_COUNT) )
			{
				u8ResetTimCntAD4 = 1;
				fStepConfgAD_Bx(CS5532B2 , 1);		//重新初始化AD2的第一步--复位
				if(u8ErrContAD4>10){
					u8ErrContAD4 = 0;
					u8StateAD |= ERROR_AD4;
				}
			}
		}
	}
	else
	{
		u8ResetTimCntAD4++;
		if( 16 == u8ResetTimCntAD4 )				//20ms延时到
		{
			fStepConfgAD_Bx(CS5532B2 , 2);			//重新初始化AD2的第二步--配置
			u8ResetTimCntAD4 = 0;
		}
	}
	return 16;						
}

/****************************************************************************
* 名	称：static s32 ADJOB_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：写
* 入口参数：u8* buffer		接收缓冲区			无用
			u32 len			希望接收的长度	无用
* 出口参数：s32				操作是否成功		0成功	
* 说	明：
****************************************************************************/
static s32 ADJOB_Write(u8* buffer,u32 len) 
{
	return 0;
}

/****************************************************************************
* 名	称：static s32 ADJOB_Ioctl(u32 cmd, u32 lParam)
* 功	能：控制
* 入口参数：u32 cmd			命令
			u32 lParam		参数
* 出口参数：s32				依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 ADJOB_Ioctl(u32 cmd, u32 lParam)
{	
	switch(cmd)
	{
		case AD_CMDCheck:
		{
			return u8StateAD;											//设备是否故障		32位的低4位表示，A高D低
		}
	}
	return 1;
}

/****************************************************************************
* 名	称：void Tim6TimeISR(void)
* 功	能：定时达成
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Tim6TimeISR(void)
{
	SysSemPost(ADTimeEvent);					//发送时间达成信号量
}

/****************************************************************************
* 名	称：u8 ADJOB_init(u8 PRIO_t)
* 功	能：AD采样任务初始化
* 入口参数：u8 PRIO_t							优先级
* 出口参数：u8		是否成功   1成功  0失败
* 说	明：无
****************************************************************************/
bool ADJOB_init(u8 PRIO_t)
{		
 	HVL_ERR_CODE err = HVL_NO_ERR;								//错误状态
	
	//注册用变量
	DEV_REG adjob = 		//设备注册信息表						不必static		
	{
		CHAR_AD,						//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		(u8*)ADJob_pDisc,				//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)ADJOB_Open,		//设备打开函数指针
		(u32*)ADJOB_Close, 	//设备关闭函数指针
		(u32*)ADJOB_Read,		//字符读函数
		(u32*)ADJOB_Write,	//字符写函数
		(u32*)ADJOB_Ioctl		//控制函数
	};																		
	
	CS5532A_Init();	   //CS5532引脚初始化
	CS5532B_Init();	   //CS5532引脚初始化										
	if(ADTimeEvent == (SYS_EVENT*)0) 
		 ADTimeEvent = SysSemCreate(0);   					//创建采样时间达成信号量
		
 	if(TIM6_Config(Tim6TimeISR,125,Mode_10US) == 0)			//10微秒计数一次	125次激活一个中断响应  1250us = 1.25ms
		return false;
	
 	TIM6_Dev = DeviceOpen(CHAR_TIM6,0,&err);			//设备打开	//获得句柄
	if(err != HVL_NO_ERR)
 		return false;
		
	if(DeviceInstall(&adjob) != HVL_NO_ERR)				//注册设备
		return false;	


	if(false == fStepConfgAD_Ax(CS5532A1+CS5532A2 , 1))//复位A1A2
		return false;
	if(false == fStepConfgAD_Bx(CS5532B1+CS5532B2 , 1))//复位B1B2
		return false;	
	SysTimeDly(2);									   //20ms延时
	if(false == fStepConfgAD_Ax(CS5532A1+CS5532A2 , 2))//配置A1A2
		return false;	
	if(false == fStepConfgAD_Bx(CS5532B1+CS5532B2 , 2))//配置B1B2
		return false;		
	else 
		return true;	

}

