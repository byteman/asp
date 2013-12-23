#include "includes.h"
#include "serial_protocol.h"
#include "sdk_serial.h"
#include "stm32f4_crc.h"
#include "debug_info.h"
#include "task_msg.h"
#include "app_msg.h"
#include "dt3102_io.h"
#include "crc.h"
#include "overtime_queue.h"

#define COMRECVLEN	128

static char ComRecvBuf[COMRECVLEN] = {0};
static int ComKeepLen = 0;

void SerialRecv_Thread(void *arg)
{
	uint32 Reallen = 0, len = 0;

	Com_Port6_Init();

	//延时1秒初始化OverQueue, 方便随机数生成
	SysTimeDly(100);
	OverQueue_Init();	

	while(1) 
	{
		//读取串口缓冲区数据长度
		Usart6_Get_BufferLen((u32)&len);
		if(len > COMRECVLEN) len = COMRECVLEN;
		
		//串口没有接收到数据, 延时120ms
		if(len == 0) 
		{
			SysTimeDly(12);
			continue;
		}
		
		//长度越界保护
		if(ComKeepLen < 0) ComKeepLen = 0;
		
		//读数据
		if((len + ComKeepLen) > COMRECVLEN) 
		{
			Reallen = Usart6_ReadData(&ComRecvBuf[0],len);
			ComKeepLen = Reallen;
		}
		else
		{
			Reallen = Usart6_ReadData(&ComRecvBuf[ComKeepLen],len);
			ComKeepLen += Reallen;
		}

		SDK_Protocol_Parse(ComRecvBuf, &ComKeepLen);	
	}
}
