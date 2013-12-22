
#ifndef __IOCTRL_H
#define __IOCTRL_H
#include "stm32f4xx.h"

// #define LED_PIN                   GPIO_Pin_14
// #define LED_GPIO_PORT             GPIOD

typedef struct  {
		u8 In0;             // 输入测试电平 0:低电平  1:高电平
    u8 In1;             // 输入测试电平 0:低电平  1:高电平
    u8 In2;             // 输入测试电平 0:低电平  1:高电平
    u8 In3;             // 输入测试电平 0:低电平  1:高电平
    u8 In4;             // 输入测试电平 0:低电平  1:高电平
    u8 In5;             // 输入测试电平 0:低电平  1:高电平
    u8 In6;             // 输入测试电平 0:低电平  1:高电平
    u8 In7;             // 输入测试电平 0:低电平  1:高电平
		u8 In8;             // 输入测试电平 0:低电平  1:高电平
    u8 In9;             // 输入测试电平 0:低电平  1:高电平
    
} INTEST;

typedef struct  {
		u8 Out0;             // 输入测试电平 0:低电平  1:高电平
    u8 Out1;             // 输入测试电平 0:低电平  1:高电平
    u8 Out2;             // 输入测试电平 0:低电平  1:高电平
    u8 Out3;             // 输入测试电平 0:低电平  1:高电平
    
} OUTTEST;

// void IOInit(void);
void OutPortTest(void);
// u8 ReadInfCurtain(void);
// u8 ReadInductCoil(void);
// void LedSet(u8 en);
// void LedToggle(void);

void InPortTest(void);

// #define INTEST char
// #define OUTTEST char

// extern INTEST	InTest;
// extern OUTTEST	OutTest;



#endif


