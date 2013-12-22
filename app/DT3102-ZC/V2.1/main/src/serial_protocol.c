/******  file description   ******
 *	created time:	2012-11-10 22:00
 *  author:			
 *	file type:		C/C++ source file
 *	path:			src/wj_test
 *	latest modified:2012-12-05
**********************************/
#include "includes.h"
#include "app_rtc.h"
#include "serial_protocol.h"		//本文件的头文件
#include "sdk_serial.h"
#include "overtime_queue.h"			//操作待发送车辆数据队列
#include "task_msg.h"						//消息队列相关函数
#include "app_msg.h"
#include "data_store_module.h"	//车辆数据分区存储模块
#include "device_info.h"
#include "debug_info.h"
#include "stm32f4_crc.h"
#include "crc.h"
#include "usart6.h"
#include "dt3102_io.h"
#include "task_def.h"

#define PROTOCOL_HEAD		0xA3		//协议数据帧头
#define PROTOCOL_VERSION	0x20		//协议版本号

#define MAX_SDATA_LENGTH	  64		//最大数据长度

//协议解析模块用到的静态全局变量
static uint16 RepeatCmdID = 0;						//当前重新采集请求流水号
static uint16 ForceCmdID = 0;						//当前强制采集请求流水号
static uint16 LongCarCmdID = 0;					//当前分段采集请求流水号

//定义的发送数据缓冲区
static char write_buffer[MAX_SDATA_LENGTH];			//组包函数返回的需要通过串口发送给SDK的协议数据

static TaskMsg msg;
//====================================================================================
//与sdk通信数据发送处理

//发送sdk回应
//flag 1:发送长度14,需要回应ret 0:发送长度13,不需要回应ret
void Send_SDK_Ack(uint8 flag, uint8 cmd, uint16 id, uint8 ret)
{
	int status = 0; 
	int16 tmp16 = 0;
	char buf[14] = {0};		//多个线程调用使用
	char len = 0;
	
	buf[0] = PROTOCOL_HEAD;		//帧头
	buf[3] = PROTOCOL_VERSION;	//版本号
	
	buf[4] = id >> 8;			//流水号
	buf[5] = id;

	buf[6] = cmd;				//命令号

	len = 7;
	if(flag == 1)
	{
		buf[len++] = ret;
	}
	status = Get_Device_Status();	 //设备状态
	buf[len++] = status >> 24;
	buf[len++] = status >> 16;
	buf[len++] = status >> 8;
	buf[len++] = status;

	tmp16 = len+2;				//数据包长度										
	buf[1] = tmp16;
	tmp16 = ~tmp16 + 1;			//长度补码
	buf[2] = tmp16;

	tmp16 = CRC16(1, (u8 *)buf, len);	//CRC
	buf[len++] = tmp16 >> 8;
	buf[len++] = tmp16;

	Usart6_WriteData(buf, len);
}

//发送待发送数据队列元素
void Send_SDK_OverQueueData(OvertimeData *pCar)
{
	int status = 0, i = 0; 
	int16 tmp16 = 0;
	u8 tmp8 = 0, slen = 0;

	if(pCar == NULL) return;

	//倒车回应
	if(pCar->cmd == BackCarCmd) 
	{
		Send_SDK_Ack(0, BackCarCmd, pCar->SerialID, 0);
		//置已发送标志
		if(pCar->bSend == 0) pCar->bSend = 1;
		return;
	}

	//修改sdk主动请求命令车辆信息流水号
	if((pCar->cmd != NormalCarCmd) && (pCar->bSend == 0))
	{
		switch(pCar->cmd)
		{
			case RepeatCarCmd:
				pCar->SerialID = RepeatCmdID;
				break;
				
			case ForceCarCmd:
				pCar->SerialID = ForceCmdID;
				break;
				
			case LongCarCmd:
				pCar->SerialID = LongCarCmdID;
				break;
				
			default:
				break;
		}
	}

	//置已发送标志
	if(pCar->bSend == 0) pCar->bSend = 1;
	
	if(pCar->AxleNum < 2) pCar->AxleNum = 2;
	if(pCar->AxleNum > 6) pCar->AxleNum = 6;

	write_buffer[0] = PROTOCOL_HEAD;		//帧头
	write_buffer[3] = PROTOCOL_VERSION;	//版本号
	
	tmp16 = pCar->SerialID;					//流水号
	write_buffer[4] = tmp16 >> 8;
	write_buffer[5] = tmp16;

	write_buffer[6] = pCar->cmd;			//命令号

	tmp16 = pCar->year;						//时间
	write_buffer[7] = tmp16 >> 8;			
	write_buffer[8] = tmp16;				
	write_buffer[9] = pCar->mon;
	write_buffer[10] = pCar->day;
	write_buffer[11] = pCar->hour;
	write_buffer[12] = pCar->min;
	write_buffer[13] = pCar->sec;
	
	write_buffer[14] = pCar->CarStatus;	//车辆状态
	
	status = Get_Device_Status();			//设备状态
	write_buffer[15] = status >> 24;
	write_buffer[16] = status >> 16;
	write_buffer[17] = status >> 8;
	write_buffer[18] = status;
	
	tmp16 = pCar->speed;
	write_buffer[19] = tmp16 >> 8;			//速度
	write_buffer[20] = tmp16;				
	write_buffer[21] = pCar->speeda;		//加速度
	
	write_buffer[22] = pCar->AxleNum;			//轴数
	write_buffer[23] = pCar->AxleGroupNum;	//轴组数

	slen = 24;

	for(i = 0; i < pCar->AxleNum; i++)		//轴重
	{
		write_buffer[slen]   = pCar->AxleWet[i] >> 8;
		write_buffer[slen+1] = pCar->AxleWet[i];
		slen += 2;
	}

	for(i = 0; i < pCar->AxleGroupNum; i++)		//轴组类型
	{
		write_buffer[slen] = pCar->AxleGroupType[i];
		slen++;
	}

	for(i = 0; i < pCar->AxleNum - 1; i++) //轴间距
	{
		tmp16 = pCar->AxleLen[i];
		write_buffer[slen]   = tmp16 >> 8;
		write_buffer[slen+1] = tmp16;
		slen += 2;
	}
	
	tmp8 = slen + 2;						//数据包长度										
	write_buffer[1] = tmp8;
	tmp8 = ~tmp8 + 1;						//长度补码
	write_buffer[2] = tmp8;

	tmp16 = CRC16(1, (u8 *)write_buffer, slen);	//CRC
	write_buffer[slen] = tmp16 >> 8;
	write_buffer[slen+1] = tmp16;
	slen += 2;

	Usart6_WriteData(write_buffer, slen);
}


//检查时间是否有效
static BOOL Check_Date_Valid(CSysTime *time)
{
	if(time)
	{
		if(time->year < 1990 || time->year > 2100) return FALSE;
		if(time->mon < 1 || time->mon > 12) return FALSE;
		if(time->day > 31) return FALSE;
		if(time->hour > 23) return FALSE;
		if(time->min > 59) return FALSE;
		if(time->sec > 59) return FALSE;
		
		return TRUE;
	}
	return FALSE;
}

/*******************************************************
* 协议数据解包函数
* 函数名称：SDK_DataPacket_Parse
* 函数功能：将串口接收到的SDK协议数据解包，得到应用层数据
* 参数：    1、uint8 *data 接收到的串口数据
*           2、uint8 length  接收到的数据长度
* 返回值：  void
*******************************************************/
void SDK_DataPacket_Parse(uint8 *data, uint8 length)
{
	CSysTime proTime = {0};
	uint8 cmd = 0;													//解包得到的命令号
	uint16 id;														//解包得到的流水号
	char isSuccess = 0;										//控制外设是否成功标志
	OvertimeData *pCar = NULL;
	DeviceStatus device = {0};
	static uint16 pay_id = 0;

	id = data[1];						//流水号
	id <<= 8;
	id += data[2];
	cmd = data[3];						//命令号

	memset(&msg,0,sizeof(msg));		//清空消息

	switch(cmd)
	{
		case IORequstCmd:				//IO状态请求
			Send_SDK_Ack(0, cmd, id, 0);
			break;
			
		case NormalCarCmd:				//正常称重数据回应
		case BackCarCmd:				//倒车信息回应
			pCar = OverQueue_Get_Head();									
			if(pCar != NULL) 
			{
				if((pCar->SerialID == id) &&
					(pCar->cmd == cmd) &&
					(pCar->bSend == 1))
				{
					OverQueue_Remove_Head();				
					debug(Debug_Business,"//Recv SDK car ack, SerialID=%d!\r\n", id);
				}
			}
			break;
			
		case RepeatCarCmd:				//重新采集请求
			//保存流水号
			RepeatCmdID = id;
			
			//给业务处理线程投递重新采集请求消息
			msg.msg_id = Msg_Cmd_Repeat;				
			Task_QPost(&BusinessMsgQ, &msg);
			break;
			
		case ForceCarCmd:				//强制采集请求
			//更新当前强制采集请求流水号
			ForceCmdID = id;
			
			//给业务处理线程投递光幕释放消息
			msg.msg_id = Msg_Cmd_Force;					
			Task_QPost(&BusinessMsgQ, &msg);
			break;
			
		case LongCarCmd:				//分段采集请求
			//更新当前分段采集请求流水号stepSampleID
			LongCarCmdID = id;
			
			//给业务处理线程投递分段采集消息
			msg.msg_id = Msg_Cmd_PlusSave;		//分段采集消息
			msg.msg_ctx = data[4];				//分段数
			Task_QPost(&BusinessMsgQ, &msg);
			break;
			
		case PayNoticeCmd:				//首车缴费完成通知
			if(pay_id != id) 
			{
				//流水号不重复, 给业务处理线程投递首车缴费完成通知
				msg.msg_id = Msg_Cmd_PayNotify;				
				Task_QPost(&BusinessMsgQ, &msg);
				pay_id = id;
			}
			else
			{
				debug(Debug_Warning, "//缴费通知流水号重复!\r\n");
			}

			//车辆信息缴费通知上送波形置高
			device.bitValue.carinfo_send_flag = 1;
			Update_CarInfo_SendFlag(device);

			//发送回应
			Send_SDK_Ack(0, cmd, id, 0);
			
			//车辆信息缴费通知上送波形置低
			device.bitValue.carinfo_send_flag = 0;
			Update_CarInfo_SendFlag(device);
			break;
			
		case IOContrlCmd:				//外设控制请求
			//1、控制外设，得到控制结果isSuccess
			switch(data[4])
			{
				case 1:			//控制红绿灯
					if((data[5] & 0x04) && (data[5] & 0x08))	//红绿灯自动控制
					{
							;		//暂时没有实现
					}
					else
					{
						isSuccess = fPeripheral_Control(TrafficSignal,data[5]);
					}
					break;
					
				case 2:			//控制道闸
					if((data[5] & 0x10) && (data[5] & 0x20))	//前道闸自动控制
					{
						;		//暂时没有实现
					}
					else
					{
						isSuccess = fPeripheral_Control(BarrierGate,data[5] >> 1);
					}
					break;
					
				case 3:								
					break;
					
				default: break;
			}
			//根据控制结果，组包
			Send_SDK_Ack(1, cmd, id, isSuccess);
			break;

		case TimeSetCmd:				//SDK时间同步命令
			proTime.year = (data[4] << 8) + data[5];
			proTime.mon = data[6];
			proTime.day = data[7];
			proTime.hour = data[8];
			proTime.min = data[9];
			proTime.sec = data[10];
			if(Check_Date_Valid(&proTime))		
			{
				Set_System_Time(&proTime);
			}

			Send_SDK_Ack(0, cmd, id, 0);
			break;		
			
		default:
			break;											
	}
}

#define COMHEADLEN 9

void SDK_Protocol_Parse(char *buf, int *blen)
{
	char dlen = 0, inv_dlen = 0;
	int i, len = *blen;

	if(buf == NULL) return;
	if(len < COMHEADLEN) return;
	
	//分包
	while(len >= COMHEADLEN) 
	{
		//查找报文头
		for(i = 0; i < len; i++)
		{
			if(buf[i] == PROTOCOL_HEAD)
				break;
		}
		//丢掉报文头之前的数据
		if(i > 0)
		{
			len -= i;
			memmove(buf, &buf[i], len);
		}
		if(len < COMHEADLEN) break;
	
		//长度校验
		dlen = buf[1];
		inv_dlen = buf[2];
		inv_dlen = ~inv_dlen + 1;
		if(dlen != inv_dlen)
		{
			len -= 1;		//丢掉帧头
			memmove(buf, &buf[1], len);
			continue;
		}
		//数据长度不够,继续接收
		if(len < dlen) break;
	
		//数据包有效性检查
		if(((dlen < COMHEADLEN) || (dlen > 64)) ||	//数据长度判断
			(buf[3] != PROTOCOL_VERSION) ||			//协议版本号检查
			(CRC16(1, buf, dlen) != 0))			//crc检验
		{
			len -= 7;		//丢掉命令号之前的数据
			memmove(buf, &buf[7], len);
			continue;
		}
		
		//命令解析
		debug(Debug_Warning, "//Uart6 received sdk cmd:%d\r\n", buf[6]);
		SDK_DataPacket_Parse(&buf[3], dlen - 3);
	
		//丢掉已解析的命令
		len -= dlen;
		memmove(buf, &buf[dlen], len);
	}//end while(len >= COMHEADLEN)	

	*blen = len;
}

