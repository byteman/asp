#ifndef _MX25L3208DM2I_12G_INIT_H_	
#define _MX25L3208DM2I_12G_INIT_H_

#ifdef __cplusplus		   	//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include "SPIx.h"
	
#define	MX25L3208_CEPin		GPIO_Pin_9
#define MX25L3208_CEPort	GPIOC

#define MX25L3208_SPI			0//SPI3
	
void MX25L3208_Init(void);											//初始化FLASH设备
void GetMX25L3208ID(SPI_Cfg* spix,u8* ID);			//获取设备ID号
	
#ifdef __cplusplus		   	//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	u8 ID[3]={0};

	MX25L3208_Init();
	GetMX25L3208ID(ID);
*/
