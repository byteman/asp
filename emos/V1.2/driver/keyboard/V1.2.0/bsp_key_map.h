/*
*********************************************************************************************************
*
*	模块名称 : 按键功能映射
*	文件名称 : bsp_key_map.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2012-2013, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_KEY_MAP_H
#define __BSP_KEY_MAP_H

/* 根据应用程序的功能重命名按键宏 */
#define KEY_DOWN_K1		KEY_1_DOWN
#define KEY_UP_K1		KEY_1_UP
#define KEY_LONG_K1		KEY_1_LONG

#define KEY_DOWN_K2		KEY_2_DOWN
#define KEY_UP_K2		KEY_2_UP
#define KEY_LONG_K2		KEY_2_LONG

#define KEY_DOWN_K3		KEY_3_DOWN
#define KEY_UP_K3		KEY_3_UP
#define KEY_LONG_K3		KEY_3_LONG

#define JOY_DOWN_L		KEY_4_DOWN		/* 左 */
#define JOY_UP_L		KEY_4_UP
#define JOY_LONG_L		KEY_4_LONG

#define JOY_DOWN_U		KEY_5_DOWN		/* 上 */
#define JOY_UP_U		KEY_5_UP
#define JOY_LONG_U		KEY_5_LONG

#define JOY_DOWN_D		KEY_6_DOWN		/* 下 */
#define JOY_UP_D		KEY_6_UP
#define JOY_LONG_D		KEY_6_LONG

#define JOY_DOWN_OK		KEY_7_DOWN		/* ok */
#define JOY_UP_OK		KEY_7_UP
#define JOY_LONG_OK		KEY_7_LONG

#define JOY_DOWN_R		KEY_8_DOWN		/* 右 */
#define JOY_UP_R		KEY_8_UP
#define JOY_LONG_R		KEY_8_LONG


/* 按键ID, 主要用于bsp_KeyState()函数的入口参数 */
typedef enum
{
	KID_K1 = 0,
	KID_K2,
	KID_K3,
	KID_JOY_L,
	KID_JOY_U,
	KID_JOY_D,
	KID_JOY_OK,
	KID_JOY_R
}KID_E;

#endif


