/*
 * Brief: 通过shell命令-update,串口升级应用程序,传输协议ymodem;  
 * 
 * History: 1. 2013.03.22 参考bootload程序完成ymodem协议解析与升级程序移植;
 *		   2. 2013.03.26 优化协议解析,增加对SecureCRT传输的支持;  river
 */
#include "includes.h"
#include "system_init.h"
#include "stm32f4_crc.h"
#include "crc.h"
#include "net_business_thread.h"
#include "ymodem_update.h"
#include "softdog.h"

#define MODEM_SOH 		0x01 		//128字节开始
#define MODEM_STX 		0x02		//1028字节开始
#define MODEM_EOT 		0x04		//文件传输结束
#define MODEM_CAN 		0x18 		//取消传输
#define MODEM_C 		"C"			//大写字母C 0x43 
#define FileDataSUB		0x1A		//文件数据补足数据

static char MODEM_ACK = 0x06;		//确认应答
static char MODEM_NAK = 0x15;		//出现错误

static char *prompt0 = "擦除固件flash区域,请等待...\r\n";
static char *prompt1 = "擦除固件flash失败\r\n";
static char *prompt2 = "擦除固件flash成功\r\n";
static char *prompt3 = "等待接收升级文件, 60秒超时...\r\n";
static char *prompt4 = "60秒没有操作, 退出\r\n";
static char *prompt5 = "文件传输完成!\r\n";
static char *prompt6 = "写更新标志失败!\r\n";
static char *prompt7 = "写入flash失败!\r\n";

static const u16 XYMODEM_CRC_TAB[256] =	   //YMODEM效验数据表
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41,
    0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70,
    0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f,
    0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e,
    0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d,
    0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c,
    0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a,
    0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9,
    0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8,
    0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
};

#define YMODEM_RX_CRC(ch, crc) ((XYMODEM_CRC_TAB[((crc >> 8) & 0xff)] ^ (crc << 8) ^ ch) & 0xffff)		//接收CRC效验
#define YMODEM_TX_CRC(ch, crc) (XYMODEM_CRC_TAB[((crc >> 8) ^ ch) & 0xff] ^ (crc << 8))					//发送CRC效验

/****************************************************************************
* 名	称：u16 Ymodem_CRC16(u8* data, u32 len)
* 入口参数：u8* data	参与效验的数据
		   u32 len		参与效验的数据个数
* 出口参数：u16 CRC	效验值
* 说	明：接收方只对数据进行TX效验
****************************************************************************/ 
static u16 Ymodem_CRC16(u8* data, u32 len)
{
	u16 crc = 0x0000;
	u32 i;
	for (i = 0; i < len; i++)
	{	
		crc = XYMODEM_CRC_TAB[((crc >> 8) ^ data[i]) & 0xff] ^ (crc << 8);
	}
	
	return crc;
}

//=================================================
//update

static unsigned char UpdateBuffer[1024] = {0};
static uint32 FileCRC = 0;

static int Recv_Packet(uint32 len, uint8 sid, uint8 code)
{
	int n = 0, rlen = 0;
	uint16 crc = 0;
	char crc0, crc1;
	char key = 0;

	//判断接收序号
	rlen = Usart1_ReadData(&key, 1);
	if(rlen == 0) return 3;
	if(key != sid) return 1;

	//判断序号补码
	rlen = Usart1_ReadData(&key, 1);
	if(rlen == 0) return 3;
	if(key != code) return 2;

	//接收定长数据包
	for(n = 0; n < len; n++)
	{
		rlen = Usart1_ReadData(&key, 1);
		if(rlen == 0) return 3;
		UpdateBuffer[n] = key;
	}

	//接收crc校验
	rlen = Usart1_ReadData(&key, 1);
	if(rlen == 0) return 3;
	crc0 = key;

	rlen = Usart1_ReadData(&key, 1);
	if(rlen == 0) return 3;
	crc1 = key;
	
	//crc校验判断
	crc = Ymodem_CRC16(UpdateBuffer, len);
	if((((u8)(crc >> 8)) != crc0) && 
		(((u8)crc) != crc1))
	{
		return 4;
	}
	
	return 0;
}

static int Ymodem_Update(void)
{
	char state = 0, key; 
	int len = 0, i, ret;
	uint8 inv_code = 0xff, serial_id = 0;
	int rfilelen, filelen = 0, addr = 0;
	BOOL fail_flag = FALSE, tool_flag = FALSE;
	BOOL cflag = FALSE;

	Usart1_Set_OvertimeTick(50);	//设置串口接收超时时间为50ms

	while(1)
	{
		switch(state)
		{
			case 0:
				//接收SOH信号后的128个字节
				ret = Recv_Packet(128, 0, 0xff);
				if(ret != 0) return ret;
				
				//查找文件名
				for(i = 0; i < 128; i++)
				{
					if(UpdateBuffer[i] == 0) break;
				}
				
				//获取文件长度
				rfilelen = atoi(&UpdateBuffer[i+1]);
				if(rfilelen > 0) 
				{	
					tool_flag = TRUE;		//超级终端传输
					filelen = rfilelen;
				}
				//硬件crc清零
				CRC_ResetDR();
				FileCRC = 0;
				
				//回应ack并且开启传输升级文件
				Usart1_WriteData(&MODEM_ACK, 1);
				Usart1_WriteData(MODEM_C, 1);
				state = 1;
				break;

			case 1:
				inv_code -= 1;
				serial_id += 1;
			
				//最后一包数据
				if(cflag)
				{
					inv_code = 0xff;
					serial_id = 0;
				}
				
				len = Usart1_ReadData(&key, 1);
				if(len == 0) return 3;

				switch(key)
				{
					case MODEM_CAN:
						Usart1_WriteData("\r\n取消传输\r\n", 12);
						return 5;

					case MODEM_EOT:
						Usart1_WriteData(&MODEM_ACK, 1);
						Usart1_WriteData(MODEM_C, 1);
						cflag = TRUE;
						continue;

					case MODEM_SOH:
						len = 128;
						break;

					case MODEM_STX:
						len = 1024;
						break;

					default:
						len = 0;
						break;
				}

				//接收定长数据包
				ret = Recv_Packet(len, serial_id, inv_code);
				if(ret != 0) return ret; 
				
				if(cflag)
				{
					//传输完成
					state = 2;
					break;
				}

				if(len > 0)
				{
					if(!tool_flag)
					{
						//查看是否是最后一包数据
						//匹配secureCRT
						i = len - 1;
						if((UpdateBuffer[i] == FileDataSUB) 	&& 
							(UpdateBuffer[i-1] == FileDataSUB)	&&
							(UpdateBuffer[i-2] == FileDataSUB)	&& 
							(UpdateBuffer[i-3] == FileDataSUB))
						{
							for(i = 0; i < len; i++)
							{
								if((UpdateBuffer[i] == FileDataSUB) 	&& 
									(UpdateBuffer[i+1] == FileDataSUB)	&&
									(UpdateBuffer[i+2] == FileDataSUB)	&& 
									(UpdateBuffer[i+3] == FileDataSUB))
									break;
							}
							len = i;
						}
						filelen += len;
					}
					else
					{
						//匹配超级终端
						if(rfilelen < len) 
						{
							len = rfilelen;
						}
						else
						{
							rfilelen -= len;
						}
					}

					//写入flash
					if(Flash_APPBak_Store(addr, UpdateBuffer, len) == 0) fail_flag = TRUE;
					addr += len;	

					//计算升级文件crc校验值					
					FileCRC = CRC32H_Chack8(0, UpdateBuffer, len);
				}	
				Usart1_WriteData(&MODEM_ACK, 1);
				break;

			case 2:
				//传输结束
				Usart1_WriteData(&MODEM_ACK, 1);
			
				SysTimeDly(100);
				Usart1_WriteData(prompt5, strlen(prompt5));

				//更新升级标志
				if(!fail_flag)
				{
					if(Flash_APPInfo_Store(FileCRC, filelen, PCBOOT) == 0)
					{
						Usart1_WriteData(prompt6, strlen(prompt6));
						return 1;
					}
					Flash_UPFlag();
					sprintf(UpdateBuffer, ">>升级文件CRC: %x,长度: %d\r\n", FileCRC, filelen);
					Usart1_WriteData(UpdateBuffer, strlen(UpdateBuffer));
					return 0;
				}

				Usart1_WriteData(prompt7, strlen(prompt7));
				return 1;

			default:
				break;
		}//end switch(state)
	}//end while(1)
}

int Do_Serial_Update(char *arg)
{
	char key = 0;
	int ecnt = 0;
	int ret = 0;

	//开始升级, 关闭软件看门狗监测
	Set_SoftDog_Changer(TRUE);
	while(1)
	{
		Usart1_WriteData(prompt0, strlen(prompt0));
		//擦除固件flash区域
		if(Erase_AppFlash() != 0)
		{
			Usart1_WriteData(prompt1, strlen(prompt1));
			ret = 1;
			break;
		}
		Usart1_WriteData(prompt2, strlen(prompt2));
		Usart1_WriteData(prompt3, strlen(prompt3));

		Usart1_Clear_Buffer();
		//等待60s
		while(1)
		{
			ecnt++;
			if(ecnt >= 60)
			{
				Usart1_WriteData("\r\n", 2);
				Usart1_WriteData(prompt4, strlen(prompt4));
				return 1;
			}
			
			//开启传输
			Usart1_WriteData(MODEM_C, 1);
			
			if(Usart1_ReadData(&key, 1) > 0) 
			{
				//ctrl+c取消升级
				if(key == 0x03)
				{
					Usart1_WriteData("\r\n取消升级\r\n", 12);
					return 1;
				}
				
				//接收到MODEM_SOH
				if(key == MODEM_SOH) break;
				
				Usart1_Clear_Buffer();
			}
		}

		ret = Ymodem_Update(); 
		break;
	}
	
	key = 0x03;
	if(ret != 0) Usart1_WriteData(&key, 1);

	Usart1_Set_OvertimeTick(1000);
	Set_SoftDog_Changer(FALSE);

	return ret;
}

