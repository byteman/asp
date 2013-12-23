#ifndef _USART_CFG_H_
#define _USART_CFG_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"				

// 接收的模式
#define	RX_PACKET_MODE 0  //按字符超时模式接收
#define	RX_LENGTH_MODE 1	//按定长接收
#define	RX_KEY_MODE    2	//按关键字接收

typedef enum{
	CMD_SET_RX_TIMEOUT ,	//设置接收超时时间 (ms)
	CMD_GET_RX_TIMEOUT 	, //获取接收超时时间 (ms)
	CMD_SET_TX_TIMEOUT ,	//设置发送超时时间 (ms)
	CMD_GET_TX_TIMEOUT 	, //获取发送超时时间 (ms)
	CMD_SET_RX_MODE 	,		//设置接收的模式(0,1,2)
	CMD_GET_RX_MODE 	,		//获取当前接收的模式
	CMD_SET_KEY_CHAR 	,		//关键字接收模式下，设置关键字
	CMD_GET_KEY_CHAR 	,		//关键字接收模式下，获取关键字
	CMD_SET_N_CHAR_TIMEOUT ,		//字符超时模式下，设置N个字符的传输时间为超时时间
	CMD_GET_N_CHAR_TIMEOUT 		, //字符超时模式下，获取N个字符超时
	CMD_FLUSH_INPUT				,	//清空接收缓存中的数据
	CMD_FLUSH_OUTPUT				, //清空发送缓存中的数据
	CMD_GET_INPUT_BUF_SIZE 		//获取接收缓存中的数据字节数
}USART_CTRL_CMD;


#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif
