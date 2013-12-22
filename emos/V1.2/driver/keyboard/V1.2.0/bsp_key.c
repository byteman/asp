/*
*********************************************************************************************************
*
*	模块名称 : 独立按键驱动模块
*	文件名称 : bsp_key.c
*	版    本 : V1.0
*	说    明 : 扫描独立按键，具有软件滤波机制，具有按键FIFO。可以检测如下事件：
*				(1) 按键按下
*				(2) 按键弹起
*				(3) 长按键
*				(4) 长按时自动连发
*
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2013-02-01 armfly  正式发布
*
*	Copyright (C), 2013-2014, 安富莱电子 www.armfly.com
*
*********************************************************************************************************
*/

//#include <rthw.h>
//#include <rtthread.h>

#include "Keyboard.h"
#include "Key_cfg.h"
#include "stm32f4xx.h"
#include "c2.h"
#include "driver.h"
#include "dt3102_IO.h"
#include "IO.h"


#include "bsp_key.h"
#include "key_cfg.h"
#include "Keyboard.h"

static KEY_T s_tBtn[KEY_COUNT + 1]; 
static KEY_FIFO_T s_tKey;		/* 按键FIFO变量,结构体 */
 
 u16 KeyPin = 0;	
 GPIO_TypeDef* KeyPort = (GPIO_TypeDef*)0;

/*
	该程序适用于安富莱STM32-X4、STM32-F4开发板

	如果用于其它硬件，请修改GPIO定义和 IsKeyDown1 - IsKeyDown8 函数

	如果用户的按键个数小于8个，你可以将多余的按键全部定义为和第1个按键一样，并不影响程序功能
	#define KEY_COUNT    8	  这个在 bsp_key.h 文件中定义
*/

/*
*********************************************************************************************************
*	函 数 名: IsKeyDownX
*	功能说明: 判断按键是否按下
*	形    参: 无
*	返 回 值: 返回值1 表示按下，0表示未按下
*********************************************************************************************************
*/
typedef struct
{
	uint16_t KeyPin;		/* 键盘引脚*/
	GPIO_TypeDef* KeyPort;				 
}KEY_IO_T;

static KEY_IO_T KeyIO[MATRIX_KEY_ROW + 1] = 
{
	{KEY_L1_PIN,KEY_L1_PORT},
	{KEY_L2_PIN,KEY_L2_PORT},
	{KEY_L3_PIN,KEY_L3_PORT},
	{KEY_L4_PIN,KEY_L4_PORT},
	{KEY_L5_PIN,KEY_L5_PORT},
	{KEY_L6_PIN,KEY_L6_PORT},
	{KEY_L7_PIN,KEY_L7_PORT},
};

/****************************************************************************
* 名	称：static s32 BEEP_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		无用
						u32 len				无用
						u32* Reallen	无用
						u32 lParam		参数
* 出口参数：s32						按键值	0没有	非0即按键值
* 说	明：无
****************************************************************************/
static s32 bsp_KEY_Read(void)
{
	s8	s8KeyValue;
	u8	u8KeyLine,u8KeyRow,err;
	u8	i = 0;
	
	for(i = 0;i < MATRIX_KEY_ROW;i++)
	{
		KeyPin 	= KeyIO[i].KeyPin;
		KeyPort = KeyIO[i].KeyPort;
	
		//Matrix_Key_IntEnable(FALSE);				//关闭中断
		if(KeyCheck(&u8KeyLine,&u8KeyRow) == 0)	//检测到按键按下 
		{
			s8KeyValue = -2;
		}						//错误按键值
		else
		{
			//s8KeyValue = (VKEY_VAL)KeyTable[u8KeyLine][u8KeyRow];
			s8KeyValue = Key_Get_KeyValue(u8KeyLine,u8KeyRow);
			return s8KeyValue;
		}
	}
	//Matrix_Key_IntEnable(TRUE);					//打开中断
	return s8KeyValue;
}


static uint8_t IsKeyDown(void)
{
		return 1;
}
/*
*********************************************************************************************************
*	函 数 名: bsp_InitKeyVar
*	功能说明: 初始化按键变量
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_InitKeyVar(void)
{
	uint8_t i;

	/* 对按键FIFO读写指针清零 */
	s_tKey.Read = 0;
	s_tKey.Write = 0;

	/* 给每个按键结构体成员变量赋一组缺省值 */
	for (i = 0; i < KEY_COUNT; i++)
	{
		
		s_tBtn[i].IsKeyDownFunc = IsKeyDown;
		s_tBtn[i].LongTime = KEY_LONG_TIME;			/* 长按时间 0 表示不检测长按键事件 */
		
		s_tBtn[i].Count = KEY_FILTER_TIME / 2;		/* 计数器设置为滤波时间的一半 */
		s_tBtn[i].State = 0;							/* 按键缺省状态，0为未按下 */
		//s_tBtn[i].KeyCodeDown = 3 * i + 1;				/* 按键按下的键值代码 */
		//s_tBtn[i].KeyCodeUp   = 3 * i + 2;				/* 按键弹起的键值代码 */
		//s_tBtn[i].KeyCodeLong = 3 * i + 3;				/* 按键被持续按下的键值代码 */
		s_tBtn[i].RepeatSpeed = 30;						/* 按键连发的速度，0表示不支持连发 */ /* 每隔50ms自动发送键值 */

		s_tBtn[i].RepeatCount = 0;						/* 连发计数器 */
		
	}

	/* 如果需要单独更改某个按键的参数，可以在此单独重新赋值 */
	/* 比如，我们希望按键1按下超过1秒后，自动重发相同键值 */
	s_tBtn[KEY_Del].LongTime = 100;
	s_tBtn[KEY_Del].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

	s_tBtn[KEY_ADD].LongTime = 100;
	s_tBtn[KEY_ADD].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

	s_tBtn[KEY_CAR].LongTime = 100;
	s_tBtn[KEY_CAR].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

	s_tBtn[KEY_AXIS].LongTime = 100;
	s_tBtn[KEY_AXIS].RepeatSpeed = 5;	/* 每隔50ms自动发送键值 */

	/* 判断按键按下的函数 */
// 	s_tBtn[0].IsKeyDownFunc = IsKeyDown1;
// 	s_tBtn[1].IsKeyDownFunc = IsKeyDown2;
// 	s_tBtn[2].IsKeyDownFunc = IsKeyDown3;
// 	s_tBtn[3].IsKeyDownFunc = IsKeyDown4;
// 	s_tBtn[4].IsKeyDownFunc = IsKeyDown5;
// 	s_tBtn[5].IsKeyDownFunc = IsKeyDown6;
// 	s_tBtn[6].IsKeyDownFunc = IsKeyDown7;
// 	s_tBtn[7].IsKeyDownFunc = IsKeyDown8;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_DetectKey
*	功能说明: 检测一个按键。非阻塞状态，必须被周期性的调用。
*	形    参：按键结构变量指针
*	返 回 值: 无
*********************************************************************************************************
*/
static void bsp_DetectKey(uint8_t i,uint8_t _keyvalild)
{
	KEY_T *pBtn;

	pBtn = &s_tBtn[i];
	//if (pBtn->IsKeyDownFunc())
	if(_keyvalild)
	{
		if (pBtn->Count < KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if(pBtn->Count < 2 * KEY_FILTER_TIME)
		{
			pBtn->Count++;
		}
		else
		{
			if (pBtn->State == 0)
			{
				pBtn->State = 1;

				/* 发送按钮按下的消息 */
				//bsp_PutKey((uint8_t)(3 * i + 1));
				bsp_PutKey((uint8_t)(i),KEY_DOWN);
			}

			if (pBtn->LongTime > 0)
			{
				if (pBtn->LongCount < pBtn->LongTime)
				{
					/* 发送按钮持续按下的消息 */
					if (++pBtn->LongCount == pBtn->LongTime)
					{
						/* 键值放入按键FIFO */
						//bsp_PutKey((uint8_t)(3 * i + 3));
						bsp_PutKey((uint8_t)(i),KEY_LONG);
					}
				}
				else
				{
					if (pBtn->RepeatSpeed > 0)
					{
						if (++pBtn->RepeatCount >= pBtn->RepeatSpeed)
						{
							pBtn->RepeatCount = 0;
							/* 常按键后，每隔10ms发送1个按键 */
							//bsp_PutKey((uint8_t)(3 * i + 1));
							bsp_PutKey((uint8_t)(i),KEY_DOWN);
						}
					}
				}
			}
		}
	}
	else
	{
		if(pBtn->Count > KEY_FILTER_TIME)
		{
			pBtn->Count = KEY_FILTER_TIME;
		}
		else if(pBtn->Count != 0)
		{
			pBtn->Count--;
		}
		else
		{
			if (pBtn->State == 1)
			{
				pBtn->State = 0;

				/* 发送按钮弹起的消息 */
				//bsp_PutKey((uint8_t)(3 * i + 2));
				bsp_PutKey((uint8_t)(i),KEY_UP);
			}
		}

		pBtn->LongCount = 0;
		pBtn->RepeatCount = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_InitKey
*	功能说明: 初始化按键. 该函数被 bsp_Init() 调用。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_InitKey(void)
{
	bsp_InitKeyVar();		/* 初始化按键变量 */
	bsp_InitKeyHard();		/* 初始化按键硬件 */
	
	//Key_Inif_KeyHard();
	
}

/*
*********************************************************************************************************
*	函 数 名: bsp_PutKey
*	功能说明: 将1个键值压入按键FIFO缓冲区。可用于模拟一个按键。
*	形    参：_KeyCode : 按键代码
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_PutKey(uint8_t _KeyCode, uint8_t _KeyValue)
{
	s_tKey.Buf[s_tKey.Write].KeyCode = _KeyCode;
	s_tKey.Buf[s_tKey.Write].KeyValue = _KeyValue;

	if (++s_tKey.Write  >= KEY_FIFO_SIZE)
	{
		s_tKey.Write = 0;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKey
*	功能说明: 从按键FIFO缓冲区读取一个键值。
*	形    参：无
*	返 回 值: 按键代码
*********************************************************************************************************
*/
uint16_t bsp_GetKey(void)
{
	uint16_t ret = 0;

	if (s_tKey.Read == s_tKey.Write)
	{
		return KEY_ERR;
	}
	else
	{
		ret = ( s_tKey.Buf[s_tKey.Read].KeyCode << 8) + s_tKey.Buf[s_tKey.Read].KeyValue;

		if (++s_tKey.Read >= KEY_FIFO_SIZE)
		{
			s_tKey.Read = 0;
		}
		return ret;
	}
}

/*
*********************************************************************************************************
*	函 数 名: bsp_GetKeyState
*	功能说明: 读取按键的状态
*	形    参：_ucKeyID : 按键ID，从0开始
*	返 回 值: 1 表示按下， 0 表示未按下
*********************************************************************************************************
*/
uint8_t bsp_GetKeyState(VKEY_VAL _ucKeyID)
{
	return s_tBtn[_ucKeyID].State;
}

/*
*********************************************************************************************************
*	函 数 名: bsp_KeyScan
*	功能说明: 扫描所有按键。非阻塞，被systick中断周期性的调用
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
void bsp_KeyScan(void)
{
	int8_t i;

	//////////矩阵键盘读取键盘值的方式 /////////////////
	
	i = bsp_KEY_Read();
	if(i > 0)
	{
		bsp_DetectKey(i,1);
	}
	else
	{
		for(i = 0;i < KEY_COUNT;i ++)
		{
			bsp_DetectKey(i,0);	
		}
	}
}


/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
