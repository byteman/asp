#ifndef _I2CX_H_
#define _I2CX_H_

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "stm32f4xx.h"
	
typedef struct
{
	I2C_TypeDef*		I2Cx;						//I2C选择，0-IO口模拟I2C；1-I2C1；2-I2C2；3-I2C3；其他无效
	u16 						SCK_Pin;				//SCK引脚号			GPIO_Pin_0 - GPIO_Pin_15
	GPIO_TypeDef* 	SCK_Port;				//SCK端口号			GPIOA - GPIOI
	u16 						SDA_Pin;				//SDA引脚号			GPIO_Pin_0 - GPIO_Pin_15
	GPIO_TypeDef* 	SDA_Port;				//SDA端口号			GPIOA - GPIOI
}I2C_Cfg;													//I2C配置	

/****************************************************************************
* 名	称：u8 I2Cx_Init(I2C_Cfg* I2C_t)
* 功	能：I2C引脚初始化
* 入口参数：I2C_Cfg* I2C_t		I2C配置信息
* 出口参数：u8							初始化是否成功	1成功	0失败
* 说	明：I2C引脚必须配置为开漏输出 
****************************************************************************/
u8 I2Cx_Init(void);															//I2C引脚初始化

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
u32 I2CSendByteADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u8 ADDR,u8* WData,u32 Length);	//8位发送地址数据

/****************************************************************************
* 名	称：u8 I2CSendWordADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u16 ADDR,u8* WData,u32 Length)
* 功	能：发送16位地址数据
* 入口参数：I2C_Cfg* Rx					I2C器件信息
						u8 DeviceAddr				器件地址
						u16 ADDR						目标寄存器
						u8 WData						数据
* 出口参数：u32									是否成功		1成功	0失败
* 说	明：对目标器件地址、目标寄存器、发送数据			最大支持65536个地址
****************************************************************************/
u32 I2CSendWordADDRData(u8 DeviceAddr,u16 ADDR,u8* WData,u32 Length);	//16位发送地址数据

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
u32 I2CReceiveByteADDRData(u8 DeviceAddr,u8 ADDR,u8* RData,u32 Length);			//8位地址接收数据

/****************************************************************************
* 名	称：u32 I2CReceiveWordADDRData(I2C_Cfg* Ix,u8 DeviceAddr,u16 ADDR,u8* RData,u32 Length)
* 功	能：发送数据
* 入口参数：I2C_Cfg* Ix					I2C器件信息
						u8 DeviceAddr				器件地址
						u16 ADDR						目标寄存器
						u8* RData						读取的数据
						u32 Length					希望读取的数据长度
* 出口参数：s32									读取的数据个数
* 说	明：对目标器件地址、目标寄存器、发送数据
****************************************************************************/
s32 I2CReceiveWordADDRData(u8 DeviceAddr,u16 ADDR,u8* RData,u32 Length);		//16位地址接收数据

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

