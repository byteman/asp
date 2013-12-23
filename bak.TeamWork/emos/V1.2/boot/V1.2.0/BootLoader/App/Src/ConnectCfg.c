#include "ConnectCfg.h"
#include "printf_init.h"		//DebugPf
#include "crc_init.h"				//CRC
#include "static_init.h"		//ADDR
#include "rtc_init.h"				//RTC_AlarmA

/****************************************************************************
* 名	称：void BootLoaderOver(void)
* 功	能：BootLoader结束初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void BootLoaderOver(void)
{
	//关闭定时器6
	TIM_ITConfig(TIM6, TIM_FLAG_Update, DISABLE);						//关闭定时器中断
	TIM_Cmd(TIM6, DISABLE);																	//关闭定时器
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, DISABLE);		//关闭定时器电源
	
	//关闭RTC
	RTC_ITConfig(RTC_IT_ALRA,DISABLE);											//关闭报警A中断
	RTC_AlarmCmd(RTC_Alarm_A,DISABLE);											//关闭报警A
	RTC_ITConfig(RTC_IT_ALRB,DISABLE);											//关闭报警B中断
	RTC_AlarmCmd(RTC_Alarm_B,DISABLE);											//关闭报警A
	PWR_BackupAccessCmd(DISABLE);														//关闭RTC后备寄存器通道
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, DISABLE);		//关闭PWR时钟	
	RCC_RTCCLKCmd(DISABLE);																	//关闭RTC时钟来源
	
	//关闭串口1
	USART_ITConfig(USART1, USART_IT_RXNE, DISABLE);					//关闭接收中断
	USART_Cmd(USART1, DISABLE);  														//关闭串口1外设
	
	//关闭串口2
	USART_ITConfig(USART3, USART_IT_RXNE, DISABLE);					//关闭接收中断
	USART_Cmd(USART3, DISABLE);  														//关闭串口3外设

	//关闭串口6
	USART_ITConfig(USART6, USART_IT_RXNE, DISABLE);					//关闭接收中断
	USART_Cmd(USART6, DISABLE);  														//关闭串口6外设
}

/****************************************************************************
* 名	称：void TimDelayMS(const u16 DelayMS)
* 功	能：定时器的毫秒级延迟
* 入口参数：const u16 DelayMS	延迟时间量		毫秒
* 出口参数：无
* 说	明：无
****************************************************************************/
void TimDelayMS(const u16 DelayMS)
{
	u8 Sone = 0;																//一次时间
	u8 STwo = 0;																//二次时间
	
	Sone = (u8)(GetTimeMSecCnt() / DelayMS);		//一次时间获取
	
	do
	{
		STwo = (u8)(GetTimeMSecCnt() / DelayMS);	//二次时间获取
	}while(STwo == Sone);												//未超过1秒
}

/****************************************************************************
* 名	称：void BlockDataCheck(void)
* 功	能：数据块检查
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void BlockDataCheck(void)
{
	u32 RUNCRCOne = 0;
	u32 RUNCRCOneBak = 0;
	u32 RUNCRCDataSize = 0;

	//应用区代码检查
	DebugPf(0,"运行区固件效验..........");
	RUNCRCOne = ReadAppCRCAddress();				//读取存储的CRC效验值
	RUNCRCDataSize = ReadAppSizeAddress();	//读取数据大小
	if(RUNCRCDataSize < AppSpaceSize)				//检测数据大小是否超过存放区空间，超过则没有数据，没有数据时数据全FF
	{
		if(ReadAppUpSrcAddress() == PCBOOT)
			RUNCRCOneBak = CRC32H_Chack8(1,(u8*)((volatile u8*)AppStartAddress),RUNCRCDataSize);		//应用区计算获得的文件校验值		  //STM32硬件效验
		else if(ReadAppUpSrcAddress() == NETBOOT)
			;
		
		if(RUNCRCOne == RUNCRCOneBak)					//CRC比较
		{
			DebugPf(0,"OK!\r\n");									//数据完整
		}
		else
			DebugPf(0,"Error!\r\n");							//数据有误
	}
	else
		DebugPf(0,"No Data!\r\n");							//没有数据
	
	RUNCRCOne = 0;
	RUNCRCOneBak = 0;
	RUNCRCDataSize = 0;
	
	//GB字库检查
	DebugPf(0,"GB字库区数据效验........");			
	RUNCRCOne = ReadGBCRCAddress();					//读取存储的CRC效验值
	RUNCRCDataSize = ReadGBSizeAddress();		//读取数据大小
	if(RUNCRCDataSize < GBFontSpaceSize)		//检测数据大小是否超过存放区空间，超过则没有数据，没有数据时数据全FF
	{
		if(ReadGBUpSrcAddress() == PCBOOT)
			RUNCRCOneBak = CRC32H_Chack8(1,(u8*)((volatile u8*)GBFontStartAddress),RUNCRCDataSize);		//GB字库区计算获得的文件校验值
		else if(ReadGBUpSrcAddress() == NETBOOT)
			;
		
		if(RUNCRCOne == RUNCRCOneBak)					//CRC比较
		{
			DebugPf(0,"OK!\r\n");									//数据完整
		}
		else
			DebugPf(0,"Error!\r\n");							//数据有误
	}
	else
		DebugPf(0,"No Data!\r\n");							//没有数据
	
	RUNCRCOne = 0;
	RUNCRCOneBak = 0;
	RUNCRCDataSize = 0;
	
	//ASCII字库检查
	DebugPf(0,"ASCII字库区数据效验.....");
	RUNCRCOne = ReadASCIICRCAddress();				//读取存储的CRC效验值
	RUNCRCDataSize = ReadASCIISizeAddress();	//读取数据大小
	if(RUNCRCDataSize < ASCIIFontSpaceSize)		//检测数据大小是否超过存放区空间，超过则没有数据，没有数据时数据全FF
	{
		if(ReadASCIIUpSrcAddress() == PCBOOT)
			RUNCRCOneBak = CRC32H_Chack8(1,(u8*)((volatile u8*)ASCIIFontStartAddress),RUNCRCDataSize);		//ASCII字库区计算获得的文件校验值
		else if(ReadASCIIUpSrcAddress() == NETBOOT)
			;
		
		if(RUNCRCOne == RUNCRCOneBak)						//CRC比较
		{	
			DebugPf(0,"OK!\r\n");										//数据完整
		}
		else
			DebugPf(0,"Error!\r\n");								//数据有误
	}
	else
		DebugPf(0,"No Data!\r\n");								//没有数据
}

/****************************************************************************
* 名	称：u32 AutoBaudRateCheck(const u8 Val)
* 功	能：自动波特率识别检查
* 入口参数：const u8 Val		识别字符
* 出口参数：u32				波特率值
* 说	明：无
****************************************************************************/
u32 AutoBaudRateCheck(const u8 Val)
{
	u32 BaudRateVal = BootLoaderUsart_BaudRate;			//波特率值最开始为设定值
	u8 selfval[2] = {0};		//自适应找' '值，只会有一个值，大于1个则属于波特率过低
	u8 selfCnt = 0;					//自适应自加
	u8 selflen = 0;					//收到的字符数
	u8 Time03S = 0;					//0.3秒检测		检测20次 = 6秒

	while(1)																	//自适应波特率
	{
		BootLoaderUsart_RX_Clear();							//清空接收缓冲区
		
		TimDelayMS(300);												//接收等待0.3秒
		Time03S++;
		if(Time03S >= 20)												//0.3/次 * 20次 = 6S
			return 0;															//没检测到
			
		selflen = BootLoaderUsart_GetString(selfval,2);	//从缓冲区获取字符
		if(selflen >= 1)												//收到数据
		{
			if(selfval[0] == Val)									//判断字符是否正确
			{
				BootLoaderUsart_RX_Clear();					//清空接收缓冲区
				return BaudRateVal;									//正确 返回波特率值
			}
		}
		
		selfCnt++;															//字符有错 改变波特率
		
		switch(selfCnt)
		{
			case 1:		//2400
			{
				BaudRateVal = 2400;
				break;
			}
			case 2:		//4800
			{
				BaudRateVal = 4800;
				break;
			}
			case 3:		//9600
			{
				BaudRateVal = 9600;
				break;
			}
			case 4:		//10416
			{
				BaudRateVal = 10416;
				break;
			}
			case 5:		//19200
			{
				BaudRateVal = 19200;
				break;
			}
			case 6:		//38400
			{
				BaudRateVal = 38400;
				break;
			}
			case 7:		//57600
			{
				BaudRateVal = 57600;
				break;
			}
			case 8:		//115200
			{
				BaudRateVal = 115200;
				break;
			}
			default:
			{
				selfCnt = 0;		//恢复2400再来
				break;
			}
		}
		
		BootLoaderUsart_Init(BaudRateVal);		//重新设置串口波特率
	}
}
