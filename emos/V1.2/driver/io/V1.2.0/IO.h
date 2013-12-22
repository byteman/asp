/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: IO.h
**创   建   人: 
**创 建 日  期: 
**修        改: 
**最后修改日期: 
**描        述: 包括了关于CPU的一些基本的IO操作(STM32F40X系列)
********************************************************************************************************/
#ifndef		_IO_ACCESS_H_
#define		_IO_ACCESS_H_

//输入输出选择
#define IO_MODE_INPUT 		0x00000000UL
#define IO_MODE_OUTPUT		0x55555555UL
#define IO_MODE_ALTER			0xAAAAAAAAUL
#define IO_MODE_ANALOG		0xFFFFFFFFUL

//输出方式
#define IO_PUSH_PULL			0x0000UL
#define IO_OPEN_DRAIN			0xFFFFUL

//速度选择
#define IO_SPEED_2M 		0x00000000UL
#define IO_SPEED_25M		0x55555555UL
#define IO_SPEED_50M		0xAAAAAAAAUL
#define IO_SPEED_100M		0xFFFFFFFFUL




//设置IO端口属性
#define IoSetMode(bit, mode) (bit##_PORT##->MODER) &= ~(((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN)) | (((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN))<<1)); \
														 (bit##_PORT##->MODER) |= ((((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN)) | (((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN))<<1)) & mode)

#define IoSetSpeed(bit, speed) (bit##_PORT##->OSPEEDR) &= ~(((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN)) | (((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN))<<1)); \
																(bit##_PORT##->OSPEEDR) |= ((((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN)) | (((unsigned long)(bit##_PIN) * (unsigned long)(bit##_PIN))<<1)) & speed)

#define IoPushPull(bit) 		(bit##_PORT##->OTYPER) &= (~(bit##_PIN))
#define IoOpenDrain(bit) 		(bit##_PORT##->OTYPER) |= (bit##_PIN)


//设置IO状态
#define IoHi(bit)							(bit##_PORT##->BSRRL) = bit##_PIN
#define IoLo(bit)							(bit##_PORT##->BSRRH) = bit##_PIN 
																		
//获取IO状态
#define GetIo(bit)						(((bit##_PORT##->IDR & (bit##_PIN)) > 0) ? 1 : 0)



/*****************

1、端口和引脚定义方法
定义端口时，假设要控制的引脚命名为SCK，则须定义
#define SCK_PORT					GPIOA
#define SCK_PIN						GPIO_Pin_1


2、使用方法
输出高电平时调用 IoHi(SCK);
输出低电平时调用 IoLo(SCK);
获取IO电平时调用 GetIo(SCK);
设置IO端口属性，例如设置为50MHz速度下推挽输出：
                 IoSetMode(SCK, IO_MODE_OUTPUT);
								 IoSetSpeed(SCK, IO_SPEED_50M);
								 IoPushPull(SCK);

******************/





		
//************************************************************************
#endif

