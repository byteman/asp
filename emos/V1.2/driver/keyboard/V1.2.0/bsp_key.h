/*
*********************************************************************************************************
*
*	模块名称 : 按键驱动模块
*	文件名称 : bsp_key.h
*	版    本 : V1.0
*	说    明 : 头文件
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

#ifndef __BSP_KEY_H
#define __BSP_KEY_H

#define KEY_COUNT    30	   					/* 按键个数 */

/*
	按键滤波时间50ms, 单位10ms。
	只有连续检测到50ms状态不变才认为有效，包括弹起和按下两种事件
	即使按键电路不做硬件滤波，该滤波机制也可以保证可靠地检测到按键事件
*/
#define KEY_FILTER_TIME   2
#define KEY_LONG_TIME     20			/* 单位10ms， 持续1秒，认为长按事件 */

/*
	每个按键对应1个全局的结构体变量。
*/
typedef struct
{
	/* 下面是一个函数指针，指向判断按键手否按下的函数 */
	uint8_t (*IsKeyDownFunc)(void); /* 按键按下的判断函数,1表示按下 */

	uint8_t  Count;			/* 滤波器计数器 */
	uint16_t LongCount;		/* 长按计数器 */
	uint16_t LongTime;		/* 按键按下持续时间, 0表示不检测长按 */
	uint8_t  State;			/* 按键当前状态（按下还是弹起） */
	uint8_t  RepeatSpeed;	/* 连续按键周期 */
	uint8_t  RepeatCount;	/* 连续按键计数器 */
}KEY_T;

/*
	定义键值代码, 必须按如下次序定时每个键的按下、弹起和长按事件

	推荐使用enum, 不用#define，原因：
	(1) 便于新增键值,方便调整顺序，使代码看起来舒服点
	(2) 编译器可帮我们避免键值重复。
*/

#if 0
typedef enum
{
	KEY_NONE = 0, 
	KEY_ERR = 0,         							//错误按键 
	KEY_0,											//0
	KEY_1,											//1
	KEY_2,											//2
	KEY_3,											//3
	KEY_4,											//4
	KEY_5,											//5
	KEY_6,											//6
	KEY_7,											//7
	KEY_8,											//8
	KEY_9,											//9
	KEY_Del,										//删除
	KEY_UNIT,										//单位
	KEY_OK,											//确定
	KEY_QUERY,										//查询
	KEY_ADD,										//+-
	KEY_DOT,										//.
	KEY_PRINT,										//打印
	KEY_UP,											//上
	KEY_LEFT,										//左
	KEY_RIGHT,										//右
	KEY_MENU,										//菜单
	KEY_BACK,										//返回
	KEY_DOWN,										//下
	KEY_CAR,										//整车
	KEY_AXIS,										//轴重
	KEY_DIAGNOSIS,									//诊断
	KEY_ZERO,										//置零
}VKEY_VAL; 									//按键值

#endif 

/* 按键FIFO用到变量 */
#define KEY_FIFO_SIZE	40

#define KEY_DOWN	1
#define KEY_UP		2
#define KEY_LONG	3 

typedef struct
{
	uint8_t KeyCode;					/* 缓冲区读指针 */
	uint8_t KeyValue;					/* 缓冲区写指针 */
}KEY_INFO_T;

typedef struct
{
	KEY_INFO_T 	Buf[KEY_FIFO_SIZE];		/* 键值缓冲区 */
	uint8_t 		Read;					/* 缓冲区读指针 */
	uint8_t 		Write;					/* 缓冲区写指针 */
}KEY_FIFO_T;

/* 供外部调用的函数声明 */
void bsp_InitKey(void);
void bsp_KeyScan(void);
void bsp_PutKey(uint8_t _KeyCode, uint8_t _KeyValue);
uint16_t bsp_GetKey(void);
uint8_t bsp_GetKeyState(VKEY_VAL _ucKeyID);

#endif

/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
