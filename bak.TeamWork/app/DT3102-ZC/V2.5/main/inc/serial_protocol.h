/******  file description   ******/
/*	created time: 2012-11-10 22:00
 *  author:  
 *	file type:	C/C++ header file
 *	path:		src/wj_test
 *	latest modified:	2012-11-29
 *********************************/

#ifndef __SERIAL_PROTOCOL_H__
#define __SERIAL_PROTOCOL_H__

#include "includes.h"
#include "usart_cfg.h"			//串口配置函数
#include "usart6.h"				//串口6，与SDK通信线程
#include "usart1.h"				
#include "overtime_queue.h"

//sdk协议解析
void SDK_Protocol_Parse(char *buf, int *blen);


/*以下函数为协议数据组包函数,根据协议的数据格式组成数据包, 通过串口发送给SDK*/
/*
 * Function:发送待发送数据队列元素
 * Param: 
 */
void Send_SDK_OverQueueData(OvertimeData *pCar);

/*
 * Function: 发送sdk回应
 * Param:
 *	flag 1:发送长度14,需要回应ret 0:发送长度13,不需要回应ret
 *	ret: 回应结果; cmd: 命令号; id: 流水号
 */
void Send_SDK_Ack(uint8 flag, uint8 cmd, uint16 id, uint8 ret);

#define SDK_OVERTIME_TICK 50 //默认协议是500ms, 安徽超时是2秒
#endif //__SERIAL_PROTOCOL_H__
