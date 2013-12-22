#ifndef _F24CL64_INIT_H_
#define _F24CL64_INIT_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
#include "I2Cx.h"

#define FM24CL64_A_SIZE 		8192				//FM24CL64容量8KB
	
#define FM24CL64_CMDERASE			0					//芯片擦除
	
typedef struct
{
	u8				DeviceAddr;						//器件地址
	I2C_Cfg		I2Cx;									//I2C配置
}EEPROM_Cfg;											//EEPROM 器件

typedef struct
{
	u32				StartAddr;						//开始地址
	u8				DefaultVal;						//默认数据
	u32				Len;									//擦除数据长度
}EEPROM_EraseCfg;									//EEPROM 擦除用参数

/****************************************************************************
* 名	称：u8 FM24CL64_Init(u8 prio)
* 功	能：铁电初始化
* 入口参数：u8 prio			存储互斥量优先级
* 出口参数：u8		是否成功		1成功	0失败
* 说	明：需要I2C库和CRC库
****************************************************************************/
u8 FM24CL64_Init(u8 prio);		//设备初始化	

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				无用
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 BlockDeviceRead(HANDLE IDH,u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备读操作
* 入口参数：HANDLE IDH		设备句柄
						u32 offset		读偏移地址
						u8* Buffer		存放数据地址				
						u32 len				希望读取的数据个数
						u32* Reallen	实际读取的数据个数
						u32 lParam		参数									0直接读	1效验读
* 出口参数：s32						读取操作是否成功			1成功	0失败
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 BlockDeviceWrite(HANDLE IDH,u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备写操作
* 入口参数：HANDLE IDH		设备句柄							
						u32 offset		写偏移地址
						u8* Buffer		写数据源地址					
						u32 len				希望写入的数据个数		
						u32* Reallen	实际写入的数据个数		len + 2个(效验)
						u32 lParam		参数									0直接写	1效验写
* 出口参数：s32						写入操作是否成功			1成功	0失败
* 说	明：会在每次存入的数据后面追加2位效验数据
例如：参数1 size = 8
			存储占用位置 [0]-[7]参数1 [8]-[9]效验值
			参数2必须从位置[10]开始存储，否则会破坏参数1数据
****************************************************************************/	

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备控制
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明			EEPROM_EraseCfg
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数												返回值
FM24CL64_CMDERASE					EEPROM_EraseCfg							擦除的数据字节数
****************************************************************************/

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	u8 val = 0xCD;			//一字节数据	溢出检查用
	u8 W[300] = {0};	  //写数据
	u8 R[300] = {0};	  //读数据
	u8 C[10000] = {0};	//检查数据
	u8 i = 0;		  			//计数器
	u32 len = 0;	  		//长度
	u32 addr = 0;	  		//开始地址
	u32 RealLen = 0;		//实际读写个数

	HVL_ERR_CODE err = HVL_NO_ERR;								//错误状态
	HANDLE FM24CL64_Dev = 0;											//设备句柄
	
	EEPROM_EraseCfg F24_Def = 		//F24擦除用默认值
	{
		0,													//擦除的开始地址
		0xCC,												//默认数据
		FM24CL64_A_SIZE							//擦除数据个数	8K
	};
	
	USART_STRUCT usart1cfg = 
	{
		57600,																	//波特率
		USART_WordLength_8b,										//数据位
		USART_StopBits_1,												//停止位
		USART_Parity_No													//效验位
	};

	USART1_Config(&usart1cfg);								//串口初始化
	Set_PrintfPort(1);												//设置打印串口
	printf("测试开始\r\n");

	if(FM24CL64_Init(5) == 0)									//EEPROM初始化			优先级
		return;
	
	FM24CL64_Dev = DeviceOpen(BLOCK_AT24C,0,&err);								//设备打开
	if(err != HVL_NO_ERR)
	{
		printf("设备打开失败\r\n");
		while(1);																										
	}
	
	SysTimeDly(500);		//稍停5秒		//防止烧写芯片时执行write操作
	
	//======================================空间大小测试
	//满存储空间数据存储，再溢出一个数据存储，查看其溢出的数据是否会覆盖第一个位置数据(检查数据块容量和溢出是否会覆盖)
	if(DeviceIoctl(FM24CL64_Dev,FM24CL64_CMDERASE,(u32)&F24_Def) != FM24CL64_A_SIZE)		//写0-8191字节数据	共8192字节数据
	{printf("擦除出错\r\n");while(1);}		
	if(BlockDeviceRead(FM24CL64_Dev,0,C,10000,&RealLen,0) == 0)							//读取数据，检查整个芯片的数据存储情况
	{printf("读取出错\r\n");while(1);}	
	if(RealLen != 10000)			//读取的数据个数和希望读取的数据个数不匹配
	{printf("数据不匹配出错\r\n");while(1);}		
	
	RealLen = 0;				//清空复位
	memset(C,0,10000);	//清空复位
	
	if(BlockDeviceWrite(FM24CL64_Dev,FM24CL64_A_SIZE,&val,1,&RealLen,0) == 0)					//写入1数据到溢出地址	
	{printf("写入操作出错\r\n");while(1);}	
	if(RealLen != 1)												//写入的数据个数和希望写入的数据个数不匹配
	{printf("写入数据不匹配出错\r\n");while(1);}
	
	RealLen = 0;				//清空复位
	
	if(BlockDeviceRead(FM24CL64_Dev,0,C,10000,&RealLen,0) == 0)							//读取数据，检查本来希望写到1024上的数据是否覆盖了0上的数据，存储最大位置
	{printf("读取出错\r\n");while(1);}
	if(RealLen != 10000)										//读取的数据个数和希望读取的数据个数不匹配
	{printf("数据不匹配出错\r\n");while(1);}
	
	RealLen = 0;				//清空复位
	
	//清空存储器
	if(DeviceIoctl(FM24CL64_Dev,FM24CL64_CMDERASE,(u32)&F24_Def) != FM24CL64_A_SIZE)
	{printf("擦除出错\r\n");while(1);}	
	
	//带效验参数存储测试
	for(i = 0;i < 100;i++)
		W[i] = i;
	
	if(BlockDeviceWrite(FM24CL64_Dev,0,W,100,&RealLen,1) == 0)					//写入1数据到溢出地址	
	{printf("写入操作出错\r\n");while(1);}	
	if(RealLen != 102)												//写入的数据个数和希望写入的数据个数不匹配
	{printf("写入数据不匹配出错\r\n");while(1);}
	
	if(BlockDeviceRead(FM24CL64_Dev,0,R,120,&RealLen,0) == 0)						//读取数据，查看效验位是否写入
	{printf("读取出错\r\n");while(1);}
	if(RealLen != 120)												//读取的数据个数和希望读取的数据个数不匹配
	{printf("数据不匹配出错\r\n");while(1);}
	
	RealLen = 0;				//清空复位
	
	if(BlockDeviceRead(FM24CL64_Dev,0,R,100,&RealLen,1) == 0)							//读取数据，检查本来希望写到1024上的数据是否覆盖了0上的数据，存储最大位置
	{printf("读取出错\r\n");while(1);}
	if(RealLen != 100)										//读取的数据个数和希望读取的数据个数不匹配
	{printf("数据不匹配出错\r\n");while(1);}
*/
