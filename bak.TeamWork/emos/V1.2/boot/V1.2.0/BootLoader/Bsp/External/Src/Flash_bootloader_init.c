#include "Flash_bootloader_init.h"
#include "ConnectCfg.h"			//通讯口	//ComPort项
#include "printf_init.h"		//DebugPf
#include "static_init.h"		//CPUID
#include "ymodem_init.h"		//ymodem协议
#include "rtc_init.h"				//时间显示
#include "crc_init.h"				//CRC32H_Chack8

/***********************
bin			 		2进制文件
hex					16进制文件
Flash Loader Demo	烧hex文件

STM32矢量表
0x00		Initial Main SP
0x04		Reset
0x08		NMI
0x0C		Hard Fault
0x10		Memory Manage
0x14		Bus Fault
0x18		Usage Fault
0x1C-0x28	Reserved
0x2C		SVCall
0x30		Debug Monitor
0x34		Reserved
0x38		PendSV
0x3C		Svstick
0x40		IRQ0
..			More IRQs

IAP步骤：
BootLoader端	设置跳转地址										ApplicationAddress 0x08XX XXXX
Application端	*设置起始地址和程序大小								0x08XX XXXX 0xXXXX
				*设置中断向量表偏移地址和起始地址一致				NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0xXXXXX);
				设置Linker中R/O Base和起始地址一致					0x08XX XXXX
				修改*.sct文件中地址和起始地址一致，程序大小一致		0x08XX XXXX
				生成Bin 二进制文件
				查看*.map	RESET地址是否等于						0x08XX XXXX  
***********************/

//引导菜单显示收发数据用串口
#define BootLoader_SendByte  			ComSendByte							//BootLoader菜单单字符发送
#define BootLoader_GetByte_Wait  	ComGetByte_Wait					//BootLoader菜单接收串口 等待
#define BootLoader_SendString 		ComSendString						//BootLoader菜单字符串发送
#define BootLoader_RX_Clear 			BootLoaderUsart_RX_Clear						//BootLoader菜单接收清空
#define BootSendFinishWait()			ComSendFinishWait()			//等待发送完成

/****************************************************************************
* 名	称：void ShowMenu(const u32 BaudRate)
* 功	能：BootLoader菜单显示
* 入口参数：const u32 BaudRate		波特率
* 出口参数：无
* 说	明：程序开始地址为：0x00000000
****************************************************************************/
void ShowMenu(const u32 BaudRate)
{
	BootLoader_SendByte(0x0C);
	BootLoader_SendByte(0x0C);		//清屏 2次更彻底

	DebugPf(0,"*****************************************************\r\n");
	DebugPf(0,"                STM32 Bootloader Menu\r\n");
	DebugPfEx(0,"           (%s)\r\n",BootVersion);
	DebugPf(0,"\r\n");
  DebugPf(0,"                Build Date : ");
	DebugPf(0,__DATE__);		  //显示编译日期
	DebugPf(0,"\r\n");
	DebugPf(0,"                Build Time : ");
	DebugPf(0,__TIME__);		  //显示编译时间
	DebugPf(0,"\r\n");
	DebugPf(0,"\r\n");
	DebugPf(0,"        ( 1 ) : Download Application Program From Ymodem (256K)\r\n");							//终端获取应用程序固件文件
	DebugPf(0,"        ( 2 ) : Download GB Font From Ymodem (256K)\r\n");													//终端获取GB字库文件
	DebugPf(0,"        ( 3 ) : Download ASCII Font From Ymodem (16K)\r\n");												//终端获取ASCII字库文件
	DebugPf(0,"        ( 4 ) : Download Auxiliary Application Program From Ymodem (64K)\r\n");		//终端获取辅助应用程序文件
	DebugPf(0,"        ( 5 ) : Block Data Check\r\n");															//检查块中的数据
	DebugPf(0,"        ( 6 ) : CPU ID\r\n");																				//CPU ID
	DebugPf(0,"        ( 7 ) : Show Time\r\n");																			//显示当前时间
	DebugPf(0,"        ( 8 ) : Reset System\r\n");																	//重启
	DebugPf(0,"        ( E ) : Exit\r\n");																					//退出
	DebugPf(0,"        ( D ) : Auxiliary\r\n");																			//辅助应用程序
	DebugPf(0,"\r\n");
	DebugPfEx(0,"-----------  %d(S) Wait....  ",(u8)MenuWaitTime);									//提示等待时间为秒
	DebugPfEx(0,"BaudRate：%d----------\r\n",BaudRate);															//提示等待时间为秒
	DebugPf(0,"*****************************************************\r\n");
	DebugPf(0,"Input Number:");
}

/****************************************************************************
* 名	称：u8 Key_Input(const u8 timeS)
* 功	能：按键输入
* 入口参数onst 簎8 timeS	n*5秒
* 出口参数：u8	1	返回刷新菜单
				0	退出不用回显菜单
* 说	明：1	运行程序
			2	下载程序
			3	退出BOOTLOADER
			4	YMODEM接收文件
		其它	错误按键
****************************************************************************/
u8 Key_Input(const u8 timeS)
{
	u8 num = 0;								//输入番号
	u8 flag = 0;							//状态标志	
	u8 CPUID[12] = {0};				//CPUID
	u32 TimeCnt = 0;					//秒延迟计数器

	u32 AuxiliaryAppCRCOne = 0;					//辅助程序读取效验码
	u32 AuxiliaryAppCRCOneBak = 0;			//辅助程序计算效验码
	u32 AuxiliaryAppCRCDataSize = 0;		//辅助程序效验数据大小

	while(1)
	{
		flag = BootLoader_GetByte_Wait(&num,11200000);		//接收数据，延迟1S (RTC秒计时计算出来)
		if(flag == 0)		  								//未接收到数据	   	//2秒未按键跳转
		{
			TimeCnt++;											//秒延时增加
			
			if(TimeCnt >= timeS)						//时间达成
			{
				DebugPfEx(0,"\r\n%d秒无操作，自动退出BootLoader......\r\n",timeS);
				return 0;
			}
		}
		else			 											 //接收到数据		//2秒有按键
		{
			TimeCnt = 0;									//重新计时
	
			DebugPfEx(0,"%c\r\n",num);		//回显键入的字符命令
			switch(num)
			{
				case '1':
				case '2':
				case '3':
				case '4':
				{
					switch(num)
					{
						case '1':
						{
							DebugPfEx(0,"Download Application Program From Ymodem.....'B' or 'b' = EXIT!  %d(S) Wait...\r\n",(u32)ShowGetFileWait);		//接收完成后返回菜单
							flag = Ymodem_START(1);	//接收固件文件
							break;
						}
						case '2':
						{
							DebugPfEx(0,"Download GB Font From Ymodem.....'B' or 'b' = EXIT!  %d(S) Wait...\r\n",(u32)ShowGetFileWait);		//接收完成后返回菜单
							flag = Ymodem_START(2);	//接收GB字库文件
							break;
						}
						case '3':
						{
							DebugPfEx(0,"Download ASCII Font From Ymodem.....'B' or 'b' = EXIT!  %d(S) Wait...\r\n",(u32)ShowGetFileWait);		//接收完成后返回菜单
							flag = Ymodem_START(3);	//接收ASCII字库文件
							break;
						}
						case '4':
						{
							DebugPfEx(0,"Download Auxiliary Application Program Font From Ymodem.....'B' or 'b' = EXIT!  %d(S) Wait...\r\n",(u32)ShowGetFileWait);		//接收完成后返回菜单
							flag = Ymodem_START(4);	//接收辅助应用程序文件
							break;
						}
						default:
						{
							DebugPf(0,"Option Error.....\r\n");		//接收完成后返回菜单
							flag = 0;								//错误
							break;
						}	
					}
					
					if(flag == 1)		 //接收成功
					{
						DebugPf(0,"****************************\r\n");
						DebugPf(0,"File Name : ");
						BootLoader_SendString(GetFlieNameDataAddr(),GetFlieNameLen());		 //文件名字
						BootSendFinishWait();													//等待ISR发送中断将数据发送完	
						DebugPf(0,"\r\n");
						DebugPfEx(0,"File Size : %d\r\n",GetFlieSize());	 	//文件大小	
						DebugPf(0,"****************************\r\n");
						DebugPf(0,"Receive File Complete!! Press Key To Return...\r\n");
	
						BootLoader_GetByte_Wait(&num,168000000);	//显示停留 15秒
					}
					return 1;		   //返回菜单
				}
				case '5':
				{
					BlockDataCheck();													//检查数据块
					BootLoader_GetByte_Wait(CPUID,168000000);	//显示停留 约15秒
					return 1;			//返回菜单
				}
				case '6':
				{
				 	GetCpuIdHex(CPUID);			//获取CPUID
					DebugPfEx(0,"CPU ID：%X-",CPUID[0]);
					DebugPfEx(0,"%X-",CPUID[1]);
					DebugPfEx(0,"%X-",CPUID[2]);
					DebugPfEx(0,"%X-",CPUID[3]);
					DebugPfEx(0,"%X-",CPUID[4]);
					DebugPfEx(0,"%X-",CPUID[5]);
					DebugPfEx(0,"%X-",CPUID[6]);
					DebugPfEx(0,"%X-",CPUID[7]);
					DebugPfEx(0,"%X-",CPUID[8]);
					DebugPfEx(0,"%X-",CPUID[9]);
					DebugPfEx(0,"%X-",CPUID[10]);
					DebugPfEx(0,"%X\r\n",CPUID[11]);
					BootLoader_GetByte_Wait(CPUID,168000000);	//显示停留 约15秒
					return 1;
				}
				case '7':
				{
				 	ShowTime();
					BootLoader_GetByte_Wait(&num,168000000);	//显示停留 约15秒
					return 1;
				}
				case '8':
				{
					NVIC_SystemReset();	//固件库自带重启命令
					return 1;			//返回菜单
				}
				case 'E':		 //退出
				case 'e':		 //退出
				{		  
				 	DebugPf(0,"Exit.....\r\n");	//Bye			
					return 0;
				}
				case 'D':		 //执行辅助程序
				case 'd':		 //执行辅助程序
				{
					//应用区代码检查
					DebugPf(0,"辅助程序效验..........");
					AuxiliaryAppCRCOne = ReadAuxiliaryAppCRCAddress();				//读取存储的CRC效验值
					AuxiliaryAppCRCDataSize = ReadAuxiliaryAppSizeAddress();	//读取数据大小
					if(AuxiliaryAppCRCDataSize < AuxiliaryAppSpaceSize)				//检测数据大小是否超过存放区空间，超过则没有数据，没有数据时数据全FF
					{
						if(ReadAuxiliaryAppUpSrcAddress() == PCBOOT)
							AuxiliaryAppCRCOneBak = CRC32H_Chack8(1,(u8*)((volatile u8*)AuxiliaryAppStartAddress),ReadAuxiliaryAppSizeAddress());		//应用区计算获得的文件校验值		  //STM32硬件效验
						else if(ReadAuxiliaryAppUpSrcAddress() == NETBOOT)
							;
						
						if(AuxiliaryAppCRCOne == AuxiliaryAppCRCOneBak)					//CRC比较
						{
							DebugPf(0,"\r\n辅助应用程序效验通过，跳转执行辅助程序......\r\n");
							
							BootLoaderOver();													//结束引导
							Jump_IAP(AuxiliaryAppStartAddress);				//程序跳转	
						}
						else
						{
							DebugPf(0,"Error!\r\n");									//数据有误
						}
					}
					else
						DebugPf(0,"No Data!\r\n");									//没有数据
				 		
					return 0;
				}
				
				default:			   //刷新菜单
					return 1;		   //返回菜单
			}
		}
	}	
}

/****************************************************************************
* 名	称：void BOOTLoader(const u32 BaudRate)
* 功	能：BootLoader执行程序
* 入口参数：const u32 BaudRate		波特率
* 出口参数：无
* 说	明：程序开始地址为：0x00000000
****************************************************************************/
void BOOTLoader(const u32 BaudRate)
{
	u8 flag = 0;								//执行返回标志

	while(1)
	{
		ShowMenu(BaudRate);										//显示菜单

		CRC_ResetDR();	  										//接收完成时清空CRC
		BootLoader_RX_Clear();								//清空接收缓冲区

		flag = Key_Input((u8)MenuWaitTime);		//等待按键输入
		if(flag == 0)													//退出不用回显菜单
		{	
			DebugPf(0,"BYE!\r\n");
			break;															//退出循环
		}
	}
}
 
/****************************************************************************
* 名	称：void Jump_IAP(const u32 MainAppAddr)
* 功	能：IAP跳转
* 入口参数：const u32 MainAppAddr		主程序地址
* 出口参数：无
* 说	明：跳至应用程序执行
(volatile u32*) 可用 (__IO uint32_t*) 代替
****************************************************************************/
void Jump_IAP(const u32 MainAppAddr)
{
	typedef void(*pFunction)(void);					//定义指向用户程序的函数指针
	u32 JumpAddress = 0;										//跳转地址
	pFunction Jump_To_Application;					//定义用户程序的入口

	JumpAddress = *(volatile u32*)(MainAppAddr + 4);	  //Reset地址 (首地址+4)
	Jump_To_Application = (pFunction)JumpAddress;				//设置应用程序的入口地址 
	__set_MSP(*(volatile u32*)MainAppAddr);			  			//初始化应用程序的堆栈指针	 //注意此处的地址是首地址而不是Reset地址
	Jump_To_Application();									  					//跳转到Main程序
}

