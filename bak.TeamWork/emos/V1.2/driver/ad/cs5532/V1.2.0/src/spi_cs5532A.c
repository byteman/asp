#include "spi_cs5532A.h"
#include "Includes.h"
#include <stdbool.h>
#include "dt3102_IO.h"

#define CS5532A_SPI                           SPI1
#define CS5532A_SPI_CLK                       RCC_APB2Periph_SPI1
#define CS5532A_SPI_CLK_INIT                  RCC_APB2PeriphClockCmd

#define CS5532A_SPI_SCK_GPIO_PIN              GPIO_Pin_5
#define CS5532A_SPI_SCK_GPIO_PORT             GPIOA
#define CS5532A_SPI_SCK_GPIO_CLK              RCC_AHB1Periph_GPIOA
#define CS5532A_SPI_SCK_SOURCE                GPIO_PinSource5
#define CS5532A_SPI_SCK_AF                    GPIO_AF_SPI1

#define CS5532A_SPI_MISO_GPIO_PIN             GPIO_Pin_6
#define CS5532A_SPI_MISO_GPIO_PORT            GPIOA
#define CS5532A_SPI_MISO_GPIO_CLK             RCC_AHB1Periph_GPIOA
#define CS5532A_SPI_MISO_SOURCE               GPIO_PinSource6
#define CS5532A_SPI_MISO_AF                   GPIO_AF_SPI1

#define CS5532A_SPI_MOSI_GPIO_PIN             GPIO_Pin_5
#define CS5532A_SPI_MOSI_GPIO_PORT            GPIOB
#define CS5532A_SPI_MOSI_GPIO_CLK             RCC_AHB1Periph_GPIOB
#define CS5532A_SPI_MOSI_SOURCE               GPIO_PinSource5
#define CS5532A_SPI_MOSI_AF                   GPIO_AF_SPI1

#define CS5532A1_CS_GPIO_PIN                  GPIO_Pin_0
#define CS5532A1_CS_GPIO_PORT                 GPIOD
#define CS5532A1_CS_GPIO_CLK                  RCC_AHB1Periph_GPIOD

#define CS5532A2_CS_GPIO_PIN                  GPIO_Pin_1
#define CS5532A2_CS_GPIO_PORT                 GPIOD
#define CS5532A2_CS_GPIO_CLK                  RCC_AHB1Periph_GPIOD


#define CS5532A1_CS_LOW()       IoLo(CS5532A1_CS_GPIO)
#define CS5532A1_CS_HIGH()      IoHi(CS5532A1_CS_GPIO)
#define CS5532A2_CS_LOW()       IoLo(CS5532A2_CS_GPIO)
#define CS5532A2_CS_HIGH()      IoHi(CS5532A2_CS_GPIO) 
//��MISO״̬
#define	CS5532A_READ_MISO()		GetIo(CS5532A_SPI_MISO_GPIO)
//��SPI״̬
#define	CS5532A_SPI_STATUS(flag)	( (CS5532A_SPI->SR & flag) ? SET : RESET )
//����SPI����
#define	CS5532A_SPI_SEND8DATA(data)	( CS5532A_SPI->DR = data )
//��ȡSPI����
#define	CS5532A_SPI_RECEIVE8DATA()	(CS5532A_SPI->DR)

static void CS5532A_LowLevel_Init(void)
{
  /*!< Enable the SPI clock */
  CS5532A_SPI_CLK_INIT(CS5532A_SPI_CLK, ENABLE);

  /*!< Enable GPIO clocks */
  RCC_AHB1PeriphClockCmd(CS5532A_SPI_SCK_GPIO_CLK |
                         CS5532A_SPI_MISO_GPIO_CLK | 
                         CS5532A_SPI_MOSI_GPIO_CLK |
                         CS5532A1_CS_GPIO_CLK|
                         CS5532A2_CS_GPIO_CLK
                         , ENABLE);
  
  /*!< SPI pins configuration *************************************************/

  /*!< Connect SPI pins to AF5 */  
  GPIO_PinAFConfig(CS5532A_SPI_SCK_GPIO_PORT, CS5532A_SPI_SCK_SOURCE, CS5532A_SPI_SCK_AF);
  GPIO_PinAFConfig(CS5532A_SPI_MISO_GPIO_PORT, CS5532A_SPI_MISO_SOURCE, CS5532A_SPI_MISO_AF);
  GPIO_PinAFConfig(CS5532A_SPI_MOSI_GPIO_PORT, CS5532A_SPI_MOSI_SOURCE, CS5532A_SPI_MOSI_AF);
  
	//�滻,�滻��Ĵ����֮ǰ�Ĵ�����200���ֽ�
	IoSetMode(CS5532A_SPI_SCK_GPIO, IO_MODE_ALTER);				//SPI SCK
	IoSetSpeed(CS5532A_SPI_SCK_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532A_SPI_SCK_GPIO);  

	IoSetMode(CS5532A_SPI_MOSI_GPIO, IO_MODE_ALTER);			//SPI MOSI
	IoSetSpeed(CS5532A_SPI_MOSI_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532A_SPI_MOSI_GPIO);  
	
	IoSetMode(CS5532A_SPI_MISO_GPIO, IO_MODE_ALTER);			//SPI MISO
	IoSetSpeed(CS5532A_SPI_MISO_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532A_SPI_MISO_GPIO);  
	
	IoSetMode(CS5532A1_CS_GPIO, IO_MODE_OUTPUT);				//CS5532A1 CS
	IoSetSpeed(CS5532A1_CS_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532A1_CS_GPIO);  

	IoSetMode(CS5532A2_CS_GPIO, IO_MODE_OUTPUT);				//CS5532A2 CS
	IoSetSpeed(CS5532A2_CS_GPIO, IO_SPEED_50M);
	IoPushPull(CS5532A2_CS_GPIO);  
}
/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void CS5532A_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;

  CS5532A_LowLevel_Init();
    
  /*!< Deselect the FLASH: Chip Select high */
  CS5532A1_CS_HIGH();
  CS5532A2_CS_HIGH();

  /*!< SPI configuration */
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;//16��Ƶ��5.25Mhz

  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
  SPI_Init(CS5532A_SPI, &SPI_InitStructure);

  /*!< Enable the CS5532A_SPI  */
  SPI_Cmd(CS5532A_SPI, ENABLE);
}

static u8 CS5532A_TRByte(u8 byte)
{
  while (CS5532A_SPI_STATUS(SPI_I2S_FLAG_TXE) == RESET);
  CS5532A_SPI_SEND8DATA(byte);
  while (CS5532A_SPI_STATUS(SPI_I2S_FLAG_RXNE) == RESET);
  return CS5532A_SPI_RECEIVE8DATA();
}

static u8 CS5532A_SendByte(u8 CS5532Ax,u8 byte)
{
    CS5532A1_CS_HIGH();
    CS5532A2_CS_HIGH();
    if (CS5532Ax & CS5532A1) {
        CS5532A1_CS_LOW();
    }
    if (CS5532Ax & CS5532A2) {
        CS5532A2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);

    CS5532A_TRByte(byte);
        
    CS5532_CS_Delay(NS_50);

    return 0;
}

static u8 CS5532A_SendWord(u8 CS5532Ax,u32 uWord)
{
    CS5532A1_CS_HIGH();
    CS5532A2_CS_HIGH();
    if (CS5532Ax & CS5532A1) {
        CS5532A1_CS_LOW();
    }
    if (CS5532Ax & CS5532A2) {
        CS5532A2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);

    CS5532A_TRByte((uWord&0xff000000)>>24);
    CS5532A_TRByte((uWord&0x00ff0000)>>16);
    CS5532A_TRByte((uWord&0x0000ff00)>>8);
    CS5532A_TRByte((uWord&0x000000ff)>>0);

    CS5532_CS_Delay(NS_50);

    return 0;
}

static u32 CS5532A_ReadWord(u8 CS5532Ax)
{
    u32 uWord = 0;

    CS5532A1_CS_HIGH();
    CS5532A2_CS_HIGH();
    if (CS5532Ax == CS5532A1) {
        CS5532A1_CS_LOW();
    } else {
        CS5532A2_CS_LOW();
    }
    CS5532_CS_Delay(NS_50);
    
    uWord = (CS5532A_TRByte(0x00)<<24);
    uWord |= (CS5532A_TRByte(0x00)<<16);
    uWord |= (CS5532A_TRByte(0x00)<<8);
    uWord |= (CS5532A_TRByte(0x00)<<0);
    
    CS5532_CS_Delay(NS_50);
    
    return uWord;
}
static u32 CS5532Ax_ReadWord(void)
{
    u32 uWord = 0;
    
    uWord = (CS5532A_TRByte(0x00)<<16);
    uWord |= (CS5532A_TRByte(0x00)<<8);
    uWord |= (CS5532A_TRByte(0x00)<<0);
    CS5532A_TRByte(0x00);				//����������������
    
    return uWord;
}
/****************************************************************************
* ��	�ƣ�void CS5532_SetCMD(void)
* ��	�ܣ����ó��������
* ��ڲ�������
* ���ڲ�������
* ˵	����	
****************************************************************************/
static void CS5532A_SetCMD(u8 CS5532Ax)
{
	u8 i;
	for(i=0;i<15;i++)
		CS5532A_SendByte(CS5532Ax,SYNC1);
	CS5532A_SendByte(CS5532Ax,SYNC0);		
}

/****************************************************************************
* ��	�ƣ�void CS5532_ConfigRegister(void)
* ��	�ܣ����üĴ���
* ��ڲ�������
* ���ڲ�������
* ˵	����	
****************************************************************************/ 	
static void CS5532A_ConfigRegister(u8 CS5532Ax)
{
	CS5532A_SendByte(CS5532Ax,ConfigRegister+WriteRegister);					//���üĴ���
	CS5532A_SendWord(CS5532Ax,SETUP1_VRS);									//�ο���ѹ		 	2.5V < Vref <= VA+
}

/****************************************************************************
* ��	�ƣ�void CS5532_ChannelRegister(void)
* ��	�ܣ�ͨ���Ĵ���
* ��ڲ�������
* ���ڲ�������
* ˵	����	
****************************************************************************/ 	
static void CS5532A_ChannelRegister(u8 CS5532Ax)
{
	CS5532A_SendByte(CS5532Ax,ChannelRegister + ChannelSet_1 + WriteRegister);	//ͨ���Ĵ�����ͨ�����üĴ���1
	CS5532A_SendWord(CS5532Ax,SETUP1+SETUP2);									//дͨ������
}

/****************************************************************************
* ��	�ƣ�void CS5532_StartConvert(u8 flag,u8 SetupNum)
* ��	�ܣ���ʼת��
* ��ڲ�����u8 flag			0����ת��	1����ת��
			u8 SetupNum 	ʹ�õڼ������õ�Ԫ�е����� 1-8(CS5532 1-2)	
* ���ڲ�������
* ˵	������
****************************************************************************/
static BOOL CS5532A_Start(u8 CS5532Ax,u8 flag,u8 SetupNum)
{
	u8 SetupX=0x00;

	switch(SetupNum)   //���õ�Ԫ
	{
		case 1:
			SetupX=ChannelPointer_1;	//���õ�Ԫ1
			break;
		case 2:
			SetupX=ChannelPointer_2;	//���õ�Ԫ2
			break;
		case 3:
			SetupX=ChannelPointer_3;	//���õ�Ԫ3
			break;
		case 4:
			SetupX=ChannelPointer_4;	//���õ�Ԫ4
			break;
		case 5:
			SetupX=ChannelPointer_5;	//���õ�Ԫ5
			break;
		case 6:
			SetupX=ChannelPointer_6;	//���õ�Ԫ6
			break;
		case 7:
			SetupX=ChannelPointer_7;	//���õ�Ԫ7
			break;
		case 8:
			SetupX=ChannelPointer_8;	//���õ�Ԫ8
			break;
		default:
			SetupX=ChannelPointer_1;	//���õ�Ԫ1
			break;	
	}
	if (flag == 0)
		CS5532A_SendByte(CS5532Ax,StartConvert+SetupX+Single);			//���͵���ת������
	else
		CS5532A_SendByte(CS5532Ax,StartConvert+SetupX+Continuation);	//��������ת������
	
	CS5532A1_CS_HIGH();
	CS5532A2_CS_HIGH();

	return TRUE;
}

int32 CS5532Ax_GetValue(void)
{
	int32 ADCData = 0;
	CS5532A_TRByte(0x00);				  		//��SDOֵ
	ADCData = CS5532Ax_ReadWord();   			//��ADC����

	//��չ
	if((ADCData & 0x00800000) != 0){
		ADCData |= 0xFF000000;
	}

	return ADCData;        			  //24λ����
}

/****************************************************************************
* ��	�ƣ�u8 CS5532A1_Flag(void)
* ��	�ܣ��Ƿ�ת�����	��ѯ��ʽ
* ��ڲ�������
* ���ڲ�����u8	1ת�����
				0ת��δ���
* ˵	����ת����ɺ�CS5532��SDO����һֱΪ�͵�ƽ
****************************************************************************/
u8 CS5532A1_Flag(void)
{
	CS5532A2_CS_HIGH();
	CS5532A1_CS_LOW();	
	CS5532_CS_Delay(NS_50);
	if( Bit_RESET == CS5532A_READ_MISO() )  //ת�����
		return 1;
	else
		return 0;
}
u8 CS5532A2_Flag(void)
{
	CS5532A1_CS_HIGH();
	CS5532A2_CS_LOW();
	CS5532_CS_Delay(NS_50);
	if( Bit_RESET == CS5532A_READ_MISO() )  //ת�����
		return 1;
	else
		return 0;
}

/****************************************************************************
* ��	�ƣ�u8 fStepConfgAD_Ax( u8 u8Channel , u8 Step )
* ��	�ܣ���λ5532����ʼ��
* ��ڲ�����u8	Ҫ��ʼ����ͨ����
* ���ڲ�����u8	��ʼ������
* ˵	����
			��һ�ε��øú��� -����λ		Step = 1
			��ʱ20ms(�ⲿʵ��)
			�ڶ��ε��øú��� -��оƬ����	Step = 2
****************************************************************************/
u8 fStepConfgAD_Ax( u8 u8Channel , u8 Step )
{
	u8 ret=u8Channel;
    u32 temp;
	if( 1 == Step )		//��λ
	{
		CS5532A_SetCMD(u8Channel);			   					 //���ó�����ģʽ

		CS5532A_SendByte(u8Channel,ConfigRegister+WriteRegister);//�����üĴ�������д����
		CS5532A_SendWord(u8Channel,RS1);						 //��λ,��������ģʽ
		return true ;
	}
	//��ʱ20�����
	if( 2 == Step )		//����
	{
		//�����üĴ������ж�����	    0x10000000
		CS5532A_SendByte(u8Channel,ConfigRegister+ReadRegister);	    
		
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);					    //RVΪ1 0x10000000 ��λ��������	��ȡһ�κ�RV�Զ�Ϊ0
			if(temp!=0x10000000)
				ret &= ~CS5532A1;								    //��λ��־����ȷ
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);				        //RVΪ1 0x10000000 ��λ��������	��ȡһ�κ�RV�Զ�Ϊ0
			if(temp!=0x10000000)
				ret &= ~CS5532A2;								    //��λ��־����ȷ
		}
		if(!ret) {
			return ret;
		}
		
		//�����üĴ���  ��ʼ��ֵ��֤    0x00000000
		CS5532A_SendByte(u8Channel,ConfigRegister+ReadRegister);	    
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)                                   //���üĴ�����λĬ��ֵ
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)                                   //���üĴ�����λĬ��ֵ
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}
		
		//��ƫ�ƼĴ���1  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ExcursionRegister+Excursion_1+ReadRegister);  
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)                                   //ƫ�ƼĴ���1��λĬ��ֵ
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)                                   //ƫ�ƼĴ���1��λĬ��ֵ
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//��ƫ�ƼĴ���2  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ExcursionRegister+Excursion_2+ReadRegister);
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)                                   //ƫ�ƼĴ���2��λĬ��ֵ
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)                                   //ƫ�ƼĴ���2��λĬ��ֵ
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}
		
		//������Ĵ���1  ��ʼ��ֵ��֤   0x01000000
		CS5532A_SendByte(u8Channel,GainRegister+GainSet_1+ReadRegister);	    
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x01000000)                                   //����Ĵ���1��λĬ��ֵ
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x01000000)                                   //����Ĵ���1��λĬ��ֵ
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//������Ĵ���2  ��ʼ��ֵ��֤   0x01000000
		CS5532A_SendByte(u8Channel,GainRegister+GainSet_2+ReadRegister);			
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x01000000)                                   //����Ĵ���2��λĬ��ֵ
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x01000000)                                   //����Ĵ���2��λĬ��ֵ
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//��ͨ���Ĵ���1  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ChannelRegister+ChannelSet_1+ReadRegister);		
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//��ͨ���Ĵ���2  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ChannelRegister+ChannelSet_2+ReadRegister);		
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//��ͨ���Ĵ���3  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ChannelRegister+ChannelSet_3+ReadRegister);		
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)
				ret &= ~CS5532A2;
		}
		if(!ret) {
			return ret;
		}

		//��ͨ���Ĵ���4  ��ʼ��ֵ��֤   0x00000000
		CS5532A_SendByte(u8Channel,ChannelRegister+ChannelSet_4+ReadRegister);		
		if(u8Channel & ret & CS5532A1) {
			temp = CS5532A_ReadWord(CS5532A1);				        
			if(temp!=0x00000000)
				ret &= ~CS5532A1;
		}
		if(u8Channel & ret & CS5532A2) {
			temp = CS5532A_ReadWord(CS5532A2);	
			if(temp!=0x00000000)
				ret &= ~CS5532A2;
		}
	}
	
	CS5532A_ConfigRegister(ret);	//����	  ���üĴ���	  �ο���ѹ
	CS5532A_ChannelRegister(ret);	//����	  ͨ���Ĵ���	  ����ͨ�� + ���� + ������ + ��˫�� + ����λ + ��ʱ +  ��·��� +  ƫ��/��������ͨ��ѡ��

	CS5532A_Start(ret,1,2);			//��������ת��	
	
	return true;
}