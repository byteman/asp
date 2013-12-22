#include "lcd_init.h"
#include "includes.h"

#define LCD_SX 								240				//宽
#define LCD_SY 								128				//高
#define LCD_BYTES_X  					30 				//显示区宽度
#define TEXT_HOME_ADDRESS 		0x0000 		//文本显示区首地址
#define GRAPHIC_HOME_ADDRESS 	0x01E0 		//图形显示区首地址

#define RowFlag			0											//顺序标志
#define ChaosFlag		1											//乱序标志

typedef struct
{
	GPIO_TypeDef* LCDPor;				//端口号			GPIOA - GPIOI
	u16 LCDPin;									//引脚号			GPIO_Pin_0 - GPIO_Pin_15
}LCD_IO;											//LCD IO引脚

//数据引脚
static LCD_IO LCD_DataLine[8] = 							
{
	{GPIOG,GPIO_Pin_0},					//0
	{GPIOG,GPIO_Pin_1},					//1
	{GPIOG,GPIO_Pin_2},					//2
	{GPIOG,GPIO_Pin_3},					//3
	{GPIOG,GPIO_Pin_4},					//4
	{GPIOG,GPIO_Pin_5},					//5
	{GPIOG,GPIO_Pin_6},					//6
	{GPIOG,GPIO_Pin_7}					//7	
};
static u8 DataLineOffset = 0;	//数据线偏移量	1 = 1个引脚

//控制引脚
static LCD_IO WR 			= {GPIOG,GPIO_Pin_8};			//LCD_WR				//写信号			L有效
static LCD_IO RD 			= {GPIOG,GPIO_Pin_9};			//LCD_RD				//读信号			L有效
static LCD_IO CE 			= {GPIOG,GPIO_Pin_10};		//LCD_CE				//使能				L有效
static LCD_IO CD 			= {GPIOG,GPIO_Pin_11};		//LCD_CD				//命令or数据	H命令 L数据
static LCD_IO RST 		= {GPIOG,GPIO_Pin_12};		//LCD_RST				//复位				L有效
static LCD_IO FS 			= {GPIOG,GPIO_Pin_13};		//LCD_FS				//字体				
static LCD_IO LED 		= {GPIOG,GPIO_Pin_15}; 		//LCD_LED				//背光				H背光亮 L背光灭		
static LCD_IO AD_CTRL = {GPIOC,GPIO_Pin_8};			//LCD_AD_CTRL		//控制引脚

static u8 DataLineChaosRowFlag = RowFlag;				//顺序还是乱序	自动判断
static GPIO_TypeDef* DataLineSamePort = 0;			//同一端口号		自动判断

/****************************************************************************
* 名	称：void LCD_Delay(u16 nDly)
* 功	能：延时
* 入口参数：延时时长
* 出口参数：无
* 说	明：无		
****************************************************************************/
void LCD_Delay(u16 nDly)
{
	u32 temp = nDly * 2;
	while(temp--);
}

/****************************************************************************
* 名	称：u8 LCD_DataLineRead(void)
* 功	能：读数据线上的数据
* 入口参数：无
* 出口参数：u8		8位数据线上的8位数据
* 说	明：无
****************************************************************************/
u8 LCD_DataLineRead(void)
{
	u16 Readu16 = 0;
	u8 Readu8 = 0;
	
	GPIO_SetBits(AD_CTRL.LCDPor,AD_CTRL.LCDPin);		
	
	if(DataLineSamePort != (GPIO_TypeDef*)0)		//如果是同一端口
	{
		if(DataLineChaosRowFlag == RowFlag)				//如果是顺序引脚
		{
			Readu16 = GPIO_ReadInputData(DataLineSamePort);					//读端口上的16位数据
			Readu8 = (u8)((Readu16 >> DataLineOffset) & 0x00FF);		//取低8位数据
		}
		else		//非顺序
		{
			Readu8 |=  GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[0].LCDPin);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[1].LCDPin) << 1);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[2].LCDPin) << 2);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[3].LCDPin) << 3);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[4].LCDPin) << 4);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[5].LCDPin) << 5);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[6].LCDPin) << 6);
			Readu8 |= (GPIO_ReadInputDataBit(DataLineSamePort,LCD_DataLine[7].LCDPin) << 7);
		}
	}
	else	//不同端口
	{
		Readu8 |=  GPIO_ReadInputDataBit(LCD_DataLine[0].LCDPor,LCD_DataLine[0].LCDPin);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[1].LCDPor,LCD_DataLine[1].LCDPin) << 1);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[2].LCDPor,LCD_DataLine[2].LCDPin) << 2);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[3].LCDPor,LCD_DataLine[3].LCDPin) << 3);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[4].LCDPor,LCD_DataLine[4].LCDPin) << 4);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[5].LCDPor,LCD_DataLine[5].LCDPin) << 5);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[6].LCDPor,LCD_DataLine[6].LCDPin) << 6);
		Readu8 |= (GPIO_ReadInputDataBit(LCD_DataLine[7].LCDPor,LCD_DataLine[7].LCDPin) << 7);
	}
	
	GPIO_ResetBits(AD_CTRL.LCDPor,AD_CTRL.LCDPin);	
	
	return Readu8;																					//返回低8位数据
}

/****************************************************************************
* 名	称：u8 LCDCheckBusy(u8 nMask)
* 功	能：忙检测
* 入口参数：u16 nMask			忙关键字
* 出口参数：u8 	是否忙		1忙 0闲
* 说	明：无
****************************************************************************/
u8 LCDCheckBusy(u8 Mask)
{
	u8 Val = 0;			//LCD状态值
	u16 i;					//累加器

	GPIO_SetBits(CD.LCDPor,CD.LCDPin);			//数据
	GPIO_ResetBits(RD.LCDPor,RD.LCDPin);		//读允许
	
  for(i = 0;i < 10000;i++)								//允许失败次数
	{
		Val = LCD_DataLineRead();							//读取8位数据
		if((Val & Mask) == Mask)
			break;
	}
	GPIO_SetBits(RD.LCDPor,RD.LCDPin);			//读不允许
	
	if(i > 10000)
		return 1;					//忙
	else
		return 0;					//闲
}

/****************************************************************************
* 名	称：void LCD_DataLineOut(void)
* 功	能：将LCD数据线配置为通用输出模式
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void LCD_DataLineOut(void)
{
	u8 i = 0;			//累加器
	u8 No = 0;		//引脚号
	u16 Pin = 0;	//临时引脚值
	
	if(DataLineSamePort != (GPIO_TypeDef*)0)		//如果是同一端口
	{
		if(DataLineChaosRowFlag == RowFlag)				//如果是顺序引脚
			DataLineSamePort->MODER |= 0x00005555 << (DataLineOffset * 2); 							//连续性引脚模式为输出01
		else																			//非顺序
		{
			for(i = 0; i < 8; i++)									//遍历引脚
			{
				No = 0;																//引脚号
				Pin = LCD_DataLine[i].LCDPin;					//临时引脚号
				
				while(1)
				{
					if((Pin & 0x01) == 0x01)						//是否找到引脚
						break;
					else
					{	
						Pin = Pin >> 1;										//引脚号位移
						if(No < 15)												//没有超过最大引脚号
							No++;														//引脚号
						else
							break;
					}
				}
				DataLineSamePort->MODER |= 0x0001 << (No * 2); 		//非连续性引脚模式输出01
			}
		}
	}
	else	//不同端口
	{
		for(i = 0; i < 8; i++)									//遍历引脚
		{
			No = 0;																//引脚号
			Pin = LCD_DataLine[i].LCDPin;					//临时引脚号
			
			while(1)
			{
				if((Pin & 0x01) == 0x01)						//是否找到引脚
					break;
				else
				{	
					Pin = Pin >> 1;										//引脚号位移
					if(No < 15)												//没有超过最大引脚号
						No++;														//引脚号
					else
						break;
				}
			}
			LCD_DataLine[i].LCDPor->MODER |= 0x0001 << (No * 2); 		////不同端口引脚模式输出01
		}
	}
}

/****************************************************************************
* 名	称：void LCD_DataLineIn(void)
* 功	能：将LCD数据线配置为通用输入模式
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void LCD_DataLineIn(void)
{
	u8 i;							//累加器
	u16 uTmp = 0;			//移位后的补码
	u16 PinNo = 0;		//管脚号
	
	if(DataLineSamePort != (GPIO_TypeDef*)0)		//如果是同一端口
	{
		if(DataLineChaosRowFlag == RowFlag)				//如果是顺序引脚
		{	
			for(i = 0;i < DataLineOffset;i++)				//根据移位计算出补码
				uTmp = (uTmp << 2) + 1;
			DataLineSamePort->MODER &= 0xFFFF0000 << (DataLineOffset * 2) | uTmp; 		//改变引脚模式为输入00
		}
		else						//非顺序
		{
			for(i = 0; i < 8; i++)		//遍历引脚
			{
				PinNo = LCD_DataLine[i].LCDPin / 2;		//找引脚号
				for(i = 0;i < PinNo;i++)							//根据移位计算出补码
					uTmp = (uTmp << 2) + 1;
				DataLineSamePort->MODER |= 0x00 << (PinNo * 2) | uTmp; 									//非连续性引脚模式输出00
			}
		}
	}
	else	//不同端口
	{
		for(i = 0; i < 8; i++)		//遍历引脚
		{
			PinNo = LCD_DataLine[i].LCDPin / 2;		//找引脚号
			for(i = 0;i < PinNo;i++)							//根据移位计算出补码
				uTmp = (uTmp << 2) + 1;
			LCD_DataLine[i].LCDPor->MODER |= 0x00 << (PinNo * 2) | uTmp; 							//不同端口引脚模式输出00
		}
	}
}

/****************************************************************************
* 名	称：void LCD_DataLineWrite(u8 uData)
* 功	能：给数据线写数据
* 入口参数：u16 uData				8位数据
* 出口参数：
* 说	明：无
****************************************************************************/
void LCD_DataLineWrite(u8 uData)
{
	u8 i = 0;						//累加器
	u16 nTmp = uData;		//需要发送的数据
	
	if(DataLineSamePort != (GPIO_TypeDef*)0)		//如果是同一端口
	{
		if(DataLineChaosRowFlag == RowFlag)				//如果是顺序引脚
		{
			DataLineSamePort->BSRRH = 0x00FF << DataLineOffset;			//清空数据
			DataLineSamePort->BSRRL = nTmp << DataLineOffset ;			//写入数据
		}
		else											//非顺序
		{
			for(i = 0;i < 8;i++)		//遍历引脚
			{
				if((uData >> i) & 0x01 == 1)
					GPIO_SetBits(DataLineSamePort,LCD_DataLine[i].LCDPin);			//拉高
				else
					GPIO_ResetBits(DataLineSamePort,LCD_DataLine[i].LCDPin);		//拉低
			}
		}
	}
	else												//不同端口
	{
		for(i = 0;i < 8;i++)			//遍历引脚
		{
			if((uData >> i) & 0x01 == 1)
				GPIO_SetBits(LCD_DataLine[i].LCDPor,LCD_DataLine[i].LCDPin);			//拉高
			else
				GPIO_ResetBits(LCD_DataLine[i].LCDPor,LCD_DataLine[i].LCDPin);		//拉低
		}
	}
}

/****************************************************************************
* 名	称：	u8 LCD_WriteData(u8 nData)
* 功	能：	写LCD命令
* 入口参数：u8	nData		数据
* 出口参数：u8	发送命令是否成功			1成功	0失败
* 说	明：	失败为忙状态
****************************************************************************/
u8 LCD_WriteData(u8 nData)
{
	if(LCDCheckBusy(0x03) == 1)						//忙检测
		return 0;
	
	LCD_DataLineOut();										//数据线切换成输出模式
	GPIO_ResetBits(CD.LCDPor,CD.LCDPin);	//数据
	GPIO_ResetBits(WR.LCDPor,WR.LCDPin);	//写允许
	
	LCD_DataLineWrite(nData);							//发送数据
	LCD_Delay(80);												//延时			等待写完成

  GPIO_SetBits(WR.LCDPor,WR.LCDPin);		//不允许写
	LCD_DataLineIn();											//数据线切换成输入模式
		
	return 1;
}

/****************************************************************************
* 名	称：	u8 LCD_WriteCommand(u8 nCmd)
* 功	能：	写LCD命令
* 入口参数：u8	nCmd 		命令
* 出口参数：u8	发送命令是否成功			1成功	0失败
* 说	明：	失败为忙状态
****************************************************************************/
u8 LCD_WriteCommand(u8 nCmd)
{
	if(LCDCheckBusy(0x03) == 1)						//忙检测
		return 0;
	
	LCD_DataLineOut();										//数据线切换成输出模式
	GPIO_SetBits(CD.LCDPor,CD.LCDPin);		//命令
	GPIO_ResetBits(WR.LCDPor,WR.LCDPin);	//写允许
	
	LCD_DataLineWrite(nCmd);							//发送数据
	LCD_Delay(80);												//延时	
	
  GPIO_SetBits(WR.LCDPor,WR.LCDPin);		//不允许写
	LCD_DataLineIn();											//数据线切换成输入模式	
	return 1;
}

/****************************************************************************
* 名	称：	u8 LCD_WriteDataAuto(u8 nData)
* 功	能：	写LCD数据自动
* 入口参数：u8	nData		数据
* 出口参数：u8	发送命令是否成功			1成功	0失败
* 说	明：	无
****************************************************************************/
u8 LCD_WriteDataAuto(u8 nData)
{
	if(LCDCheckBusy(0x08) == 1)
		return 0;
	
	LCD_DataLineOut();										//数据线切换成输出模式
	GPIO_ResetBits(CD.LCDPor,CD.LCDPin);	//数据
	GPIO_ResetBits(WR.LCDPor,WR.LCDPin);	//写允许
	
	LCD_DataLineWrite(nData);							//发送数据
	LCD_Delay(80);												//延时

  GPIO_SetBits(WR.LCDPor,WR.LCDPin);		//不允许写
	LCD_DataLineIn();											//数据线切换成输入模式
	
	return 1;
}

/****************************************************************************
* 名	称：	u8 send_2data_and_1cmd_to_lcm(u8 DataL,u8 DataH,u8 Cmd)
* 功	能：	写2个8位数据和1个命令到LCM
* 入口参数：u8 DataL			低8位数据
						u8 DataH			高8位数据
						u8 Cmd				命令
* 出口参数：u8	发送命令是否成功			1成功	0失败
* 说	明：	无
****************************************************************************/
u8 send_2data_and_1cmd_to_lcm(u8 DataL,u8 DataH,u8 Cmd)
{
	if(LCD_WriteData(DataL) == 0)	return 0;		//低8位数据
	if(LCD_WriteData(DataH) == 0)	return 0;		//高8位数据
	if(LCD_WriteCommand(Cmd) == 0) return 0;	//8位命令
	
	return 1;
}

/****************************************************************************
* 名	称：	u8 send_1hex_data_and_1cmd_to_lcm(u16 Data,u8 Cmd)
* 功	能：	写1个16位数据和1个命令到LCM
* 入口参数：u8 DataL			低8位数据
						u8 DataH			高8位数据
						u8 Cmd				命令
* 出口参数：u8	发送命令是否成功			1成功	0失败
* 说	明：	无
****************************************************************************/
u8 send_1hex_data_and_1cmd_to_lcm(u16 Data,u8 Cmd)
{
	u8 DataH = Data >> 8;
	u8 DataL = Data;
	
	if(LCD_WriteData(DataL) == 0)	return 0;		//低8位数据
	if(LCD_WriteData(DataH) == 0)	return 0;		//高8位数据
	if(LCD_WriteCommand(Cmd) == 0) return 0;	//8位命令
	
	return 1;
}

/****************************************************************************
* 名	称：	u8 ClearScreen(void)
* 功	能：	清屏
* 入口参数：无
* 出口参数：u8 清屏是否成功		1成功 0失败
* 说	明：	无
****************************************************************************/
u8 ClearScreen(void)
{
	u8 i,j;

	//文本显示区首地址	//地址指针位置(设置读写操作指针)
	if(send_1hex_data_and_1cmd_to_lcm(TEXT_HOME_ADDRESS,LCD_ADR_POS) == 0)	
		return 0;		
	
	if(LCD_WriteCommand(LCD_AUT_WR) == 0)		//自动写设置 
		return 0;

	for(j = 0; j < 144; j++)	   						//参数为144同时清除图形区部分
	{
		for(i = 0; i < 30; i++)
		{
			if(LCD_WriteDataAuto(0x00) == 0)		//自动写	清空RAM
				return 0;
		}
	}

	if(LCD_WriteCommand(LCD_AUT_OVR) == 0)	//自动读写结束 
		return 0;
	
	return 1;
}

/****************************************************************************
* 名	称：void DataLine_Check(void)
* 功	能：数据线检查
* 入口参数：无
* 出口参数：无
* 说	明：1：检查是否处于同一端口上
					2：引脚号是否连续
****************************************************************************/
void DataLine_Check(void)
{
	u8 i;		//累加器
	u8 CRFlag = 0;				//顺序乱序查询标志
	u8 PortFlag = 0;			//端口查询标志
	
	DataLineChaosRowFlag = RowFlag;						//顺序还是乱序
	DataLineSamePort = (GPIO_TypeDef*)0;			//同一端口号
	
	//遍历引脚
	for(i = 0 ; i < 8 ; i++)
	{	
		if(CRFlag == 0)			//没有找过顺序还是乱序
		{
			if(LCD_DataLine[0].LCDPin << i != LCD_DataLine[i].LCDPin)		//如果GPIO	Pin不连续增加则跳出
			{
				DataLineChaosRowFlag = ChaosFlag;		//乱序
				CRFlag = 1;												//已找过顺序乱序
			}
		}
		if(PortFlag == 0)		//没有找过端口是否一致
		{
			if(LCD_DataLine[0].LCDPor != LCD_DataLine[i].LCDPor)				//如果GPIO PORT不一致则跳出
			{
				DataLineSamePort = (GPIO_TypeDef*)0;			//非同一端口号		端口清除
				PortFlag = 1;															//已找过端口一致性
			}
		}
	}
	
	//遍历完所有引脚
	if(CRFlag == 0)		//前期没出现乱序
		DataLineChaosRowFlag = RowFlag;								//顺序
	
	if(PortFlag == 0)	//前期没出现端口非一致性
		DataLineSamePort = LCD_DataLine[0].LCDPor;		//一致
}

/****************************************************************************
* 名	称：u8 LCD_PortInit(void)
* 功	能：引脚初始化
* 入口参数：无
* 出口参数：u8	初始化是否成功			1成功	0失败
* 说	明：无		
****************************************************************************/
u8 LCD_PortInit(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	//开启引脚时钟
	if((LCD_DataLine[0].LCDPor == GPIOA) || (LCD_DataLine[1].LCDPor == GPIOA) || (LCD_DataLine[2].LCDPor == GPIOA) || (LCD_DataLine[3].LCDPor == GPIOA) || (LCD_DataLine[4].LCDPor == GPIOA) || (LCD_DataLine[5].LCDPor == GPIOA) || (LCD_DataLine[6].LCDPor == GPIOA) || (LCD_DataLine[7].LCDPor == GPIOA)   
		||(WR.LCDPor == GPIOA) || (RD.LCDPor == GPIOA) || (CE.LCDPor == GPIOA) || (CD.LCDPor == GPIOA) || (RST.LCDPor == GPIOA) || (FS.LCDPor == GPIOA) || (LED.LCDPor == GPIOA) || (AD_CTRL.LCDPor == GPIOA) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);}		//开启引脚时钟A	
	if((LCD_DataLine[0].LCDPor == GPIOB) || (LCD_DataLine[1].LCDPor == GPIOB) || (LCD_DataLine[2].LCDPor == GPIOB) || (LCD_DataLine[3].LCDPor == GPIOB) || (LCD_DataLine[4].LCDPor == GPIOB) || (LCD_DataLine[5].LCDPor == GPIOB) || (LCD_DataLine[6].LCDPor == GPIOB) || (LCD_DataLine[7].LCDPor == GPIOB)   
		||(WR.LCDPor == GPIOB) || (RD.LCDPor == GPIOB) || (CE.LCDPor == GPIOB) || (CD.LCDPor == GPIOB) || (RST.LCDPor == GPIOB) || (FS.LCDPor == GPIOB) || (LED.LCDPor == GPIOB) || (AD_CTRL.LCDPor == GPIOB) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);}		//开启引脚时钟B	
	if((LCD_DataLine[0].LCDPor == GPIOC) || (LCD_DataLine[1].LCDPor == GPIOC) || (LCD_DataLine[2].LCDPor == GPIOC) || (LCD_DataLine[3].LCDPor == GPIOC) || (LCD_DataLine[4].LCDPor == GPIOC) || (LCD_DataLine[5].LCDPor == GPIOC) || (LCD_DataLine[6].LCDPor == GPIOC) || (LCD_DataLine[7].LCDPor == GPIOC)   
		||(WR.LCDPor == GPIOC) || (RD.LCDPor == GPIOC) || (CE.LCDPor == GPIOC) || (CD.LCDPor == GPIOC) || (RST.LCDPor == GPIOC) || (FS.LCDPor == GPIOC) || (LED.LCDPor == GPIOC) || (AD_CTRL.LCDPor == GPIOC) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);}		//开启引脚时钟C	
	if((LCD_DataLine[0].LCDPor == GPIOD) || (LCD_DataLine[1].LCDPor == GPIOD) || (LCD_DataLine[2].LCDPor == GPIOD) || (LCD_DataLine[3].LCDPor == GPIOD) || (LCD_DataLine[4].LCDPor == GPIOD) || (LCD_DataLine[5].LCDPor == GPIOD) || (LCD_DataLine[6].LCDPor == GPIOD) || (LCD_DataLine[7].LCDPor == GPIOD)   
		||(WR.LCDPor == GPIOD) || (RD.LCDPor == GPIOD) || (CE.LCDPor == GPIOD) || (CD.LCDPor == GPIOD) || (RST.LCDPor == GPIOD) || (FS.LCDPor == GPIOD) || (LED.LCDPor == GPIOD) || (AD_CTRL.LCDPor == GPIOD) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);}		//开启引脚时钟D
	if((LCD_DataLine[0].LCDPor == GPIOE) || (LCD_DataLine[1].LCDPor == GPIOE) || (LCD_DataLine[2].LCDPor == GPIOE) || (LCD_DataLine[3].LCDPor == GPIOE) || (LCD_DataLine[4].LCDPor == GPIOE) || (LCD_DataLine[5].LCDPor == GPIOE) || (LCD_DataLine[6].LCDPor == GPIOE) || (LCD_DataLine[7].LCDPor == GPIOE)   
		||(WR.LCDPor == GPIOE) || (RD.LCDPor == GPIOE) || (CE.LCDPor == GPIOE) || (CD.LCDPor == GPIOE) || (RST.LCDPor == GPIOE) || (FS.LCDPor == GPIOE) || (LED.LCDPor == GPIOE) || (AD_CTRL.LCDPor == GPIOE) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);}		//开启引脚时钟E
	if((LCD_DataLine[0].LCDPor == GPIOF) || (LCD_DataLine[1].LCDPor == GPIOF) || (LCD_DataLine[2].LCDPor == GPIOF) || (LCD_DataLine[3].LCDPor == GPIOF) || (LCD_DataLine[4].LCDPor == GPIOF) || (LCD_DataLine[5].LCDPor == GPIOF) || (LCD_DataLine[6].LCDPor == GPIOF) || (LCD_DataLine[7].LCDPor == GPIOF)   
		||(WR.LCDPor == GPIOF) || (RD.LCDPor == GPIOF) || (CE.LCDPor == GPIOF) || (CD.LCDPor == GPIOF) || (RST.LCDPor == GPIOF) || (FS.LCDPor == GPIOF) || (LED.LCDPor == GPIOF) || (AD_CTRL.LCDPor == GPIOF) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);}		//开启引脚时钟F
	if((LCD_DataLine[0].LCDPor == GPIOG) || (LCD_DataLine[1].LCDPor == GPIOG) || (LCD_DataLine[2].LCDPor == GPIOG) || (LCD_DataLine[3].LCDPor == GPIOG) || (LCD_DataLine[4].LCDPor == GPIOG) || (LCD_DataLine[5].LCDPor == GPIOG) || (LCD_DataLine[6].LCDPor == GPIOG) || (LCD_DataLine[7].LCDPor == GPIOG)   
		||(WR.LCDPor == GPIOG) || (RD.LCDPor == GPIOG) || (CE.LCDPor == GPIOG) || (CD.LCDPor == GPIOG) || (RST.LCDPor == GPIOG) || (FS.LCDPor == GPIOG) || (LED.LCDPor == GPIOG) || (AD_CTRL.LCDPor == GPIOG) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);}		//开启引脚时钟G
	if((LCD_DataLine[0].LCDPor == GPIOH) || (LCD_DataLine[1].LCDPor == GPIOH) || (LCD_DataLine[2].LCDPor == GPIOH) || (LCD_DataLine[3].LCDPor == GPIOH) || (LCD_DataLine[4].LCDPor == GPIOH) || (LCD_DataLine[5].LCDPor == GPIOH) || (LCD_DataLine[6].LCDPor == GPIOH) || (LCD_DataLine[7].LCDPor == GPIOH)   
		||(WR.LCDPor == GPIOH) || (RD.LCDPor == GPIOH) || (CE.LCDPor == GPIOH) || (CD.LCDPor == GPIOH) || (RST.LCDPor == GPIOH) || (FS.LCDPor == GPIOH) || (LED.LCDPor == GPIOH) || (AD_CTRL.LCDPor == GPIOH) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);}		//开启引脚时钟H
	if((LCD_DataLine[0].LCDPor == GPIOI) || (LCD_DataLine[1].LCDPor == GPIOI) || (LCD_DataLine[2].LCDPor == GPIOI) || (LCD_DataLine[3].LCDPor == GPIOI) || (LCD_DataLine[4].LCDPor == GPIOI) || (LCD_DataLine[5].LCDPor == GPIOI) || (LCD_DataLine[6].LCDPor == GPIOI) || (LCD_DataLine[7].LCDPor == GPIOI)   
		||(WR.LCDPor == GPIOI) || (RD.LCDPor == GPIOI) || (CE.LCDPor == GPIOI) || (CD.LCDPor == GPIOI) || (RST.LCDPor == GPIOI) || (FS.LCDPor == GPIOI) || (LED.LCDPor == GPIOI) || (AD_CTRL.LCDPor == GPIOI) )
		{RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);}		//开启引脚时钟I		

	//引脚模式推挽输出
	GPIO_InitStructure.GPIO_Mode 	= GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  
		
	//控制引脚
	GPIO_InitStructure.GPIO_Pin   = WR.LCDPin;				//LCD_WR
	GPIO_Init(WR.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = RD.LCDPin;				//LCD_RD
	GPIO_Init(RD.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = CE.LCDPin;				//LCD_CE
	GPIO_Init(CE.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = CD.LCDPin;				//LCD_CD
	GPIO_Init(CD.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = RST.LCDPin;				//LCD_RST
	GPIO_Init(RST.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = FS.LCDPin;				//LCD_FS
	GPIO_Init(FS.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LED.LCDPin;				//LCD_LED
	GPIO_Init(LED.LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = AD_CTRL.LCDPin;		//LCD_AD_CTRL
	GPIO_Init(AD_CTRL.LCDPor, &GPIO_InitStructure);
	
	//数据引脚
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[0].LCDPin;		//LCD_D0
	GPIO_Init(LCD_DataLine[0].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[1].LCDPin;		//LCD_D1
	GPIO_Init(LCD_DataLine[1].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[2].LCDPin;		//LCD_D2
	GPIO_Init(LCD_DataLine[2].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[3].LCDPin;		//LCD_D3
	GPIO_Init(LCD_DataLine[3].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[4].LCDPin;		//LCD_D4
	GPIO_Init(LCD_DataLine[4].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[5].LCDPin;		//LCD_D5
	GPIO_Init(LCD_DataLine[5].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[6].LCDPin;		//LCD_D6
	GPIO_Init(LCD_DataLine[6].LCDPor, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin   = LCD_DataLine[7].LCDPin;		//LCD_D7
	GPIO_Init(LCD_DataLine[7].LCDPor, &GPIO_InitStructure);
	
	GPIO_ResetBits(RST.LCDPor,RST.LCDPin);			//复位
	GPIO_SetBits(RST.LCDPor,RST.LCDPin);				//复位无效
	GPIO_SetBits(CE.LCDPor,CE.LCDPin);					//芯片无效
	GPIO_SetBits(WR.LCDPor,WR.LCDPin);					//写无效
	GPIO_SetBits(RD.LCDPor,RD.LCDPin);					//读无效
	GPIO_SetBits(CD.LCDPor,CD.LCDPin);					//数据模式
	GPIO_ResetBits(FS.LCDPor,FS.LCDPin);				//使用8*8文字模式

	DataLine_Check();				//端口一致性和端口连续性检测
	
	GPIO_ResetBits(RST.LCDPor,RST.LCDPin);						//复位
	SysTimeDly(1);					//保持5个以上时钟周期即可复位
  GPIO_SetBits(RST.LCDPor,RST.LCDPin);							//复位完毕
	
	GPIO_ResetBits(CE.LCDPor,CE.LCDPin);							//使能LCD
	
	//初始化设置
	if(send_1hex_data_and_1cmd_to_lcm(TEXT_HOME_ADDRESS, LCD_TXT_STP) == 0) 		return 0; //文本显示区首地址
	if(send_2data_and_1cmd_to_lcm(LCD_BYTES_X, 0x00, LCD_TXT_WID) == 0) 				return 0;	//文本显示区宽度	
	if(send_1hex_data_and_1cmd_to_lcm(GRAPHIC_HOME_ADDRESS, LCD_GRH_STP) == 0) 	return 0;	//图形显示区首地址	
	if(send_2data_and_1cmd_to_lcm(LCD_BYTES_X, 0x00, LCD_GRH_WID) == 0) 				return 0; //图形显示区宽度	
	if(send_2data_and_1cmd_to_lcm(0x02,0x00,LCD_CGR_POS) == 0) 									return 0; //CGRAM偏置地址设置	
	if(LCD_WriteCommand(LCD_MOD_OR) == 0) 																			return 0;	//逻辑"或"	
	if(ClearScreen() == 0)																											return 0;	//先清屏，再开显示		
	if(LCD_WriteCommand(LCD_DIS_GON) == 0) 																			return 0; //开显示  												//启用图形显示
		
	return 1;
}

/****************************************************************************
* 名	称：void SetLCDLED(u8 OnOff)
* 功	能：设置背光
* 入口参数：u8 OnOff		背光开关		1开0关
* 出口参数：无
* 说	明：无		
****************************************************************************/
void SetLCDLED(u8 OnOff)
{
	if(OnOff > 0)
		GPIO_SetBits(LED.LCDPor,LED.LCDPin);				//打开背光
	else
		GPIO_ResetBits(LED.LCDPor,LED.LCDPin);			//关闭背光
}

/****************************************************************************
* 名	称：u8 LCD_init(void)
* 功	能：引脚初始化
* 入口参数：无
* 出口参数：u8	初始化是否成功			1成功	0失败
* 说	明：无背光则有可能初始化未通过	
****************************************************************************/
u8 LCD_init(void)
{
	if(LCD_PortInit() == 0)			//引脚初始化
		return 0;	
	SetLCDLED(1);								//初始化不通过不会打开背光	
	return 1;
}

/****************************************************************************
* 名	称：	u8 SetAddr(u16 offset)
* 功	能：	图形显示区首地址偏移量
* 入口参数：u16 offset			//偏移地址
* 出口参数：u8		是否执行成功		1成功	0失败
* 说	明：	无
****************************************************************************/
u8 SetAddr(u16 offset)
{
	if(send_1hex_data_and_1cmd_to_lcm(GRAPHIC_HOME_ADDRESS + offset,LCD_ADR_POS) == 0)		//图形显示区首地址	//地址指针位置(设置读写操作)  
		return 0;
	else
		return 1;
}
