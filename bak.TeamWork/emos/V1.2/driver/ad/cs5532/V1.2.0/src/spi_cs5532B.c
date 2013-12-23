#include "spi_cs5532B.h"
#include "Includes.h"
#include <stdbool.h>
#include "dt3102_io.h"

#define CS5532B_SPI                           SPI2
#define CS5532B_SPI_CLK                       RCC_APB1Periph_SPI2
#define CS5532B_SPI_CLK_INIT                  RCC_APB1PeriphClockCmd

#define CS5532B_SPI_SCK_GPIO_PIN              GPIO_Pin_13
#define CS5532B_SPI_SCK_GPIO_PORT             GPIOB
#define CS5532B_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOB
#define CS5532B_SPI_SCK_SOURCE                GPIO_PinSource13
#define CS5532B_SPI_SCK_AF                    GPIO_AF_SPI2

#define CS5532B_SPI_MISO_GPIO_PIN             GPIO_Pin_14
#define CS5532B_SPI_MISO_GPIO_PORT            GPIOB
#define CS5532B_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define CS5532B_SPI_MISO_SOURCE               GPIO_PinSource14
#define CS5532B_SPI_MISO_AF                   GPIO_AF_SPI2

#define CS5532B_SPI_MOSI_GPIO_PIN             GPIO_Pin_15
#define CS5532B_SPI_MOSI_GPIO_PORT            GPIOB
#define CS5532B_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define CS5532B_SPI_MOSI_SOURCE               GPIO_PinSource15
#define CS5532B_SPI_MOSI_AF                   GPIO_AF_SPI2

#define CS5532B1_CS_GPIO_PIN                   GPIO_Pin_2
#define CS5532B1_CS_GPIO_PORT                  GPIOD
#define CS5532B1_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOD

#define CS5532B2_CS_GPIO_PIN                   GPIO_Pin_3
#define CS5532B2_CS_GPIO_PORT                  GPIOD
#define CS5532B2_CS_GPIO_CLK                   RCC_AHB1Periph_GPIOD


#define CS5532B1_CS_LOW()       IoLo(CS5532B1_CS_GPIO)
#define CS5532B1_CS_HIGH()      IoHi(CS5532B1_CS_GPIO)
#define CS5532B2_CS_LOW()       IoLo(CS5532B2_CS_GPIO)
#define CS5532B2_CS_HIGH()      IoHi(CS5532B2_CS_GPIO) 
//读MISO状态
#define	CS5532B_READ_MISO()		GetIo(CS5532B_SPI_MISO_GPIO)
//读SPI状态
#define	CS5532B_SPI_STATUS(flag)	( (CS5532B_SPI->SR & flag) ? SET : RESET )
//发送SPI数据
#define	CS5532B_SPI_SEND8DATA(data)	( CS5532B_SPI->DR = data )
//获取SPI数据
#define	CS5532B_SPI_RECEIVE8DATA()	(CS5532B_SPI->DR)

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
static void CS5532B_LowLevel_Init(void)
{
//  GPIO_InitTypeDef GPIO_InitStructure;

  /*!< Enable the SPI clock */
  CS5532B_SPI_CLK_INIT(CS5532B_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(CS5532B_SPI_SCK_GPIO_CLK |
                         CS5532B_SPI_MISO_GPIO_CLK | 
                         CS5532B_SPI_MOSI_GPIO_CLK |
                         CS5532B1_CS_GPIO_CLK|
                         CS5532B2_CS_GPIO_CLK
                         , ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(CS5532B_SPI_SCK_GPIO_PORT, CS5532B_SPI_SCK_SOURCE, CS5532B_SPI_SCK_AF);
  GPIO_PinAFConfig(CS5532B_SPI_MISO_GPIO_PORT, CS5532B_SPI_MISO_SOURCE, CS5532B_SPI_MISO_AF);
  GPIO_PinAFConfig(CS5532B_SPI_MOSI_GPIO_PORT, CS5532B_SPI_MOSI_SOURCE, CS5532B_SPI_MOSI_AF);

//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
//         
//   /*!< SPI SCK pin configuration */
//   GPIO_InitStructure.GPIO_Pin = CS5532B_SPI_SCK_PIN;
//   GPIO_Init(CS5532B_SPI_SCK_GPIO_PORT, &GPIO_InitStructure);

//   /*!< SPI MOSI pin configuration */
//   GPIO_InitStructure.GPIO_Pin =  CS5532B_SPI_MOSI_PIN;
//   GPIO_Init(CS5532B_SPI_MOSI_GPIO_PORT, &GPIO_InitStructure);

//   /*!< SPI MISO pin configuration */
//   GPIO_InitStructure.GPIO_Pin =  CS5532B_SPI_MISO_PIN;
//   GPIO_Init(CS5532B_SPI_MISO_GPIO_PORT, &GPIO_InitStructure);

//   /*!< Configure CS5532B Card CS pin in output pushpull mode ********************/
//   GPIO_InitStructure.GPIO_Pin = CS5532B1_CS_PIN;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
//   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
//   GPIO_Init(CS5532B1_CS_GPIO_PORT, &GPIO_InitStructure);
//   GPIO_InitStructure.GPIO_Pin = CS5532B2_CS_PIN;
//   GPIO_Init(CS5532B2_CS_GPIO_PORT, &GPIO_InitStructure);

	//替换,替换后的代码比之前的代码大近200个字节
	IoSetMode(CS5532B_SPI_SCK_GPIO, IO_MODE_ALTER);				//SPI SCK
	IoSetSpeed(CS5532B_SPI_SCK_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532B_SPI_SCK_GPIO);  

	IoSetMode(CS5532B_SPI_MOSI_GPIO, IO_MODE_ALTER);			//SPI MOSI
	IoSetSpeed(CS5532B_SPI_MOSI_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532B_SPI_MOSI_GPIO);  
	
	IoSetMode(CS5532B_SPI_MISO_GPIO, IO_MODE_ALTER);			//SPI MISO
	IoSetSpeed(CS5532B_SPI_MISO_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532B_SPI_MISO_GPIO);  
	
	IoSetMode(CS5532B1_CS_GPIO, IO_MODE_OUTPUT);				//CS5532B1 CS
	IoSetSpeed(CS5532B1_CS_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532B1_CS_GPIO);  

	IoSetMode(CS5532B2_CS_GPIO, IO_MODE_OUTPUT);				//CS5532B2 CS
	IoSetSpeed(CS5532B2_CS_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532B2_CS_GPIO);
}

void CS5532B_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  CS5532B_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  CS5532B1_CS_HIGH();
  CS5532B2_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_8;	//8分频后5.25Mhz

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(CS5532B_SPI, &SPI_InitStructure);

  /*!< Enable the CS5532B_SPI  */
  SPI_Cmd(CS5532B_SPI, ENABLE);
}
/**
  * @brief  Sends a byte through the SPI interface and return the byte received
  *         from the SPI bus.
  * @param  byte: byte to send.
  * @retval The value of the received byte.
  */
static u8 CS5532B_TRByte(u8 byte)
{
  while (CS5532B_SPI_STATUS(SPI_I2S_FLAG_TXE) == RESET);
  CS5532B_SPI_SEND8DATA(byte);
  while (CS5532B_SPI_STATUS(SPI_I2S_FLAG_RXNE) == RESET);
  return CS5532B_SPI_RECEIVE8DATA();
}
static u32 CS5532B_ReadWord(u8 CS5532Bx)
{
    u32 uWord = 0;

    CS5532B1_CS_HIGH();
    CS5532B2_CS_HIGH();
    if (CS5532Bx == CS5532B1) {
        CS5532B1_CS_LOW();
    } else {
        CS5532B2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);
    
    uWord  = (CS5532B_TRByte(0x00)<<24);
    uWord |= (CS5532B_TRByte(0x00)<<16);
    uWord |= (CS5532B_TRByte(0x00)<<8);
    uWord |= (CS5532B_TRByte(0x00)<<0);
    
    CS5532_CS_Delay(NS_50);
    
    return uWord;
}
//读取AD值专用
static u32 CS5532Bx_ReadWord(void)
{
    u32 uWord = 0;

    uWord = (CS5532B_TRByte(0x00)<<16);
    uWord |= (CS5532B_TRByte(0x00)<<8);
    uWord |= (CS5532B_TRByte(0x00)<<0);
    CS5532B_TRByte(0x00);				//移除描述符 
        
    return uWord;
}

static u8 CS5532B_SendByte(u8 CS5532Bx,u8 byte)
{
    CS5532B1_CS_HIGH();
    CS5532B2_CS_HIGH();
    if (CS5532Bx & CS5532B1) {
        CS5532B1_CS_LOW();
    }
    if (CS5532Bx & CS5532B2) {
        CS5532B2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);

    CS5532B_TRByte(byte);
        
    CS5532_CS_Delay(NS_50);

    return 0;
}

static u8 CS5532B_SendWord(u8 CS5532Bx,u32 uWord)
{
    CS5532B1_CS_HIGH();
    CS5532B2_CS_HIGH();
    if (CS5532Bx & CS5532B1) {
        CS5532B1_CS_LOW();
    }
    if (CS5532Bx & CS5532B2) {
        CS5532B2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);

    CS5532B_TRByte((uWord&0xff000000)>>24);
    CS5532B_TRByte((uWord&0x00ff0000)>>16);
    CS5532B_TRByte((uWord&0x0000ff00)>>8);
    CS5532B_TRByte((uWord&0x000000ff)>>0);

    CS5532_CS_Delay(NS_50);

    return 0;
}
/****************************************************************************
* 名	称：void CS5532_SetCMD(void)
* 功	能：设置成命令接收
* 入口参数：无
* 出口参数：无
* 说	明：	
****************************************************************************/
static void CS5532B_SetCMD(u8 CS5532Bx)
{
	u8 i;
	for(i=0;i<15;i++)
		CS5532B_SendByte(CS5532Bx,SYNC1);
	CS5532B_SendByte(CS5532Bx,SYNC0);		
}

/****************************************************************************
* 名	称：void CS5532_ConfigRegister(void)
* 功	能：配置寄存器
* 入口参数：无
* 出口参数：无
* 说	明：	
****************************************************************************/ 	
static void CS5532B_ConfigRegister(u8 CS5532Bx)
{
	CS5532B_SendByte(CS5532Bx,ConfigRegister+WriteRegister);				//配置寄存器
	CS5532B_SendWord(CS5532Bx,SETUP1_VRS);									//参考电压		 	2.5V < Vref <= VA+
}

/****************************************************************************
* 名	称：void CS5532_ChannelRegister(void)
* 功	能：通道寄存器
* 入口参数：无
* 出口参数：无
* 说	明：	
****************************************************************************/ 
static void CS5532B_ChannelRegister(u8 CS5532Bx)
{
	CS5532B_SendByte(CS5532Bx,ChannelRegister + ChannelSet_1 + WriteRegister);	//通道寄存器的通道配置寄存器1
	CS5532B_SendWord(CS5532Bx,SETUP1+SETUP2);									//写通道配置
}

/****************************************************************************
* 名	称：void CS5532_StartConvert(u8 flag,u8 SetupNum)
* 功	能：开始转换
* 入口参数：u8 flag			0单次转换	1连续转换
			u8 SetupNum 	使用第几个设置单元中的配置 1-8(CS5532 1-2)	
* 出口参数：无
* 说	明：无
****************************************************************************/
static BOOL CS5532B_Start(u8 CS5532Bx,u8 flag,u8 SetupNum)
{
	u8 SetupX=0x00;

	switch(SetupNum)   //设置单元
	{
		case 1:
			SetupX=ChannelPointer_1;	//设置单元1
			break;
		case 2:
			SetupX=ChannelPointer_2;	//设置单元2
			break;
		case 3:
			SetupX=ChannelPointer_3;	//设置单元3
			break;
		case 4:
			SetupX=ChannelPointer_4;	//设置单元4
			break;
		case 5:
			SetupX=ChannelPointer_5;	//设置单元5
			break;
		case 6:
			SetupX=ChannelPointer_6;	//设置单元6
			break;
		case 7:
			SetupX=ChannelPointer_7;	//设置单元7
			break;
		case 8:
			SetupX=ChannelPointer_8;	//设置单元8
			break;
		default:
			SetupX=ChannelPointer_1;	//设置单元1
			break;	
	}
	if (flag == 0)
		CS5532B_SendByte(CS5532Bx,StartConvert+SetupX+Single);			//发送单次转换命令
	else
		CS5532B_SendByte(CS5532Bx,StartConvert+SetupX+Continuation);	//发送连续转换命令
	
	CS5532B1_CS_HIGH();
	CS5532B2_CS_HIGH();

	return TRUE;
}
int32 CS5532Bx_GetValue(void)
{
	int32 ADCData = 0;
	CS5532B_TRByte(0x00);				   		//清SDO值
	ADCData = CS5532Bx_ReadWord();   			//读ADC数据

	//扩展
	if((ADCData & 0x00800000) != 0){
		ADCData |= 0xFF000000;
	}

	return ADCData;        			  			//24位数据
}
/****************************************************************************
* 名	称：u8 CS5532B1_Flag(void)
* 功	能：是否转换完成	查询方式
* 入口参数：无
* 出口参数：u8	1转换完成
				0转换未完成
* 说	明：转换完成后CS5532的SDO引脚一直为低电平
****************************************************************************/
u8 CS5532B1_Flag(void)
{
	CS5532B2_CS_HIGH();
	CS5532B1_CS_LOW();	
	CS5532_CS_Delay(NS_50);
	if( Bit_RESET == CS5532B_READ_MISO() )  //转换完成
		return 1;
	else
		return 0;
}
u8 CS5532B2_Flag(void)
{
	CS5532B1_CS_HIGH();
	CS5532B2_CS_LOW();
	CS5532_CS_Delay(NS_50);
	if( Bit_RESET == CS5532B_READ_MISO() )  //转换完成
		return 1;
	else
		return 0;
}

/****************************************************************************
* 名	称：u8 fStepConfgAD_Ax( u8 u8Channel , u8 Step )
* 功	能：复位5532并初始化
* 入口参数：u8	要初始化的通道号
* 出口参数：u8	初始化步骤
* 说	明：
			第一次调用该函数 -》复位		Step = 1
			延时20ms(外部实现)
			第二次调用该函数 -》芯片配置	Step = 2
****************************************************************************/
u8 fStepConfgAD_Bx( u8 u8Channel , u8 Step)
{
	u8 ret=u8Channel;
    u32 temp;
	if( 1 == Step )		//复位
	{
		CS5532B_SetCMD(u8Channel);			   					 //设置成命令模式

		CS5532B_SendByte(u8Channel,ConfigRegister+WriteRegister);//对配置寄存器进行写操作
		CS5532B_SendWord(u8Channel,RS1);						 //复位
		return true ;
	}
	//延时20毫秒后
	if( 2 == Step )		//配置
	{
		//对配置寄存器进行读操作	    0x10000000
		CS5532B_SendByte(u8Channel,ConfigRegister+ReadRegister);	    
		
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);					    //RV为1 0x10000000 复位操作结束	读取一次后RV自动为0
			if(temp!=0x10000000)
				ret &= ~CS5532B1;								    //复位标志不正确
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);				        //RV为1 0x10000000 复位操作结束	读取一次后RV自动为0
			if(temp!=0x10000000)
				ret &= ~CS5532B2;								    //复位标志不正确
		}
		if(!ret) {
			return ret;
		}
		
		//读配置寄存器  初始化值验证    0x00000000
		CS5532B_SendByte(u8Channel,ConfigRegister+ReadRegister);	    
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)                                   //配置寄存器复位默认值
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)                                   //配置寄存器复位默认值
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}
		
		//读偏移寄存器1  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ExcursionRegister+Excursion_1+ReadRegister);  
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)                                   //偏移寄存器1复位默认值
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)                                   //偏移寄存器1复位默认值
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读偏移寄存器2  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ExcursionRegister+Excursion_2+ReadRegister);
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)                                   //偏移寄存器2复位默认值
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)                                   //偏移寄存器2复位默认值
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}
		
		//读增益寄存器1  初始化值验证   0x01000000
		CS5532B_SendByte(u8Channel,GainRegister+GainSet_1+ReadRegister);	    
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x01000000)                                   //增益寄存器1复位默认值
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x01000000)                                   //增益寄存器1复位默认值
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读增益寄存器2  初始化值验证   0x01000000
		CS5532B_SendByte(u8Channel,GainRegister+GainSet_2+ReadRegister);			
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x01000000)                                   //增益寄存器2复位默认值
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x01000000)                                   //增益寄存器2复位默认值
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读通道寄存器1  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ChannelRegister+ChannelSet_1+ReadRegister);		
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读通道寄存器2  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ChannelRegister+ChannelSet_2+ReadRegister);		
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读通道寄存器3  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ChannelRegister+ChannelSet_3+ReadRegister);		
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)
				ret &= ~CS5532B2;
		}
		if(!ret) {
			return ret;
		}

		//读通道寄存器4  初始化值验证   0x00000000
		CS5532B_SendByte(u8Channel,ChannelRegister+ChannelSet_4+ReadRegister);		
		if(u8Channel & ret & CS5532B1) {
			temp = CS5532B_ReadWord(CS5532B1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532B1;
		}
		if(u8Channel & ret & CS5532B2) {
			temp = CS5532B_ReadWord(CS5532B2);	
			if(temp!=0x00000000)
				ret &= ~CS5532B2;
		}
	}
	
	CS5532B_ConfigRegister(ret);	//设置	  配置寄存器	  参考电压
	CS5532B_ChannelRegister(ret);	//设置	  通道寄存器	  物理通道 + 增益 + 字速率 + 单双极 + 锁存位 + 延时 +  开路检测 +  偏移/增益物理通道选择

	CS5532B_Start(ret,1,2);			//开启连续转换	
	
	return true;
}
