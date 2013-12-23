#ifndef _ConnectCfg_H_
#define _ConnectCfg_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include "tim6_init.h"					//TIM6_ResetCount
#include "usart1_init.h"				//串口
#include "usart3_init.h"				//串口
#include "usart6_init.h"				//串口
	
/*
STM32F4xx 1M 空间分配
size			size				size				name				addr
1M				0x100000		1048576			Space				[0x8000000 - 0x8100000]		
Sector0 - Sector11	16K  16K  16K  16K  64K  128K  128K  128K  128K  128K  128K  128K
按扇区擦除数据


32K				0x008000		32768				BootLoader	[0x8000000 - 0x8007FFF]		Sector0 - Sector1		16K  16K
	
16K				0x004000		16384				AppFlag			[0x8008000 - 0x800BFFF]		Sector2							16K
4Byte			0x000004		4						AppUP				[0x8008000 - 0x8008003]
4Byte			0x000004		4						AppERR			[0x8008004 - 0x8008007]

16K				0x004000		16384				NULL				[0x800C000 - 0x800FFFF]		Sector3							16K

64K				0x010000		65536				NULL				[0x8010000 - 0x801FFFF]		Sector4							64K

256K			0x040000		262144			App					[0x8020000 - 0x805FFFF]		Sector5 - Sector6		128K  128K

256K			0x040000		262144			AppBak			[0x8060000 - 0x809FFFF]		Sector7 - Sector8		128K  128K
4Byte			0x000004		4						AppBakCRC		[0x809FFFC - 0x809FFFF]
4Byte			0x000004		4						AppBakSize	[0x809FFF8 - 0x809FFFB]
4Byte			0x000004		4						AppBakUPSrc	[0x809FFF4 - 0x809FFF7]

16K				0x004000		16384				ASCII				[0x800C000 - 0x800FFFF]		Sector3							16K
2K				0x000800		2048				ASCIIFont		[0x800C000 - 0x800C7FF]
4Byte			0x000004		4						ASCIICRC		[0x800FFFC - 0x800FFFF]

16K				0x020000		131072			ASCII				[0x80A0000 - 0x80BFFFF]		Sector9							128K
2K				0x000800		2048				ASCIIFont		[0x80A0000 - 0x80A07FF]
4Byte			0x000004		4						ASCIICRC		[0x80BFFFC - 0x80BFFFF]
4Byte			0x000004		4						ASCIISize		[0x80BFFF8 - 0x80BFFFB]
4Byte			0x000004		4						ASCIIUpSrc	[0x80BFFF4 - 0x80BFFF7]

256K			0x040000		262144			GB					[0x80C0000 - 0x80FFFFF]		Sector10 - Sector11	128K  128K
225K			0x038400		230400			GBFont			[0x80C0000 - 0x80D83FF]
4Byte			0x000004		4						GBCRC				[0x80FFFFC - 0x80FFFFF]
4Byte			0x000004		4						GBSize			[0x80FFFF8 - 0x80FFFFB]
4Byte			0x000004		4						GBUpSrc			[0x80FFFF4 - 0x80FFFF7]
*/

//system_stm32f4xx.c Line 150				stm32f4xx.h		Line 91									//CPU频率

//ComPort
#define BootLoaderUsart_BaudRate		57600																	//通讯口波特率
#define BootLoaderUsart_Config			USART1_Config													//通讯口配置
#define BootLoaderUsart_Init				USART1_Init														//通讯口初始化
#define ComSendByte  								USART1_SendByte												//通讯口单字符发送
#define ComGetByte_Wait  						USART1_GetByte_WaitTime								//通讯口接收等待
#define BootLoaderUsart_GetString 	USART1_GetString											//通讯口获取多个字符
#define ComSendString 							USART1_SendString											//通讯口字符串发送
#define BootLoaderUsart_RX_Clear 		USART1_RX_Buffer_Clear								//通讯口接收清空
#define ComSendFinishWait()					while(USART1CheckISRSendBusy());			//通讯口等待发送完成
#define PrintfPort									1																			//打印显示端口号 即串口号

//Message
#define BootVersion									"STM32F4_BootLoader_Version: 000001"	//版本号
#define MenuWaitTime								15																		//引导菜单无操作跳转时间 秒
#define ShowGetFileWait							60																		//显示等待接收文件N秒		具体等待时间为YmodemGetFileWaits变量，需一致
#define CopyEver										0																			//备份应用区拷贝至应用区出错时，是否一直执行拷贝动作	0不执行	1执行
#define GetTimeMSecCnt							TIM6_GetCount													//获取毫秒的函数
#define ResetTimeMSecCnt						TIM6_ResetCount												//复位计数器值

//#define RTCTimeFlag													//实时时钟开启标志		屏蔽关闭实时时钟
//#define AutoBaudRateCheckFlag								//是否自动识别波特率		启用有		屏蔽无
//#define BootKey															//是否有引导按键		启用有		屏蔽无		引导按键和命令按键只能选其一，两个都选则只有引导按键有效
#define CmdKey															//是否有命令按键	启用有		屏蔽无				如果引导按键开启则命令按键无效
		
#ifdef BootKey
#define BootKeyPort			GPIOE									//引导键引脚端口号
#define BootKeyPin			GPIO_Pin_0						//引导键引脚号
#define BootKeyDownVal	Bit_RESET							//引导键按下时电平为高还是低
#define BootKeyRCC			RCC_AHB1Periph_GPIOE	//引导建时钟电源
#endif

void BlockDataCheck(void);										//数据块检查
u32 AutoBaudRateCheck(const u8 Val);					//自动波特率识别检查
void TimDelayMS(const u16 DelayMS);						//定时器毫秒级延迟
void BootLoaderOver(void);										//BootLoader结束初始化

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}						   
#endif

#endif
