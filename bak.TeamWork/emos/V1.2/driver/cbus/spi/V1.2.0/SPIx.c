#include "SPIx.h"
#include "dt3102_IO.h"

#define FLASH_SCK_PIN		GPIO_Pin_10
#define FLASH_SCK_PORT	GPIOC
#define FLASH_MISO_PIN	GPIO_Pin_11
#define FLASH_MISO_PORT	GPIOC
#define FLASH_MOSI_PIN	GPIO_Pin_12
#define FLASH_MOSI_PORT	GPIOC


/****************************************************************************
* 名	称：u8 SPIxbyteSendGet(SPI_TypeDef* SPIt,u8 byte)	
* 功	能：硬件SPIx发送接收byte
* 入口参数：SPI_TypeDef* 	SPIt	SPI 端口号
						u8 						byte	发送命令 or 接收字符
* 出口参数：u8									接收到的数据
* 说	明：无			
****************************************************************************/
u8 SPIxbyteSendGet(SPI_TypeDef* SPIt,u8 byte)		 
{
	while (SPI_I2S_GetFlagStatus(SPIt, SPI_I2S_FLAG_TXE) == RESET);		 	//如果输出缓冲区为空
	SPI_I2S_SendData(SPIt, byte);			   																//从SPI串口发送数据0xFF
	while (SPI_I2S_GetFlagStatus(SPIt, SPI_I2S_FLAG_RXNE) == RESET);	 	//如果接收缓冲区不为空
	return (u8)SPI_I2S_ReceiveData(SPIt);
}

/****************************************************************************
* 名	称：void SPIxWordSend(SPI_TypeDef* SPIt,u8 Word)
* 功	能：硬件SPIx发送Word
* 入口参数：SPI_TypeDef* SPIt		SPI 端口号	
						u8 Word							发送命令
* 出口参数：无
* 说	明：无			
****************************************************************************/
void SPIxWordSend(SPI_TypeDef* SPIt,u32 Word)		 
{
	SPIxbyteSendGet(SPIt,(Word & 0xff000000) >> 24);
	SPIxbyteSendGet(SPIt,(Word & 0x00ff0000) >> 16);
	SPIxbyteSendGet(SPIt,(Word & 0x0000ff00) >> 8);
	SPIxbyteSendGet(SPIt,(Word & 0x000000ff) >> 0);
}

/****************************************************************************
* 名	称：u32 SPIxWordGet(SPI_TypeDef* SPIt)	
* 功	能：硬件SPIxWord接收
* 入口参数：SPI_TypeDef* SPIt		SPI 端口号
* 出口参数：接收到的32位数据
* 说	明：无			
****************************************************************************/
u32 SPIxWordGet(SPI_TypeDef* SPIt)		 
{
	u32 Word = 0;
	
	Word = (SPIxbyteSendGet(SPIt,0xA5) << 24);
	Word |= (SPIxbyteSendGet(SPIt,0xA5) << 16);
	Word |= (SPIxbyteSendGet(SPIt,0xA5) << 8);
	Word |= (SPIxbyteSendGet(SPIt,0xA5) << 0);
	
	return Word;
}

/****************************************************************************
* 名	称：void SPIS_Delay(u32 nCount)
* 功	能：一个工作的频率延时
* 入口参数：u32 nCount	延时量
* 出口参数：无
* 说	明：无
****************************************************************************/
static void SPIS_Delay(u32 nCount)
{
  while(nCount--);
}

/****************************************************************************
* 名	称：void SPISSendcmd8(SPI_Cfg* Sx,u8 cmd)
* 功	能：模拟SPI发送8位命令
* 入口参数：SPI_Cfg* Sx			SPI配置
						u8 cmd					发送的命令
* 出口参数：无
* 说	明：无
****************************************************************************/
void SPISSendcmd8(SPI_Cfg* Sx,u8 cmd)
{
	u8 i;	//8位累加器
	
	GPIO_ResetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);								//时钟复位
	for(i = 0;i < 8;i++)			//从高位开始发
	{
		if((cmd << i) & 0x80)   //装载数据
			GPIO_SetBits(Sx->SPIx_MOSI_Por,Sx->SPIx_MOSI_Pin); 
    else
			GPIO_ResetBits(Sx->SPIx_MOSI_Por,Sx->SPIx_MOSI_Pin);	

	 	GPIO_ResetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);							//时钟准备 
	 	SPIS_Delay(2);
	 	GPIO_SetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);								//发送数据
	 	SPIS_Delay(2);
	 	GPIO_ResetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);							//发送完毕
	}
}

/****************************************************************************
* 名	称：u8 SPISReciveData8(SPI_Cfg* Sx)
* 功	能：模拟SPI接收8位数据
* 入口参数：SPI_Cfg* Sx				SPI配置
* 出口参数：u8								接收到的8位数据
* 说	明：无
****************************************************************************/
u8 SPISReciveData8(SPI_Cfg* Sx)
{
	u8 temp = 0;						//读取到的数据
	u8 i;										//8位累加器
	
	for(i = 0;i < 8;i++)		//从高位开始发
	{
		GPIO_ResetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);										//时钟准备 
		SPIS_Delay(2);
		GPIO_SetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);											//开始接收
		temp <<= 1; 
		temp |= GPIO_ReadInputDataBit(Sx->SPIx_MISO_Por,Sx->SPIx_MISO_Pin);	 	//数据
		SPIS_Delay(2);
		GPIO_ResetBits(Sx->SPIx_SCK_Por,Sx->SPIx_SCK_Pin);										//发送完毕
	}
	return temp;
}

/****************************************************************************
* 名	称：u8 SPISbyteSendGet(SPI_Cfg* Sx,u8 cmd)
* 功	能：模拟SPI接收发送8位数据
* 入口参数：SPI_Cfg* Sx			SPI接口
						u8 cmd					发送的命令
* 出口参数：u8							接收的数据
* 说	明：无
****************************************************************************/
u8 SPISbyteSendGet(u8 cmd)		 
{
	u8 i;																		//8位累加器
	u8 temp = 0;														//读取到的数据
	
	IoLo(FLASH_SCK);
	for(i = 0;i < 8;i++)										//从高位开始发
	{
		if((cmd << i) & 0x80)   							//装载数据
		{
			IoHi(FLASH_MOSI);
		}
    else
		{
			IoLo(FLASH_MOSI);	
		}			

		IoLo(FLASH_SCK);
	 	SPIS_Delay(2);
		IoHi(FLASH_SCK);
		temp <<= 1;
		temp |= GetIo(FLASH_MISO);
	 	SPIS_Delay(2);
	}
	return temp;
}

/****************************************************************************
* 名	称：void SPISWordSend(SPI_Cfg* Sx,u8 Word)
* 功	能：模拟SPI Word发送
* 入口参数：SPI_Cfg* Sx				SPI接口
						Word							发送的命令
* 出口参数：无
* 说	明：无			
****************************************************************************/
void SPISWordSend(SPI_Cfg* Sx,u32 Word)		 
{
	SPISbyteSendGet((Word & 0xff000000) >> 24);
	SPISbyteSendGet((Word & 0x00ff0000) >> 16);
	SPISbyteSendGet((Word & 0x0000ff00) >> 8);
	SPISbyteSendGet((Word & 0x000000ff) >> 0);
}

/****************************************************************************
* 名	称：u32 SPISWordGet(SPI_Cfg* Sx)	
* 功	能：模拟SPI Word接收
* 入口参数：SPI_Cfg* Sx				SPI接口
* 出口参数：接收到的32位数据
* 说	明：无			
****************************************************************************/
u32 SPISWordGet(SPI_Cfg* Sx)		 
{
	u32 Word = 0;
	
	Word = (SPISbyteSendGet(0xA5) << 24);
	Word |= (SPISbyteSendGet(0xA5) << 16);
	Word |= (SPISbyteSendGet(0xA5) << 8);
	Word |= (SPISbyteSendGet(0xA5) << 0);
	
	return Word;
}

/****************************************************************************
* 名	称void SPIx_Init(SPI_Cfg* Sx)
* 功	能：通用SPI引脚初始化
* 入口参数：SPI_Cfg* Sx			SPI配置参数
* 出口参数：无
* 说	明：无
****************************************************************************/
void SPIx_Init()
{
	GPIO_InitTypeDef G;						//引脚变量

	//选择开启IO口时钟
// 	if((FLASH_SCK_PORT == GPIOC) || (FLASH_MISO_PORT == GPIOC) || (FLASH_MOSI_PORT == GPIOC))
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//开启引脚时钟C
	
// 	G.GPIO_Pin = FLASH_SCK_PIN;			//SCK
// 	G.GPIO_Mode = GPIO_Mode_OUT;
// 	G.GPIO_OType = GPIO_OType_PP;
// 	G.GPIO_Speed = GPIO_Speed_100MHz;
// 	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(FLASH_SCK_PORT, &G);
	
	IoSetMode(FLASH_SCK,IO_MODE_OUTPUT);
	IoPushPull(FLASH_SCK);
	IoSetSpeed(FLASH_SCK,IO_SPEED_100M);
	
	G.GPIO_Pin = FLASH_MOSI_PIN;			//SCK MOSI
	GPIO_Init(FLASH_MOSI_PORT, &G);		
	
// 	G.GPIO_Pin = FLASH_MISO_PIN;		//MISO
// 	G.GPIO_Mode = GPIO_Mode_IN;
// 	G.GPIO_OType = GPIO_OType_OD;
// 	G.GPIO_Speed = GPIO_Speed_2MHz;
// 	G.GPIO_PuPd = GPIO_PuPd_NOPULL;
// 	GPIO_Init(FLASH_MISO_PORT, &G);	

	IoSetMode(FLASH_MISO,IO_MODE_INPUT);
	IoOpenDrain(FLASH_MISO);
	IoSetSpeed(FLASH_MISO,IO_SPEED_2M);
}

/****************************************************************************
* 名	称：u32 SPIbyteSendGet(SPI_Cfg* Sx)	
* 功	能：模拟SPI硬件SPI单字节接收
* 入口参数：SPI_Cfg* Sx		SPI配置
* 出口参数：接收到的8位数据
* 说	明：无			
****************************************************************************/
u8 SPIbyteSendGet(u8 cmd)	 
{
		return SPISbyteSendGet(cmd);								//需要用到引脚
}

/****************************************************************************
* 名	称：u32 SPIWordGet(SPI_Cfg* Sx)	
* 功	能：模拟SPI硬件SPI字接收
* 入口参数：SPI_Cfg* Sx		SPI配置
* 出口参数：接收到的32位数据
* 说	明：无			
****************************************************************************/
u32 SPIWordGet(SPI_Cfg* Sx)		 
{
	if(Sx->SPIx == (SPI_TypeDef*)0)					//模拟SPI
		return SPISWordGet(Sx);								//需要用到引脚
	else																		//硬件SPI
		return SPIxWordGet(Sx->SPIx);					//只会用到SPI端口号
}

/****************************************************************************
* 名	称：void SPIWordSend(SPI_Cfg* Sx,u32 Word)	
* 功	能：模拟SPI硬件SPI字发送
* 入口参数：SPI_Cfg* Sx				SPI配置
* 出口参数：发送32位数据
* 说	明：无			
****************************************************************************/
void SPIWordSend(SPI_Cfg* Sx,u32 Word)		 
{
	if(Sx->SPIx == (SPI_TypeDef*)0)						//模拟SPI
		SPISWordSend(Sx,Word);									//需要用到引脚
	else																			//硬件SPI
		SPIxWordSend(Sx->SPIx,Word);						//只会用到SPI端口号
}
