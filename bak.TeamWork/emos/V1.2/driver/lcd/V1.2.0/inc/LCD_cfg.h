#ifndef __LCD_CFG_H_
#define __LCD_CFG_H_


#include "includes.h"

//while循环= 3+n*5个指令,1uS=168个指令    一个while循环一般为两个指令周期，一个for循环有三个指令周期
#define NS100_DLY(x) ((uint16)(20*x))//其中20代表经验值，此液晶的刷新延时周期必须大于150ns
#define LCD_Delay(x) WhileDly(x)



// //控制引脚
// static LCD_IO WR 			= {GPIOG,GPIO_Pin_8};		//LCD_WR			//写信号		L有效
// static LCD_IO RD 			= {GPIOG,GPIO_Pin_9};		//LCD_RD			//读信号		L有效
// static LCD_IO CE 			= {GPIOG,GPIO_Pin_10};		//LCD_CE			//使能			L有效
// static LCD_IO CD 			= {GPIOG,GPIO_Pin_11};		//LCD_CD			//命令or数据	H命令 L数据
// static LCD_IO RST 		    = {GPIOG,GPIO_Pin_12};		//LCD_RST			//复位			L有效
// static LCD_IO FS 			= {GPIOG,GPIO_Pin_13};		//LCD_FS			//字体				
// static LCD_IO LED 		    = {GPIOG,GPIO_Pin_15}; 		//LCD_LED			//背光			H背光亮 L背光灭		
// static LCD_IO AD_CTRL        = {GPIOC,GPIO_Pin_8};		//LCD_AD_CTRL		//控制引脚
#define LCD_DATA1_PORT		GPIOG
#define LCD_DATA1_PIN		GPIO_Pin_0
#define LCD_DATA2_PORT		GPIOG
#define LCD_DATA2_PIN		GPIO_Pin_1
#define LCD_DATA3_PORT		GPIOG
#define LCD_DATA3_PIN		GPIO_Pin_2
#define LCD_DATA4_PORT		GPIOG
#define LCD_DATA4_PIN		GPIO_Pin_3
#define LCD_DATA5_PORT		GPIOG
#define LCD_DATA5_PIN		GPIO_Pin_4
#define LCD_DATA6_PORT		GPIOG
#define LCD_DATA6_PIN		GPIO_Pin_5
#define LCD_DATA7_PORT		GPIOG
#define LCD_DATA7_PIN		GPIO_Pin_6
#define LCD_DATA8_PORT		GPIOG
#define LCD_DATA8_PIN		GPIO_Pin_7

#define LCD_WR_PORT			GPIOG
#define LCD_WR_PIN			GPIO_Pin_8
#define LCD_RD_PORT			GPIOG
#define LCD_RD_PIN			GPIO_Pin_9
#define LCD_CE_PORT			GPIOG
#define LCD_CE_PIN			GPIO_Pin_10
#define LCD_CD_PORT			GPIOG
#define LCD_CD_PIN			GPIO_Pin_11
#define LCD_RST_PORT		GPIOG
#define LCD_RST_PIN			GPIO_Pin_12
#define LCD_FS_PORT			GPIOG
#define LCD_FS_PIN			GPIO_Pin_13
#define LCD_LED_PORT		GPIOG
#define LCD_LED_PIN			GPIO_Pin_15
#define LCD_AD_CTRL_PORT	GPIOC
#define LCD_AD_CTRL_PIN		GPIO_Pin_8

#endif
