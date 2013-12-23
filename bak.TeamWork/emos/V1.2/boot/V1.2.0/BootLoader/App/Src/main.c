#include "main.h"
#include "stm32f4xx.h"		//固件库
#include "nvic_init.h"		//nvic_init
#include "crc_init.h"			//crc
#include "printf_init.h"	//printf
#include "Flash_bootloader_init.h"	//BootLoader
#include "rtc_init.h"			//RTC_Config															//可以查看时间
#include "static_init.h"	//BootLoader参数
#include "Flash_init.h"		//FLASH_DataSpace_Erase
#include "ConnectCfg.h"		//通讯口配置
#include "usart_cfg.h"		//USART_STRUCT
#include "tim6_init.h"		//TIM6_Config															//给选择提供精确定时

#ifdef BootKey						//有引导按键
#include "BootKey_init.h"	//引导按键
#endif

#ifdef RTCTimeFlag
#include "RTC_Alarm.h"		//Alarm_A_Respond
#endif

#define USART_RX_BUFF_SIZEMAX 128
#define USART_TX_BUFF_SIZEMAX 128
static u8 USARTRxBuffer[USART_RX_BUFF_SIZEMAX] = {0};								//串口接收缓存区
static u8 USARTTxBuffer[USART_TX_BUFF_SIZEMAX] = {0};								//串口发送缓存区

static u8 NET_Bin_KEY[] = "EMOS_V001_001_001_001";				//项目KEY

//串口配置变量
static USART_STRUCT USART_Real = 
{
	BootLoaderUsart_BaudRate,	//波特率

	USARTRxBuffer,						//接收缓冲区
	USART_RX_BUFF_SIZEMAX,		//接收缓冲区大小

	USARTTxBuffer,						//发送缓冲区
	USART_TX_BUFF_SIZEMAX,		//发送缓冲区大小
};

/****************************************************************************
* 名	称：int main(void)
* 功	能：无
* 入口参数：无
* 出口参数：无
* 说	明：固件升级流程
1：接收固件
2：固件存储到备份区，更新标志
3：检测标志，是否升级
4：需要升级，检测固件备份区代码
5：拷贝固件备份区代码到固件应用区
6：检测固件应用区代码
7：擦除更新标志

接收文件时最好都用CRC32H_Chack8作文件效验并存储用
解析时好均用CRC32H_Chack8来作解析
****************************************************************************/
int main(void)
{
	u32 FileCRC = 0;				//应用区计算出来的文件校验值
	u32 FileCRCBak = 0;			//应用备份区计算出来的文件校验值
	u32 flag = 0;						//升级完成标志	0完成	1效验错误未完成	 2固件大小错误未完成	

	#ifndef BootKey					//没有引导按键
	#ifdef CmdKey						//有命令按键
	u8 cmdlen = 0;					//接收个数
	u8 cmdVal[2] = {0};			//接收字符
	#endif
	#endif

	nvic_init();														//中断向量表地址
	
//Power_init();														//电源开关
		
	CRC32_init();														//CRC32初始化
	
	#ifdef BootKey													//引导按键
	BootKey_init();													//引导按键引脚初始化
	#endif

	if(BootLoaderUsart_Config(&USART_Real) == 0)				//按配置初始化通讯端口
		return 0;	
	Set_PrintfPort(PrintfPort);													//打印输出口配置	
	
	if(TIM6_Config(0,10,Mode_100US) == 0)								//默认为毫秒中断
		DebugPf(0,"定时器6设置失败\r\n");	
	TIM6_CntFlagOnOff(1);																//开启计数功能
	TIM6_ResetCount();																	//计时值复位
	TIM6_TimeISR_Open();																//开启定时器及中断
	
	//是否启用实时时钟	(如果晶振坏了则BootLoader也无法运行)
	#ifdef RTCTimeFlag
  if(RTC_Config(2012,10,4,4,15,45,30))								//实时时钟初始化		年月日周时分秒
		RTC_AlarmA(99,0,99,99,99,1,Alarm_A_Respond);			//配置闹钟A		每秒报警A
	#endif
	
	//是否启用自动波特率识别
	#ifdef AutoBaudRateCheckFlag
	USART_Real.BaudRate = AutoBaudRateCheck(' ');				//自动检测波特率
	if(USART_Real.BaudRate == 0)												//如果没有检测到波特率
	{
		USART_Real.BaudRate = BootLoaderUsart_BaudRate;		//设置为默认波特率
		BootLoaderUsart_Init(BootLoaderUsart_BaudRate);		//以默认波特率设置串口
	}
	#endif
	
	DebugPfEx(0,"BootLoader Start, BaudRate %d\r\n",USART_Real.BaudRate);
	
	BlockDataCheck();																		//数据块检查
	
	//是否启用了引导或命令按键
	#ifdef BootKey			//有引导按键
	if(GPIO_ReadInputDataBit(BootKeyPort,BootKeyPin) == BootKeyDownVal)
		BOOTLoader(USART_Real.BaudRate);									//进入引导功能		引导按键按下		//没有引导按键则跳过
	#else
	#ifdef CmdKey				//有命令按键
	TimDelayMS(300);																		//接收等待0.3秒
	cmdlen = BootLoaderUsart_GetString(cmdVal,2);				//从缓冲区获取字符
	if(cmdlen >= 1)																			//收到数据
	{
		if(cmdVal[0] == ' ')															//判断字符是否正确
		{
			BootLoaderUsart_RX_Clear();											//清空接收缓冲区
			BOOTLoader(USART_Real.BaudRate);								//进入引导功能		无引导按键
		}
	}
	#endif
	#endif
		
	//是否有新固件
	if(ReadAppUPFlagAddress() == UPBOOTFlag)	
	{
		if(ReadAppUpSrcAddress() == PCBOOT)					   		//固件来自哪里
		{
			//Set_PrintfPort(1);														//打印端口是否需要改变
			DebugPf(0,"有PC发来的新固件\r\n");
		}
		else if(ReadAppUpSrcAddress() == NETBOOT)	
		{
			//Set_PrintfPort(1);														//打印端口是否需要改变
			DebugPf(0,"有网络发来的新固件\r\n");
		}
		
		if(ReadAppSizeAddress() <= AppSpaceSize)					//固件不能大于固件存放空间
		{	
			DebugPf(0,"效验固件代码\r\n");
			
			//计算文件效验值
			if(ReadAppUpSrcAddress() == PCBOOT)											//PC固件		效验方法		CRC32H_Chack8
			{	
				FileCRCBak = CRC32H_Chack8(1,(u8*)((volatile u8*)AppStartAddressBak),ReadAppSizeAddress());		//应用备份区计算获得的文件校验值		  //STM32硬件效验
			}
			else if(ReadAppUpSrcAddress() == NETBOOT)								//NET固件		效验方法		可以不是CRC32H_Chack8 但必须给接收时用的CRC32H_Chack8一致
			{
				FileCRCBak = CRC16(1,(u8*)((volatile u8*)AppStartAddressBak),ReadAppSizeAddress());		//应用备份区计算获得的文件校验值
				FileCRCBak = CRC16(0,NET_Bin_KEY,sizeof(NET_Bin_KEY));	
			}
			DebugPfEx(1,"长度校验值 %X\r\n",FileCRCBak);
				   
			//包数和文件大小校验值
			if(FileCRCBak == ReadAppCRCAddress())  									//FileCRCBak计算	ReadAppCRCAddress读取	
			{	
				DebugPf(0,"效验成功，更新固件程序........\r\n");				//每个包效验通过，固件代码完整开始更新程序

				CopyErrJump:		//当出现拷贝出错时跳转至此从新进行拷贝
				if(FLASH_DataSpace_Erase(AppStartAddress,AppSpaceSize) == 0)													//擦除运行固件区
				{
					DebugPf(0,"应用程序空间擦除失败!\r\n");																							//应用程序空间擦除失败
					return 0;
				}
				
				Flash_DataWrite(AppStartAddress,ReadAppSizeAddress(),(u8*)AppStartAddressBak);				//拷贝数据到运行区	//按大小来复制固件到运行区
				DebugPf(0,"固件已更新\r\n开始对应用区固件代码进行效验...\r\n");

				//检测拷贝过来的应用区代码CRC
				
				if(ReadAppUpSrcAddress() == PCBOOT)											//PC固件		效验方法		CRC32H_Chack8
				{	
					FileCRC = CRC32H_Chack8(1,(u8*)((volatile u8*)AppStartAddress),ReadAppSizeAddress());		//应用备份区计算获得的文件校验值		  //STM32硬件效验
				}
				else if(ReadAppUpSrcAddress() == NETBOOT)								//NET固件		效验方法		可以不是CRC32H_Chack8 但必须给接收时用的CRC32H_Chack8一致
				{
					FileCRC = CRC16(1,(u8*)((volatile u8*)AppStartAddress),ReadAppSizeAddress());		//应用备份区计算获得的文件校验值
					FileCRC = CRC16(0,NET_Bin_KEY,sizeof(NET_Bin_KEY));	
				}
				if(FileCRC == ReadAppCRCAddress())
				{
					//应用区代码和备份区代码一致
					flag = Succeed;			 																												//升级完成	变量置0
					if(FLASH_DataSpace_Erase(AppFlagAddress,AppFlagSpaceSize) == 0)							//写升级完成标志
						DebugPf(0,"应用程序备份标志空间擦除失败(升级完成标志)!\r\n");							//应用程序标志空间擦除失败
					
					Flash_DataWrite(AppFlagAddress,4,(u8*)&flag);																//更新结果，执行成功

					DebugPf(0,"固件更新成功，跳转执行应用程序......\r\n");
					
					BootLoaderOver();				//结束引导
					Jump_IAP(AppStartAddress);	 						//程序跳转
				}
				else											//应用区效验失败	备份区代码拷贝到应用区时出错
				{
					if(CopyEver == 0)				//不一直拷贝
					{
						flag = CopyErr;		  																								//升级未完成	变量置3
	          if(FLASH_DataSpace_Erase(AppFlagAddress,AppFlagSpaceSize) == 0)			//写升级未完成标志
						{
							DebugPf(0,"应用程序备份标志空间擦除失败(拷贝错误标志)!\r\n");			//应用程序标志空间擦除失败
							return 0;
						}
						Flash_DataWrite(AppFlagAddress,4,(u8*)&flag);												//更新结果，拷贝错误
						DebugPf(0,"拷贝数据出错，应用区代码和备份应用区代码不一致\r\n");
					}
					else										//一直进行拷贝动作
						goto CopyErrJump;			//跳转到拷贝错误跳转处
				}
			}
			else 				   							//效验失败
			{
        flag = CRCErr;		   			//升级未完成	变量置1
        if(FLASH_DataSpace_Erase(AppFlagAddress,AppFlagSpaceSize) == 0)					//写升级未完成标志
				{
					DebugPf(0,"应用程序备份标志空间擦除失败(CRC错误标志)!\r\n");					//应用程序标志空间擦除失败
					return 0;
				}
				Flash_DataWrite(AppFlagAddress,4,(u8*)&flag);														//更新结果，CRC错误
				DebugPf(0,"固件包CRC检测不通过，不更新\r\n");
			}
		}
		else 					   							//固件大小不通过
		{
      flag = SizeErr;			   			//固件大小不通过，变量置2
      if(FLASH_DataSpace_Erase(AppFlagAddress,AppFlagSpaceSize) == 0)						//写固件大小不通过标志
			{
				DebugPf(0,"应用程序备份标志空间擦除失败(固件大小错误标志)!\r\n");				//应用程序标志空间擦除失败
				return 0;
			}
			Flash_DataWrite(AppFlagAddress,4,(u8*)&flag);															//更新结果，固件包太小
			DebugPf(0,"固件包过小，不更新\r\n");
		}

		DebugPf(0,"执行原程序\r\n");
		
		BootLoaderOver();												//结束引导
		Jump_IAP(AppStartAddress);	 						//程序跳转
		return 0;
	}
	
	DebugPf(0,"跳转执行应用程序......\r\n");
	
	BootLoaderOver();													//结束引导
	Jump_IAP(AppStartAddress);																//程序跳转
	return 0;																	//任务启动失败
}

