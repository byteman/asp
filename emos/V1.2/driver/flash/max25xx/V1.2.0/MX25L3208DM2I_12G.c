#include "MX25L3208DM2I_12G_init.h"

/****************************************************************************
* 名	称：void MX25L3208_CE(u8 Flag)
* 功	能：电源控制
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void MX25L3208_CE(u8 Flag)
{
	if(Flag == 1)
		GPIO_SetBits(GPIOC, GPIO_Pin_9);
	else
		GPIO_ResetBits(GPIOC, GPIO_Pin_9);
}

/****************************************************************************
* 名	称：void MX25L3208_Init(void)
* 功	能：DATAFLASH初始化
* 入口参数：无
* 出口参数：无
* 说	明：硬件SPI接口
****************************************************************************/
void MX25L3208_Init(void)
{
	GPIO_InitTypeDef G;																			//引脚变量
	
	if(MX25L3208_CEPort == GPIOA)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOB)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOC)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOD)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOE)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOF)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOG)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOH)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);		//打开引脚时钟
	else if(MX25L3208_CEPort == GPIOI)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);		//打开引脚时钟
	
  G.GPIO_Pin = MX25L3208_CEPin;			//引脚号		FLASH_CE
  G.GPIO_Mode = GPIO_Mode_OUT;			//输出模式
  G.GPIO_OType = GPIO_OType_PP;			//推挽
  G.GPIO_Speed = GPIO_Speed_50MHz;	//引脚频率
  G.GPIO_PuPd = GPIO_PuPd_NOPULL;		//悬浮
  GPIO_Init(MX25L3208_CEPort, &G);
	
	MX25L3208_CE(1);									//初始化电源为关闭
}

/****************************************************************************
* 名	称：void GetMX25L3208ID(SPI_Cfg* spix,u8* ID)
* 功	能：获取FLASH ID号
* 入口参数：SPI_Cfg* spix					SPI配置
						u8* ID								获取的FLASH ID号
* 出口参数：无
* 说	明：u8 ID[3] = {0};
****************************************************************************/
void GetMX25L3208ID(SPI_Cfg* spix,u8* ID)
{
	//测试模拟SPI单独的接收，单独的发送
	if(spix->SPIx == (SPI_TypeDef*)0)	
	{
		MX25L3208_CE(0);
		SPISSendcmd8(spix,0x9F);
		ID[0] = SPISReciveData8(spix);
		ID[1] = SPISReciveData8(spix);
		ID[2] = SPISReciveData8(spix);
		MX25L3208_CE(1);
	}
	
	ID[0] = 0;ID[1] = 0;ID[2] = 0;		//复位
	
	//测试模拟SPI硬件SPI单字节收发
	MX25L3208_CE(0);
	SPIbyteSendGet(spix,0x9F);
	ID[0] = SPIbyteSendGet(spix,0);
	ID[1] = SPIbyteSendGet(spix,0);
	ID[2] = SPIbyteSendGet(spix,0);
  MX25L3208_CE(1); 
}
