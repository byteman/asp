#include "ymodem_init.h"
#include "static_init.h"	//ASCII 转 long		固件存放地址
#include "String.h"				//memset
#include "Flash_init.h"		//FLASH_DataSpace_Erase
#include "crc_init.h"			//CRC32H_Chack8
#include "ConnectCfg.h"		//通讯端口		//ComPort项				//与菜单显示为同一端口
#include "printf_init.h"	//DebugPf

//ymodem协议操作串口
#define YmodemUSART_SendByte  			ComSendByte								//Ymodem协议发送串口
#define YmodemUSART_GetByte_Wait  	ComGetByte_Wait						//Ymodem协议接收串口 等待
#define YmodemUSART_RX_Buffer_Clear	BootLoaderUsart_RX_Clear	//清空Ymodem协议接收缓冲区

#define YmodemGetFileWait		60					//等待接收文件	秒

/****************************************************************************
* Ymodem协议说明
最快使用波特率57600  //115200太快，传输会出错

由接收方主动发起通讯
接收方					发送方
						0-C
C->						
SHO-0
						<-SHO
						<-序号00 补码FF 文件名+文件长度128 效验1 效验2
						0-ACK
ACK->
						0-C
C->
SOH/STX/EOT-0
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						1:<-SOH/STX 序号1 补码FE 128/1024数据 效验1 效验2		//SOH=128 STX=1024 数据不足的用0x1A补
						0-ACK
ACK->					
						重复 1 后续数据包 补码累减至0x00后复位到0xFF继续累减
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
						2:<-EOT		结束
						0-ACK
ACK->
						0-C
C->
						<-序号00 补码FF 128个0x00 效验0 效验0	全0数据包
						0-ACK
ACK->
****************************************************************************/

#define MODEM_SOH 		0x01 		//128字节开始
#define MODEM_STX 		0x02		//1028字节开始
#define MODEM_EOT 		0x04		//文件传输结束
#define MODEM_ACK 		0x06		//确认应答
#define MODEM_NAK 		0x15		//出现错误
#define MODEM_CAN 		0x18 		//取消传输
#define MODEM_C 			0x43 		//大写字母C
#define FileMessageCode	0xFF	//文件信息补码
#define FileDataCode	0xFE		//文件数据补码
#define FileDataSUB		0x1A		//文件数据补足数据

#define YMODEM_RX_CRC(ch, crc) ((XYMODEM_CRC_TAB[((crc >> 8) & 0xff)] ^ (crc << 8) ^ ch) & 0xffff)		//接收CRC效验
#define YMODEM_TX_CRC(ch, crc) (XYMODEM_CRC_TAB[((crc >> 8) ^ ch) & 0xff] ^ (crc << 8))								//发送CRC效验

#define WaitDataTime 5600000					//等待数据时间		500MS 适应9600 - 115200波特率范围

const u16 XYMODEM_CRC_TAB[256] =	   //YMODEM效验数据表
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

/****************************************************************************
* 名	称：u16 Ymodem_CRC16(u8* data, const u32 len)
* 功	能：开启YMODEM传输
* 入口参数：u8* data	参与效验的数据
			const u32 len		参与效验的数据个数
* 出口参数：u16 CRC	效验值
* 说	明：接收方只对数据进行TX效验
****************************************************************************/ 
u16 Ymodem_CRC16(u8* data, const u32 len)
{
	u16 crc = 0x0000;
	u32 i;
	for (i = 0; i < len; i++)
		crc = YMODEM_TX_CRC(data[i], crc);
	return crc;
}

/****************************************************************************
* 名	称：u8 Ymodem_FileProperty(u8* num_temp,u8* Data_temp,u8* chack_temp,const u8 code_temp)
* 功	能：开启YMODEM传输
* 入口参数：u8* num_temp	数据包序号
			u8* Data_temp	接收到的数据 128/1024
			u8* chack_temp	2位效验码
			const u8 code_temp	累减计数的补码
* 出口参数：u8 	1	接收文件信息成功
				0 	接收文件信息错误
				2	用户退出
* 说	明：无
****************************************************************************/
u8 Ymodem_FileProperty(u8* num_temp,u8* ymessage_temp,u8* chack_temp,const u8 code_temp)
{
	u8 get_temp = 0x00;					//接收到的数据
	u8 flag = 0;							//接收标志
	u8 i = 0;
	
	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	//接收数据，延迟10ms
	if(flag == 0)
		return 0;				//未接收到数据

	if(get_temp == 'B' || get_temp == 'b')		//接收到的数据为用户退出命令
		return 2;				//接收数据错误

	if(get_temp != MODEM_SOH)		//接收到的数据不是MODEM_SOH
		return 0;				//接收数据错误
	
	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);		//序号
	if(flag == 0)
		return 0;
	*num_temp = get_temp;	

	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //补码
	if(flag == 0)
		return 0;		  
	if(get_temp != 0xFF)		//0xFF
		return 0;

	while(1)				//128个数据
	{
		flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);		//128个数据
		if(flag == 0)
			return 0;
		ymessage_temp[i] = get_temp;	 
		
		i++;					//数据累加
		if(i >= 128)		
			break;
	}

	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);		  //效验值1
	if(flag == 0)
		return 0;
	chack_temp[0] = get_temp;	  

	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);		  //效验值2
	if(flag == 0)
		return 0;

	chack_temp[1] = get_temp;	  

	return 1;				//接收完成
}

/****************************************************************************
* 名	称：u8 Ymodem_Com(const u32 S_Time,u8* num_temp,u8* Data_temp,u8* chack_temp,const u8 code_temp)
* 功	能：开启YMODEM传输
* 入口参数：const u32 S_Time	 秒 开启传输后等待发送方发送数据的时间
			u8* num_temp	数据包序号
			u8* Data_temp	接收到的数据 128/1024
			u8* chack_temp	2位效验码
			const u8 code_temp	累减计数的补码
* 出口参数：u8 	1	通讯开始
				0 	通讯失败
				2	用户退出命令
* 说	明：无
****************************************************************************/
u8 Ymodem_Com(const u32 S_Time,u8* num_temp,u8* ymessage_temp,u8* chack_temp,const u8 code_temp)
{
	u8 flag = 0;															//接收标志
			
	u8 Time = 0;															//秒延迟第一次获取时间计数器
	
	Time = (u8)(GetTimeMSecCnt() / 1000);			//第一次获取时间
	
	while(1)																	//进入等待(SOH)状态	2次约为1秒 N秒内发送方找文件并发送
	{
		TimDelayMS(1000);												//延迟
		YmodemUSART_SendByte(MODEM_C);					//发一个大写字母C开启传输
		
		flag = Ymodem_FileProperty(num_temp,ymessage_temp,chack_temp,code_temp);	//开启YMODEM传输
		if(flag == 1)														//接收数据成功
			return 1;															//通讯开启成功，并接收到文件信息

		if(flag == 2)														//用户退出命令
			return 2;															//外面一层显示

		YmodemUSART_RX_Buffer_Clear();					//没有接收到SOH则清空接收缓冲区 主要是对终端的取消操作遗留数据进行清除

		if((u8)(GetTimeMSecCnt() / 1000) >= Time + S_Time)					//当前时间超过设定的时间rtc精确
			return 0;															//超时退出	外面一层进行显示
	}		
}
/****************************************************************************
* 名	称：u8 Ymodem_FilePropertyCRC(u8* Data_CRC,u8* chack_temp)
* 功	能：文件属性CRC数据效验
* 入口参数：u8* Data_CRC	需要效验的数据
			u8* chack_temp		获取得来的CRC效验值
* 出口参数：u8 	1	效验正确
				0 	效验错误
* 说	明：暂未使用效验 保持正确		
****************************************************************************/ 		
u8 Ymodem_FilePropertyCRC(u8* Data_CRC,u8* chack_temp)
{
	u16 CRC_temp = 0;		//计算出来的效验值
	CRC_temp = Ymodem_CRC16(Data_CRC,128);	  //接收方只对数据进行TX效验

	if((((u8)(CRC_temp >> 8)) != chack_temp[0]) && (((u8)CRC_temp) != chack_temp[1]))	
		return 0;	//效验失败
	else			//验证成功后发送后续通信信号
		return 1;	//效验成功
}

/****************************************************************************
* 名	称：u8 Ymodem_Data(u8* num_temp,u8* Data_temp,u8* chack_temp,const u8 code_temp)
* 功	能：文件数据接收
* 入口参数：u8* num_temp			数据块序号
			u8* Data_temp			数据
			u8* chack_temp			效验值
			const u8 code_temp			累减补码	
* 出口参数：u8 	0 	接收失败
				1	128数据接收完成
				2	1024数据接收完成
				3	文件接收完成
* 说	明：xp的超级终端发送时，文件小于128字节，其用128发送，文件大于128字节，其自动用1024发送
****************************************************************************/
u8 Ymodem_Data(u8* num_temp,u8* Data_temp,u8* chack_temp,const u8 code_temp)
{
	u8 flag = 0;					//接收标志
	u8 get_temp = 0;			//接收到的数据
	u32 i = 0;						//累加器(最大1024)

	flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	//接收数据，延迟10ms
	if(flag == 0)			//数据接收成功,并且接收到的数据为MODEM_SOH
		return 0;

	switch(get_temp)
	{
		case MODEM_SOH:		//128位数据传输
		{
			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //序号
			if(flag == 0)			
				return 0;
			*num_temp = get_temp;	

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //补码
			if(flag == 0)			
				return 0;
			if(get_temp != code_temp)	//数据补码错误
				return 0;

			i = 0;
			while(1)			//128个数据
			{
				flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	//128个数据
				if(flag == 0)
					return 0;
				Data_temp[i] = get_temp;	 
				
				i++;					//数据累加
				if(i >= 128)		
					break;
			}

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	 //效验值1
			if(flag == 0)
				return 0;
			chack_temp[0] = get_temp;	  

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	  //效验值2
			if(flag == 0)
				return 0;
			chack_temp[1] = get_temp;	  

			return 1;				//128 接收完成
		}
		case MODEM_STX:		//1024位数据传输
		{
			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //序号
			if(flag == 0)			
				return 0;
			*num_temp = get_temp;	

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //补码
			if(flag == 0)			
				return 0;
			if(get_temp != code_temp)	//数据错误
				return 0;

			i = 0;
			while(1)			//1024个数据
			{
				flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	 //1024个数据
				if(flag == 0)
					return 0;
				Data_temp[i] = get_temp;	 
				
				i++;					//数据累加
				if(i >= 1024)		
					break;
			}

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //效验值1
			if(flag == 0)
				return 0;
			chack_temp[0] = get_temp;	  

			flag = YmodemUSART_GetByte_Wait(&get_temp,WaitDataTime);	   //效验值2
			if(flag == 0)
				return 0;
			chack_temp[1] = get_temp;	  

			return 2;				//1024 接收完成
		} 
		case MODEM_EOT:		//文件传输结束
		{
			YmodemUSART_SendByte(MODEM_ACK);		//接收完成 发送确认信号
			YmodemUSART_SendByte(MODEM_C);			//再发送一次C开启一次数据接收  (接收全0数据包)
			return 3;				//文件接收完成
		}
		default :	//其它则返回错误
			return 0; 
	}
}

/****************************************************************************
* 名	称：u8 Ymodem_FileDataCRC(const u8 Byenumflag,u8* Data_CRC,u8* chack_temp)
* 功	能：文件数据CRC数据效验
* 入口参数：const u8 Byenumflag	位标志 1=128 2=1024
			u8* Data_CRC	需要效验的数据
			u8* chack_temp	获取得来的CRC效验值
* 出口参数：u8 	1	效验正确
				0 	效验错误
* 说	明：暂未使用效验 保持正确		
****************************************************************************/
u8 Ymodem_FileDataCRC(const u8 Byenumflag,u8* Data_CRC,u8* chack_temp)
{
	u16 CRC_temp=0;		//计算出来的效验值
	
	if(Byenumflag == 1)	//128位数据效验
		CRC_temp = Ymodem_CRC16(Data_CRC,128);	  //接收方只对数据进行TX效验
	else				//1024位数据效验
		CRC_temp = Ymodem_CRC16(Data_CRC,1024);	  //接收方只对数据进行TX效验

	if((((u8)(CRC_temp >> 8)) != chack_temp[0]) && (((u8)CRC_temp) != chack_temp[1]))
		return 0;	//效验失败
	else			//验证成功后发送后续通信信号
		return 1;	//效验成功
}


//对第一包文件信息数据进行解析后存放，用于以后显示用
static u8 FileName[128] = {0};	  	//名字
static u8 FileNameSize = 0;		  		//名字长度
static u32 FileSize = 0;						//转码后的文件大小
/****************************************************************************
* 名	称：u8* GetFlieNameDataAddr(void)
* 功	能：获取文件名存放地址
* 入口参数：无
* 出口参数：u8* 文件名存放地址
* 说	明：无
****************************************************************************/
u8* GetFlieNameDataAddr(void)
{
	return FileName;
}

/****************************************************************************
* 名	称：u8 GetFlieNameLen(void)
* 功	能：获取文件名长度
* 入口参数：无
* 出口参数：u8 文件名长度
* 说	明：无
****************************************************************************/
u8 GetFlieNameLen(void)
{
	return FileNameSize;
}

/****************************************************************************
* 名	称：u32 GetFlieSize(void)
* 功	能：获取文件大小
* 入口参数：无
* 出口参数：u32 文件大小
* 说	明：无
****************************************************************************/
u32 GetFlieSize(void)
{
	return FileSize;
}

/****************************************************************************
* 名	称：u8 Ymodem_FileDecode(u8* Message_temp,u8* Data_temp,u8* FileName_temp,u8* FileNameLen_temp,u32* FileSizeLen)
* 功	能：文件数据解码
* 入口参数：u8* Message_temp	获取的文件信息原始数据	解码后为文件信息
			u8* FileName_temp	解析出来的文件名
			u8 FileNameLen_temp	解析出来的文件名长度
			u32* FileSizeLen	解析出来的文件大小长度
* 出口参数：u8 	1	解析成功
				0 	解析错误
* 说	明：无		
****************************************************************************/
u8 Ymodem_FileDecode(u8* Message_temp,u8* FileName_temp,u8* FileNameLen_temp,u32* FileSizeLen)
{
	u32 i = 0;	//累加器
	u8 FileSize_temp[10] = {0};	//文件大小 ASCII码	   10位=1G
	u32 FileSizeLen_temp = 0;		//文件大小 ASCII码个数

	while(1)	//找文件名
	{
		if(Message_temp[i] == 0x00)	 		//文件名后面的NUL
		{
			*FileNameLen_temp = i;		  	//获取文件名长度
			break;
		}
		i++;
	}
	memcpy(FileName_temp,Message_temp,*FileNameLen_temp);	 //获得文件名

	i = 1;		//累加器复位
	while(1)	//找文件大小
	{
		if(Message_temp[i + *FileNameLen_temp] == 0x00)	 		//文件大小后面的NUL
		{
			FileSizeLen_temp = i - 1;		  	//获取文件大小字数
			break;
		}
		i++;
	}
	memcpy(FileSize_temp,Message_temp + *FileNameLen_temp + 1,FileSizeLen_temp);	 //获得文件大小
	*FileSizeLen = ASCIItoNumber(FileSize_temp,FileSizeLen_temp);				 //文件大小ASCII转数字型

	return 1;	//效验成功
}

/****************************************************************************
* 名	称：u8 Ymodem_START(const u8 Option)
* 功	能：YMODEM 接收数据		接收空间最小128字节 最大1024
* 入口参数：const u8 Option		1/2/3选项
* 出口参数：u8 接收是否成功	0失败	1成功
* 说	明：根据选项1/2/3的不同，保存的地址也不同
****************************************************************************/
static u32 FileCRC = 0;							//整个固件的CRC效验	每包获取时用以对包进行调试检测
static s32 ExcessSize = 0;					//计算剩余文件长度
static u32 num = 1;									//升级包序号
static u8 FileMessage[128] = {0};		//文件信息空间 	   //小于128的按128空间		//用户解析文件名，文件名长度，文件长度用
static u8 FileData[1024] = {0};  		//数据接收空间		128接收和1024接收均用此缓冲区
static u32 CtrlAddress = 0;					//操作地址	根据进入项不同而指向不同的地址区(App/Bak/Flag)
u8 Ymodem_START(const u8 Option)
{
	u8 serial_number = 0;							//序号
	u8 chack[2] = {0};								//效验值
	u8 flag = 0;											//标志
	u8 code = 0xFF;										//补码，每接收一次数据-1
	u32 newbootflag = UPBOOTFlag;			//更新标志
	u32 FileUpSrc = PCBOOT;						//文件来源(Ymodem接收均为PCBOOT来源)
	
	switch(Option)
	{
		case 1:		
		{
			CtrlAddress = AppStartAddressBak;																								//下载应用程序到备份区
			DebugPf(0,"应用程序备份空间擦除中...\r\n");
			if(FLASH_DataSpace_Erase(AppStartAddressBak,AppBakSpaceSize) == 0) 					//擦除备份区
			{
				DebugPf(0,"应用程序备份空间擦除失败!\r\n");
				return 0;
			}
			else
				DebugPf(0,"应用程序备份空间擦除完成!\r\n");
			break;
		}
		case 2:
		{
			CtrlAddress = GBFontStartAddress;																								//下载GB字库到GB字库区
			DebugPf(0,"GB字库空间擦除中...\r\n");
			if(FLASH_DataSpace_Erase(GBFontStartAddress,GBFontSpaceSize) == 0) 					//擦除GB字库区
			{
				DebugPf(0,"GB字库空间擦除失败!\r\n");
				return 0;
			}
			else
				DebugPf(0,"GB字库空间擦除完成!\r\n");
			break;
		}
		case 3:
		{
			CtrlAddress = ASCIIFontStartAddress;																						//下载ASCII字库到ASCII字库区
			DebugPf(0,"ASCII字库空间擦除中...\r\n");
			if(FLASH_DataSpace_Erase(ASCIIFontStartAddress,ASCIIFontSpaceSize) == 0) 		//擦除ASCII字库区
			{
				DebugPf(0,"ASCII字库空间擦除失败!\r\n");
				return 0;
			}
			else
				DebugPf(0,"ASCII字库空间擦除完成!\r\n");
			break;
		}
		case 4:
		{
			CtrlAddress = AuxiliaryAppStartAddress;																						//下载辅助应用程序到辅助应用程序区
			DebugPf(0,"辅助应用程序空间擦除中...\r\n");
			if(FLASH_DataSpace_Erase(AuxiliaryAppStartAddress,AuxiliaryAppSpaceSize) == 0) 		//擦除辅助应用程序区
			{
				DebugPf(0,"辅助应用程序空间擦除失败!\r\n");
				return 0;
			}
			else
				DebugPf(0,"辅助应用程序擦除完成!\r\n");
			break;
		}
		default:
			return 0;																//错误选项
	}

	flag = Ymodem_Com((u32)YmodemGetFileWait,&serial_number,FileData,chack,code);	  //开启通讯，通讯等待为15秒	   //文件信息获得	//Data_t   从头开始存放数据	
	if(flag == 0)		 									//15秒内未接收到发送方数据应答，失败
	{
		DebugPf(0,"\r\n****************************\r\n");
		DebugPf(0,"文件接收超时退出!\r\n");		   
		DebugPf(0,"****************************\r\n\r\n");
		YmodemUSART_GetByte_Wait(&flag,168000000);	   //显示停留 约15秒
		return 0;		//超时失败
	}

	if(flag == 2)
	{
		DebugPf(0,"\r\n****************************\r\n");
		DebugPf(0,"退出Ymodem!\r\n");
		DebugPf(0,"****************************\r\n\r\n");
		YmodemUSART_GetByte_Wait(&flag,14000000);	   //显示停留
		return 0;		//用户退出
	}

	if(Ymodem_FilePropertyCRC(FileData,chack) == 0)		//文件信息数据效验
	{
		DebugPf(0,"****************************\r\n");
		DebugPf(0,"文件名CRC效验错误!\r\n");
		DebugPf(0,"****************************\r\n\r\n");
		YmodemUSART_GetByte_Wait(&flag,14000000);	   //显示停留
		return 0;
	}

	memcpy(FileMessage,FileData,128);		//128文件信息转存	   0-128
	memset(FileName,0,sizeof(FileName));		//清空显示名字区
	FileNameSize = 0;	//文件名大小清除
	FileSize = 0;	//文件大小清除

	flag = Ymodem_FileDecode(FileMessage,FileName,&FileNameSize,&FileSize);	  //解析数据得到文件名和文件长度
	if(flag == 1)	  //文件信息解析成功
	{
		ExcessSize = FileSize; 						//剩余文件长度初始化
	  YmodemUSART_SendByte(MODEM_ACK);	//接收完成 发送确认信号 
		YmodemUSART_SendByte(MODEM_C);		//发起始信号，正式启动数据传输
	}

	while(1)
	{
		if(code == 0x00)
			code = 0xFF;		//补码累减完后复位
		else
			code = code - 1;	//每成功接收一次数据补码-1
		
		flag = Ymodem_Data(&serial_number,FileData,chack,code);		//一个文件数据包获取
		switch(flag)
		{
			case 1:				//接收数据包128个数据
			{
				if(Ymodem_FileDataCRC(1,FileData,chack) == 0)
					return 0;

				if(ExcessSize >= 128)
				{
					Flash_DataWrite(CtrlAddress + (FileSize - ExcessSize),128,FileData);					//获得一个包写一个包	偏移上次剩余地址
					FileCRC = CRC32H_Chack8(0,FileData,128);		//获得的文件校验值
				}
				else
				{
					Flash_DataWrite(CtrlAddress + (FileSize - ExcessSize),ExcessSize,FileData);		//获得一个包写一个包	偏移上次剩余地址
					FileCRC = CRC32H_Chack8(0,FileData,ExcessSize);	//获得的文件校验值
				}
				ExcessSize -= 128;							//计算剩余大小
				YmodemUSART_SendByte(MODEM_ACK);			//接收完成 发送确认信号
				 
				num = num + 1;		//数据包增加
				break;
			}
			case 2:				//接收数据包1024个数据
			{
				if(Ymodem_FileDataCRC(2,FileData,chack)==0)
					return 0;
				
				if(ExcessSize >= 1024)
				{
					Flash_DataWrite(CtrlAddress + (FileSize - ExcessSize),1024,FileData);					//获得一个包写一个包	偏移上次剩余地址
					FileCRC = CRC32H_Chack8(0,FileData,1024);		//获得的文件校验值
				}
				else
				{
					Flash_DataWrite(CtrlAddress + (FileSize - ExcessSize),ExcessSize,FileData);		//获得一个包写一个包	偏移上次剩余地址
					FileCRC = CRC32H_Chack8(0,FileData,ExcessSize);	//获得的文件校验值
				}
				ExcessSize -= 1024;							//计算剩余大小
				YmodemUSART_SendByte(MODEM_ACK);			//接收完成 发送确认信号
				 
				num = num + 1;		//数据包增加
				break;
			}
			case 3:				//接收文件完成
			{
				flag = Ymodem_FileProperty(&serial_number,FileData,chack,0xFF);		//接收一次以文件信息的全0数据
				if(flag == 0)		
					return 0;

				YmodemUSART_SendByte(MODEM_ACK);		//通信结束		、
				DebugPf(0,"\r\nFile Receive Achieve!\r\n");	//终端显示接收完成
				
				switch(Option)
				{
					case 1:	//如果下载的是固件，保存固件对应标志
					{
						Flash_DataWrite(AppCRCAddress,4,(u8*)&FileCRC);							//固件文件校验值
						Flash_DataWrite(AppSizeAddress,4,(u8*)&FileSize);						//固件文件大小
						Flash_DataWrite(AppUpSrcAddress,4,(u8*)&FileUpSrc);					//固件文件来源
						
						if(FLASH_DataSpace_Erase(AppFlagAddress,AppFlagSpaceSize) == 0)
						{
							DebugPf(0,"应用程序备份标志空间擦除失败!\r\n");				//应用程序标志空间擦除失败
							return 0;
						}
						Flash_DataWrite(AppUPAddress,4,(u8*)&newbootflag);					//更新标志
						break;
					}
					case 2://如果下载的是GB字库，保存GB字库对应标志
					{
						Flash_DataWrite(GBFontCRCAddress,4,(u8*)&FileCRC);					//GB字库校验值
						Flash_DataWrite(GBFontSizeAddress,4,(u8*)&FileSize);				//GB字库大小
						Flash_DataWrite(GBFontUpSrcAddress,4,(u8*)&FileUpSrc);			//GB字库来源
						break;
					}
					case 3://如果下载的是ASCII字库，保存ASCII字库对应标志
					{
						Flash_DataWrite(ASCIIFontCRCAddress,4,(u8*)&FileCRC);					//ASCII字库校验值
						Flash_DataWrite(ASCIIFontSizeAddress,4,(u8*)&FileSize);				//ASCII字库大小
						Flash_DataWrite(ASCIIFontUpSrcAddress,4,(u8*)&FileUpSrc);			//ASCII字库来源
						break;
					}
					case 4://如果下载的是ASCII字库，保存ASCII字库对应标志
					{
						Flash_DataWrite(AuxiliaryAppCRCAddress,4,(u8*)&FileCRC);					//辅助程序校验值
						Flash_DataWrite(AuxiliaryAppSizeAddress,4,(u8*)&FileSize);				//辅助程序大小
						Flash_DataWrite(AuxiliaryAppUpSrcAddress,4,(u8*)&FileUpSrc);			//辅助程序来源
						break;
					}
					default:
						break;
				}
				CRC_ResetDR();	  //接收完成时清空CRC
				return 1;
			}
			default:
				return 0;	
		}
	}		 	
}

/*
	//如果接收中途出错，主要原因有2种：
	//1：波特率过快，经测试115200会太快出错，57600实测没出现错误，建议最快使用57600来发送文件
	//2：数据存放空间不够导致接收数据失败

	u8 flag = 0;						//状态标志	
	u8 ydata[4096] = {0};  	//数据存放空间 	   //小于128的按128空间，大于128的按1024的倍数空间 例如2100大小的文件需要用3072的空间来存放
	u8 yname[128] = {0};	  //名字
	u8 ynamelen = 0;		  	//名字长度
	u32 ysizelen = 0;		  	//文件大小
	u32 ydatalen = 0;		  	//数据长度

	flag = Ymodem_START(ydata);					//接收文件
	if(flag == 1)		 //接收成功
	{
		flag = Ymodem_FileDecode(ydata,yname,&ynamelen,&ysizelen,&ydatalen);	  //解析数据
		if(flag == 1)		 //解析成功
		{
			printf("File Name : ");
			USART1_SendString(yname,ynamelen);		 //文件名字
			printf("\r\n");
		
			printf("File Size : %d\r\n",ysizelen);	 //文件大小
		
			printf("File Data : ");
			USART1_SendString(ydata+128,ydatalen);	 //文件数据	偏移128
			printf("\r\n");
		}
	}
*/
