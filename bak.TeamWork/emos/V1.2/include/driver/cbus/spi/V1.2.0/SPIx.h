#ifndef _SPIX_H_	
#define _SPIX_H_

#ifdef __cplusplus		   		
extern "C" {
#endif

#include "includes.h"			
#include "driver.h"	

// 映射表		SPIx						映射1		映射2
// 					SPI1_SCK				PA5			PB3
// 					SPI1_MISO				PA6			PB4
// 					SPI1_MOSI				PA7			PB5
// 					SPI2_SCK				PB13		PB10
// 					SPI2_MISO				PB14		PC2
// 					SPI2_MOSI				PB15		PC3
// 					SPI3_SCK				PC10		PB3
// 					SPI3_MISO				PC11		PB4
// 					SPI3_MOSI				PC12		PB5
	
#define SPIS		(SPI_TypeDef*)0				//软件模拟SPI
	
typedef struct
{
	SPI_TypeDef*		SPIx;								//SPI选择，0-IO口模拟SPI；1-SPI1；2-SPI2；3-SPI3；其他无效
	
	u16 						SPIx_SCK_Pin;				//SCK引脚号				GPIO_Pin_0 - GPIO_Pin_15
	GPIO_TypeDef* 	SPIx_SCK_Por;				//SCK端口号				GPIOA - GPIOI
	u16 						SPIx_MISO_Pin;			//MISO引脚号			GPIO_Pin_0 - GPIO_Pin_15
	GPIO_TypeDef* 	SPIx_MISO_Por;			//MISO端口号			GPIOA - GPIOI
	u16 						SPIx_MOSI_Pin;			//MOSI引脚号			GPIO_Pin_0 - GPIO_Pin_15	
	GPIO_TypeDef* 	SPIx_MOSI_Por;			//MOSI端口号			GPIOA - GPIOI
	uint16_t				SPIx_BaudRate;			//波特率					SPI_BaudRatePrescaler_2	4	8	16	32	64	128	256
}SPI_Cfg;															//SPI配置	

void SPIx_Init(void);			//初始化SPI，返回：HVL_NO_ERR表示初始化成功；返回：HVL_PARAM_ERR表示输入参数错误初始化失败

u8 SPIbyteSendGet(u8 byte);								//模拟SPI硬件SPI接收发送8位数据
void SPIWordSend(SPI_Cfg* Sx,u32 Word)	;							//模拟SPI硬件SPI Word发送
u32 SPIWordGet(SPI_Cfg* Sx);													//模拟SPI硬件SPI Word接收

void SPISSendcmd8(SPI_Cfg* Sx,u8 cmd);								//模拟SPI发送8位命令
u8 SPISReciveData8(SPI_Cfg* Sx);											//模拟SPI接收8位数据

#ifdef __cplusplus		   		
}
#endif

#endif

/*
软件模拟SPI
#include "MX25L3208DM2I_12G_init.h"
#include "SPIx.h"

u8 ID[3]={0};

SPI_Cfg SPIUser =
{
	SPIS,
	GPIO_Pin_10,
	GPIOC,
	GPIO_Pin_11,
	GPIOC,
	GPIO_Pin_12,
	GPIOC
};

	MX25L3208_Init();
	SPIx_Init(&SPIUser);
	GetMX25L3208ID(SPIS,ID);
*/

/*
硬件SPI
#include "MX25L3208DM2I_12G_init.h"
#include "SPIx.h"

u8 ID[3]={0};

SPI_Cfg SPIUser =
{
	SPI3,
	GPIO_Pin_10,
	GPIOC,
	GPIO_Pin_11,
	GPIOC,
	GPIO_Pin_12,
	GPIOC
};

	MX25L3208_Init();
	SPIx_Init(&SPIUser);
	GetMX25L3208ID(SPI3,ID);
*/
