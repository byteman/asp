#ifndef _MX251608D_INIT_H_
#define _MX251608D_INIT_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif
	
#include "includes.h"	
	
/*
MX25Lxx08D
一页		256		Byte
一扇区	4096	Byte		256*16页
一块		65536	Byte		4096*16扇区		256*256页
1608D = 2M = 2097152 Byte
*/
	
#define MX251608D_CMDERASE							0			//芯片擦除	
#define MX251608D_CMDBLOCKERASE					1			//块擦除			
#define MX251608D_CMDSECTORERASE				2			//扇区擦除
#define MX251608D_CMDISBUSY							3			//忙检测	
#define MX251608D_CMDBUSYWAIT						4			//忙等待
	
/****************************************************************************
* 名	称：u8 MX251608D_Init(u8 prio)
* 功	能：DataFlash初始化
* 入口参数：u8 prio			存储互斥量优先级
* 出口参数：u8		是否成功		1成功	0失败
* 说	明：无
****************************************************************************/
u8 MX251608D_Init(u8 prio);		//设备初始化	

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
						u32 lParam		参数									无用
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
						u32* Reallen	实际写入的数据个数		
						u32 lParam		参数									无用
* 出口参数：s32						写入操作是否成功			1成功	0失败
* 说	明：
****************************************************************************/	

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备控制
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数												返回值
MX251608D_CMDERASE				无用												无用
MX251608D_CMDBLOCKERASE		地址(擦除该地址所处的块)		无用					
MX251608D_CMDSECTORERASE	地址(擦除该地址所处的扇区)	无用
MX251608D_CMDISBUSY				无用												1忙0闲
MX251608D_CMDBUSYWAIT			无用												无用
****************************************************************************/	
	
#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
	u8 W[300] = {0};	  //写数据
	u8 R[300] = {0};	  //读数据
	u8 i = 0;		  			//计数器
	u32 len = 0;	  		//长度
	u32 addr = 0;	  		//开始地址
	u32 RealLen = 0;		//实际读写个数

#include "mx251608d_init.h"
#include "String.h"				//memcpy

	HVL_ERR_CODE err = HVL_NO_ERR;								//错误状态
	
	HANDLE MX251608D_Dev = 0;													//设备句柄
	
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
	
	if(MX251608D_Init(2) == 0)								//设备A初始化				互斥量优先级2
	{
		printf("初始化失败\r\n");
		while(1);																										
	}
	
	MX251608D_Dev = DeviceOpen(BLOCK_MX25,0,&err);								//设备打开
	if(err != HVL_NO_ERR)
	{
		printf("设备打开失败\r\n");
		while(1);																										
	}
	
	if(DeviceIoctl(MX251608D_Dev,MX251608D_CMDERASE,0) == 0)			//芯片擦除
	{
		printf("擦除操作出错\r\n");
		while(1);																										
	}
	if(DeviceIoctl(MX251608D_Dev,MX251608D_CMDBUSYWAIT,0) == 0)		//忙等待
	{
		printf("忙等待操作出错\r\n");
		while(1);																										
	}
	
	if(BlockDeviceRead(MX251608D_Dev,0,R,256,&RealLen,0) == 0)		//擦除检查
	{
		printf("读取出错\r\n");
		while(1);																										//读取失败
	}
	if(RealLen != 256)																						//读取的数据个数和希望读取的数据个数不匹配
	{
		printf("数据不匹配出错\r\n");
		while(1);																										
	}																		
	RealLen = 0;
	
	//写入
	addr = 0;
	len = 0;				//长度由1-231逐个增长

	while(1)
	{
		len++;
		if(len > 231)
			len = 1;
		

		for(i = 0; i < len; i++)
			W[i] = i;	

		if(BlockDeviceWrite(MX251608D_Dev,addr,W,len,&RealLen,0) == 0)		//写入数据
		{
			printf("写入操作出错\r\n");
			while(1);																										
		}	
		if(RealLen != len)																						//写入的数据个数和希望写入的数据个数不匹配
		{
			printf("写入数据不匹配出错\r\n");
			while(1);
		}
		RealLen = 0;
		
		addr += len;  	
		
		if(addr >= 0x00100000) 			//地址数测试
			break;  		
	}

	//读取
	addr = 0;
	len = 0;
	while(1)
	{
		len++;
		if(len > 231)
			len = 1;
		

		for(i = 0; i < len; i++)
			W[i] = i;	

		if(BlockDeviceRead(MX251608D_Dev,addr,R,len,&RealLen,0) == 0)	//读取检查
		{
			printf("读取出错\r\n");
			while(1);																										//读取失败
		}
		if(RealLen != len)																						//读取的数据个数和希望读取的数据个数不匹配
		{
			printf("数据不匹配出错\r\n");
			while(1);																										
		}																		
		RealLen = 0;
		
		//比较
		if(memcmp(W, R, len) != 0)
			break;

		addr += len;  
		
		if(addr >= 0x00100000) 		//地址数测试
			break;  		  		
	}
*/
