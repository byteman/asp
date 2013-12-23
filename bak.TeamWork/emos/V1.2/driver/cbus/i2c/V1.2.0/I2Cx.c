#include "I2Cx.h"
#include "dt3102_IO.h"

#define IIC_SCK_PORT	GPIOB						//SCK
#define IIC_SCK_PIN		GPIO_Pin_6
#define IIC_SDA_PORT	GPIOB						//SDA
#define IIC_SDA_PIN		GPIO_Pin_7

// #define SET_IIC_SCK		

/****************************************************************************
* 名	称：void I2Cm_SDAO(I2C_Cfg* Ix,u8 Flag)
* 功	能：I2C数据线拉高拉低输出  模拟
* 入口参数: I2C_Cfg* Ix		I2C配置
						u8 Flag				高低	1高 0低
* 出口参数：无
* 说	明：无
****************************************************************************/
void I2Cm_SDAO(u8 Flag)
{
	if(Flag == 1)
// 		GPIO_SetBits(Ix->SDA_Port,Ix->SDA_Pin);			//拉高
	IIC_SDA_PORT->BSRRL = IIC_SDA_PIN;
	else
// 		GPIO_ResetBits(Ix->SDA_Port,Ix->SDA_Pin);		//拉低
	IIC_SDA_PORT->BSRRH = IIC_SDA_PIN;
}

/****************************************************************************
* 名	称：u8 I2Cm_SDAI(I2C_Cfg* Ix)
* 功	能：I2C数据输入			模拟
* 入口参数: I2C_Cfg* Ix		I2C配置
* 出口参数：u8						读到的数据
* 说	明：无
****************************************************************************/
u8 I2Cm_SDAI()
{
// 	return GPIO_ReadInputDataBit(IIC_SDA_PORT,IIC_SDA_PIN);	
	return GetIo(IIC_SDA);
}

/****************************************************************************
* 名	称：void I2Cm_SCKO(I2C_Cfg* Ix,u8 Flag)
* 功	能：时钟拉高拉低		模拟
* 入口参数: I2C_Cfg* Ix		I2C配置
						u8 Flag				高低	1高 0低
* 出口参数：无
* 说	明：无
****************************************************************************/
void I2Cm_SCKO(u8 Flag)
{
	if(Flag == 1)
// 		GPIO_SetBits(Ix->SCK_Port,Ix->SCK_Pin);
	IIC_SCK_PORT->BSRRL = IIC_SCK_PIN;
	else
// 		GPIO_ResetBits(Ix->SCK_Port,Ix->SCK_Pin);
	IIC_SCK_PORT->BSRRH = IIC_SCK_PIN;
}

/****************************************************************************
* 名	称：void I2CmDelay(u32 nCount)
* 功	能：一个工作的频率延时		模拟
* 入口参数：u32 nCount	延时量
* 出口参数：无
* 说	明：无
****************************************************************************/
void I2CmDelay(u32 nCount)
{
  while(nCount--);
}

/****************************************************************************
* 名	称：u8 I2Cx_Init(I2C_Cfg* I2C_t)
* 功	能：I2C引脚初始化
* 入口参数：I2C_Cfg* I2C_t		I2C配置信息
* 出口参数：u8							初始化是否成功	1成功	0失败
* 说	明：I2C引脚必须配置为开漏输出 
****************************************************************************/
u8 I2Cx_Init()
{
// 	GPIO_InitTypeDef P; 										//定义GPIO结构体

// 	if((IIC_SDA_PORT == GPIOB) || (IIC_SCK_PORT == GPIOB))
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);	
// 	P.GPIO_Pin = IIC_SDA_PIN;					//SDA
// 	P.GPIO_Mode = GPIO_Mode_OUT;					//设置引脚为输出
// 	P.GPIO_OType = GPIO_OType_OD;					//复用开漏输出
// 	P.GPIO_Speed = GPIO_Speed_50MHz; 			//设置响应速度为50M
// 	GPIO_Init(IIC_SDA_PORT, &P);
	IoSetMode(IIC_SDA,IO_MODE_OUTPUT);
	IoOpenDrain(IIC_SDA);
	IoSetSpeed(IIC_SDA,IO_SPEED_50M);

// 	P.GPIO_Pin = IIC_SCK_PIN;					//SCK
// 	P.GPIO_Mode = GPIO_Mode_OUT;					//设置引脚为输出
// 	P.GPIO_OType = GPIO_OType_OD;					//复用开漏输出
// 	P.GPIO_Speed = GPIO_Speed_50MHz; 			//设置响应速度为50M
// 	GPIO_Init(IIC_SCK_PORT, &P);
	IoSetMode(IIC_SCK,IO_MODE_OUTPUT);
	IoOpenDrain(IIC_SCK);
	IoSetSpeed(IIC_SCK,IO_SPEED_50M);


// 	I2Cm_SCKO(1);		 								//初始化拉高
// 	I2Cm_SDAO(1);										//初始化拉高
	IoHi(IIC_SCK);
	IoHi(IIC_SDA);
	return 1;
}

/****************************************************************************
* 名	称：u8 I2Cx_Start(I2C_Cfg* Ix)
* 功	能：启动I2C总线
* 入口参数：
* 出口参数：u8									启动是否成功		1成功	0失败
* 说	明：启动I2C总线通讯	   SCLH，SDAH-SDAL，时钟高数据高到低跳变开始
****************************************************************************/
u8 I2Cx_Start()				//I2C启动	
{
// 	I2Cm_SDAO(1);
	IoHi(IIC_SDA);
	I2CmDelay(20);

// 	I2Cm_SCKO(1);
	IoHi(IIC_SCK);
	I2CmDelay(20);

	if(I2Cm_SDAI() == 0)		//SDA线未能拉高则总线忙,退出
		return 0;
				
// 	I2Cm_SDAO(0);					//低跳变
	IoLo(IIC_SDA);
	I2CmDelay(20);						// 大于0.6uS  //72条命令1us

	if(I2Cm_SDAI() == 1) 		//SDA线未能拉低则总线出错,退出
		return 0;
				
// 	I2Cm_SDAO(0);
	IoLo(IIC_SDA);
	I2CmDelay(20);
	
	return 1;						//启动成功
}

/****************************************************************************
* 名	称：u8 I2Cx_Stop(I2C_Cfg* Ix)
* 功	能：关闭I2C总线
* 入口参数：I2C_Cfg* Ix						I2C配置
* 出口参数：u8		关闭是否成功		1成功	0失败
* 说	明：结束I2C总线通讯	   SCLH，SDAL-SDAH，时钟高数据低到高跳变结束
****************************************************************************/
u8 I2Cx_Stop()								//I2C关闭
{
// 	I2Cm_SDAO(0);
	IoLo(IIC_SDA);
	I2CmDelay(20);

// 	I2Cm_SCKO(1);
	IoHi(IIC_SCK);
	I2CmDelay(20);						//大于0.6uS

// 	I2Cm_SDAO(1);					//高跳变
	IoHi(IIC_SDA);
	I2CmDelay(20);						//大于1.3uS

// 	I2Cm_SDAO(0);
	IoLo(IIC_SDA);
	I2CmDelay(20);
	return 1;
}

/****************************************************************************
* 名	称：u8 I2Cx_Ack(I2C_Cfg* Ix)
* 功	能：发送一个应答信号  低应答
* 入口参数：I2C_Cfg* Ix								I2C配置
* 出口参数：u8		命令执行是否成功		1成功	0失败
* 说	明：时钟由低-高-低跳变，发送SDA_L数据
****************************************************************************/
u8 I2Cx_Ack()								//应答	   时钟由低-高-低跳变，发送一次数据
{	
// 	if(Ix == (I2C_Cfg*)0)						//指针检查
// 		return 0;
		
// 	if(Ix->I2Cx == (I2C_TypeDef*)0)	//模拟I2C
// 	{
		I2Cm_SCKO(0);
		I2CmDelay(20);

		I2Cm_SDAO(0);										//发送数据SDA_L，应答
		I2CmDelay(20);											//大于1.3uS

		I2Cm_SCKO(1);		
		I2CmDelay(20);											//大于1uS

		I2Cm_SCKO(0);
		I2CmDelay(20);
// 	}
// 	else												//硬件I2C
// 	{
// 		return 0;									//未实现
// 	}
	return 1;
}

/****************************************************************************
* 名	称：u8 I2Cx_NoAck(I2C_Cfg* Ix)
* 功	能：不发送应答信号	 高不应答
* 入口参数：I2C_Cfg* Ix								I2C配置
* 出口参数：u8		命令执行是否成功		1成功	0失败
* 说	明：时钟由低-高-低跳变，发送SDA_H数据
****************************************************************************/
u8 I2Cx_NoAck()				//不应答
{	
// 	if(Ix == (I2C_Cfg*)0)						//指针检查
// 		return 0;
		
// 	if(Ix->I2Cx == (I2C_TypeDef*)0)	//模拟I2C
// 	{
		I2Cm_SCKO(0);
		I2CmDelay(20);

		I2Cm_SDAO(1);		   				//发送数据SDA_H，不应答
		I2CmDelay(20);

		I2Cm_SCKO(1);
		I2CmDelay(20);

		I2Cm_SCKO(0);
		I2CmDelay(20);
// 	}
// 	else												//硬件I2C
// 	{
// 		return 0;									//未实现
// 	}
	return 1;
}

/****************************************************************************
* 名	称：u8 I2Cx_WaitAck(I2C_Cfg* Ix)
* 功	能：等待应答信号
* 入口参数：I2C_Cfg* Ix								I2C配置
* 出口参数：u8		应答响应或错误代码
* 说	明：应答1，无应答0
****************************************************************************/
u8 I2Cx_WaitAck()
{
	u8 ack;	  //应答信号
// 	I2Cm_SCKO(0);
// 	I2Cm_SDAO(1);
	IoLo(IIC_SCK);
	IoHi(IIC_SDA);
					
	I2CmDelay(20);
// 	I2Cm_SCKO(1);
	IoHi(IIC_SCK);
	I2CmDelay(20);

	if(I2Cm_SDAI() == 1) 	  //有应答获取为高电平
		ack = 0;
	else 
		ack = 1;	 //获取应答信号	 1有应答  0无应答

// 	I2Cm_SCKO(0);
	IoLo(IIC_SCK);
	return ack;
}

/****************************************************************************
* 名	称：u8 I2Cx_SendByte(I2C_Cfg* Ix,u8 SendByte) 
* 功	能：从I2C接口上发送字节数据
* 入口参数：I2C_Cfg* Ix								I2C配置
						SendByte				要发送的字节
* 出口参数：u8							执行是否成功	1成功	0失败
* 说	明：多字节时重复发送即可
****************************************************************************/
u8 I2Cx_SendByte(u8 SendByte) //数据从高位到低位
{
	u8 i = 8;
	while(i--)		  							//发送8位数据
	{
// 		I2Cm_SCKO(0);
		IoLo(IIC_SCK);

		I2CmDelay(20);

		if((SendByte & 0x80) == 0x80)
// 			I2Cm_SDAO(1);  
			IoHi(IIC_SDA);
		else 
// 			I2Cm_SDAO(0); 
			IoLo(IIC_SDA);

		SendByte <<= 1;
		I2CmDelay(20);					//大于1.3uS

// 		I2Cm_SCKO(1);
		IoHi(IIC_SCK);
		I2CmDelay(20);					//大于1uS
	}
// 	I2Cm_SCKO(0);
	IoLo(IIC_SCK);
	return 1;
}

/****************************************************************************
* 名	称：u8 I2Cx_ReceiveByte(I2C_Cfg* Ix)
* 功	能：从I2C接口上接收字节数据
* 入口参数：I2C_Cfg* Ix								I2C配置
* 出口参数：u8								1字节接收数据
* 说	明：多字节重复接收即可
****************************************************************************/
u8 I2Cx_ReceiveByte()  //数据从高位到低位
{ 
	u8 i = 8;				  							//8位
	u8 ReceiveByte = 0;	  					//1字节接收数据缓存
	
// 	if(Ix == (I2C_Cfg*)0)						//指针检查
// 		return 0;
		
// 	if(Ix->I2Cx == (I2C_TypeDef*)0)	//模拟I2C
// 	{
		I2Cm_SDAO(1);
					
		while(i--)
		{
			ReceiveByte <<= 1; 
					 
			I2Cm_SCKO(0);
			I2CmDelay(20);					//大于1.3uS

			I2Cm_SCKO(1);
			I2CmDelay(20);
				
			if(I2Cm_SDAI() == 1)
				ReceiveByte |= 0x01;
		}

		I2Cm_SCKO(0);
		return ReceiveByte;	   		//返回接收到的数据
// 	}
// 	else												//硬件I2C
// 	{
// 		return 0;									//未实现
// 	}
}

/****************************************************************************
* 名	称：u32 I2CSendByteADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u8 ADDR,u8* WData,u32 Length)
* 功	能：发送8位地址数据
* 入口参数：I2C_Cfg* Rx					I2C器件信息
						u8 DeviceAddr				器件地址
						u8 ADDR							目标寄存器
						u8* WData						数据
* 出口参数：u8									发送数据的个数
* 说	明：对目标器件地址、目标寄存器、发送数据				最大支持256地址
****************************************************************************/
u32 I2CSendByteADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u8 ADDR,u8* WData,u32 Length)
{
	u32 Cnt = 0;											//实际发送的数据个数
	
// 	if(Ix == (I2C_Cfg*)0)							//指针检查
// 		return 0;
	
	if(I2Cx_Start() == 0)						//I2C开始时对方器件忙
		return 0;	  		

  I2Cx_SendByte(DeviceAddr);			//器件地址 
  if(I2Cx_WaitAck() == 0)					//等待应答
	{
		I2Cx_Stop(); 
		return 0;	  									
	}

  I2Cx_SendByte(ADDR);						//设置8位地址    
  if(I2Cx_Ack() == 0)							//等待应答
	{
		I2Cx_Stop();
		return 0;	  						
	}

	while(Length)		  								//发送指定长度数据
	{
		I2Cx_SendByte(*WData);				//发送数据
		if(I2Cx_Ack() == 0)						//等待应答
		{
			I2Cx_Stop();
			return 0;	  				
		}
		WData++;
		Cnt++;
		Length--;
	}
	
  I2Cx_Stop(); 		  							//关闭I2C	  //发送完后关闭
	return Cnt;		  									//发送的数据个数
}

/****************************************************************************
* 名	称：u8 I2CSendWordADDRData(u8 DeviceAddr,u16 ADDR,u8* WData,u32 Length)
* 功	能：发送16位地址数据
* 入口参数：
						u8 DeviceAddr				器件地址
						u16 ADDR						目标寄存器
						u8 WData						数据
* 出口参数：u32									是否成功		1成功	0失败
* 说	明：对目标器件地址、目标寄存器、发送数据			最大支持65536个地址
****************************************************************************/
u32 I2CSendWordADDRData(u8 DeviceAddr,u16 ADDR,u8* WData,u32 Length)
{	
	u32 Cnt = 0;											//实际发送的数据个数
		
	if(I2Cx_Start() == 0)						//I2C开始时对方器件忙
		return 0;	  		

  I2Cx_SendByte(DeviceAddr);			//器件地址 
  if(I2Cx_WaitAck() == 0)					//等待应答
	{
		I2Cx_Stop(); 
		return 0;	  							
	}

  I2Cx_SendByte((u8)(ADDR >> 8));			//设置高8地址 
	if(I2Cx_WaitAck() == 0)							//等待应答
	{
		I2Cx_Stop(); 
		return 0;	  											
	}
	
  I2Cx_SendByte((u8)(ADDR % 256));		//设置低8地址 
	if(I2Cx_WaitAck() == 0)							//等待应答
	{
		I2Cx_Stop();
		return 0;	  		
	}
	
	while(Length)		  									//发送指定长度数据
	{
		I2Cx_SendByte(*WData);					//发送数据
		if(I2Cx_Ack() == 0)							//等待应答
		{
			I2Cx_Stop();
			return 0;	  							
		}
		WData++;
		Cnt++;
		Length--;
	}
	
  I2Cx_Stop(); 		  							//关闭I2C	  //发送完后关闭
  return Cnt;		  									//发送的数据个数
}

/****************************************************************************
* 名	称：u32 I2CReceiveByteADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u8 ADDR,u8* RData,u32 Length)
* 功	能：发送数据
* 入口参数：I2C_Cfg* Ix					I2C器件信息
						u8 DeviceAddr				器件地址
						u8 ADDR							目标寄存器
						u8* RData						读取的数据
						u32 Length					希望读取的数据长度
* 出口参数：u32									实际读取的数据个数
* 说	明：对目标器件地址、目标寄存器、发送数据
****************************************************************************/
u32 I2CReceiveByteADDRData(u8 DeviceAddr,u8 ADDR,u8* RData,u32 Length)
{		
	u32 Cnt = 0;											//实际接收的数据个数
	
// 	if(Ix == (I2C_Cfg*)0)							//指针检查
// 		return 0;
	
	if(I2Cx_Start() == 0)	  				//I2C启动
		return 0;	   										//对方器件忙

	I2Cx_SendByte(DeviceAddr);			//器件地址
	if(I2Cx_WaitAck() == 0)					//等待应答
	{
		I2Cx_Stop();
		return 0;	
	}
	
	I2Cx_SendByte(ADDR);						//设置8位地址      
	if(I2Cx_WaitAck() == 0)					//等待应答
	{
		I2Cx_Stop();
		return 0;	   
	}
	
	I2Cx_Stop();
	
	I2Cx_SendByte(DeviceAddr | 0x01);					//器件地址 读
	if(I2Cx_WaitAck() == 0)
	{
		I2Cx_Stop();
		return 0;	   //未能接收到应答
	}

	while(Length)		  //发送指定长度数据
	{
		*RData = I2Cx_ReceiveByte();
		if(Length == 1)
		{
			if(I2Cx_NoAck() == 0)	   //数据接收完就不再应答
			{
				I2Cx_Stop();
				return 0;	   //未能接收到应答
			}
		}
		else 
		{
			if(I2Cx_Ack() == 0)
			{
				I2Cx_Stop();
				return 0;	   //未能接收到应答
			}
		}
		RData++;
		Cnt++;
		Length--;
	}

	I2Cx_Stop();			   //关闭I2C
	return Cnt;		   			//实际接收的数据个数
}

/****************************************************************************
* 名	称：u32 I2CReceiveWordADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u16 ADDR,u8* RData,u32 Length)
* 功	能：发送数据
* 入口参数：I2C_Cfg* Ix					I2C器件信息
						u8 DeviceAddr				器件地址
						u16 ADDR						目标寄存器
						u8* RData						读取的数据
						u32 Length					希望读取的数据长度
* 出口参数：s32									读取的数据个数
* 说	明：对目标器件地址、目标寄存器、发送数据				最大支持65536个地址
****************************************************************************/
s32 I2CReceiveWordADDRData(u8 DeviceAddr,u16 ADDR,u8* RData,u32 Length)
{		
	u32 Cnt = 0;											//读取的数据个数
	
	
	if(I2Cx_Start() == 0)	  				//I2C启动
		return -1;	   										//对方器件忙

	I2Cx_SendByte(DeviceAddr);			//器件地址
	if(I2Cx_WaitAck() == 0)					//等待应答
	{
		I2Cx_Stop();
		return -1;	
	}
	
	I2Cx_SendByte((u8)(ADDR >> 8));	//设置高8位地址      
	if(I2Cx_WaitAck() == 0)						//等待应答
	{
		I2Cx_Stop();
		return -1;	   
	}
	
	I2Cx_SendByte((u8)(ADDR % 256));	//设置低8位地址      
	if(I2Cx_WaitAck() == 0)						//等待应答
	{
		I2Cx_Stop();
		return -1;	   
	}
	
	I2Cx_Stop();
	
	I2Cx_SendByte(DeviceAddr | 0x01);					//器件地址 读
	if(I2Cx_WaitAck() == 0)
	{
		I2Cx_Stop();
		return -1;	   //未能接收到应答
	}

	while(Length)		  //发送指定长度数据
	{
		*RData = I2Cx_ReceiveByte();
		if(Length == 1)
		{
			if(I2Cx_NoAck() == 0)	   //数据接收完就不再应答
			{
				I2Cx_Stop();
				return -1;	   //未能接收到应答
			}
		}
		else 
		{
			if(I2Cx_Ack() == 0)
			{
				I2Cx_Stop();
				return -1;	   //未能接收到应答
			}
		}
		RData++;				//地址偏移
		Cnt++;					//读取的数据个数
		Length--;				//需要接收的数据个数
	}

	I2Cx_Stop();		//关闭I2C
	return Cnt;		   	//实际数据个数
}
