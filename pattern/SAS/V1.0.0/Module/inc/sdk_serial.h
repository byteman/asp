#ifndef SDK_SERIAL_H
#define SDK_SERIAL_H

#include "includes.h"
#include "usart_cfg.h"			//串口配置函数
#include "usart6.h"				//串口6，与SDK通信线程

/**********************************************************************/
//串口6初始化函数
void Com_Port6_Init(void);

//串口6读、写函数
s32 Usart6_ReadData(char *buf,u32 len);

s32 Usart6_WriteData(char *buf, u32 len);

s32 Usart6_Get_BufferLen(u32 param);

#endif //__SERIAL_PROTOCOL_H__

