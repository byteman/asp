#include "eth_init.h"
#include "stm32f4xx.h"
#include "stm32f4x7_eth.h"
#include "driver.h"

#define DP83848_PHY_ADDRESS       0x01
#define PHY_MICR                  0x11 								//MII中断控制寄存器
#define PHY_MICR_INT_EN           ((u16)0x0002) 			//PHY开启中断
#define PHY_MICR_INT_OE           ((u16)0x0001) 			//PHY开启输出中断信号
#define PHY_MISR                  0x12 								//MII中断状态和MISC控制寄存器
#define PHY_MISR_LINK_INT_EN      ((u16)0x0020) 			//启用链接状态改变中断
#define PHY_LINK_STATUS           ((u16)0x2000) 			//PHY连接状态中断标志位

#define ETH_LINK_EXTI_LINE             EXTI_Line0
#define ETH_LINK_EXTI_PORT_SOURCE      EXTI_PortSourceGPIOC
#define ETH_LINK_EXTI_PIN_SOURCE       EXTI_PinSource0
#define ETH_LINK_EXTI_IRQn             EXTI0_IRQn 

#define ETH_LINK_PIN                   GPIO_Pin_0
#define ETH_LINK_GPIO_PORT             GPIOC
#define ETH_LINK_GPIO_CLK              RCC_AHB1Periph_GPIOC

__IO uint8_t EthLinkStatus = 0;

//中断注册信息表
static SDEV_IRQ_REGS ETH_IRQ = 
{
	0,								//中断线号
	0,								//中断线开启标志
	(pIRQHandle)0,		//中断句柄
};

/****************************************************************************
* 名	称：void ETH_GPIO_Config(void)
* 功	能：以太网引脚初始化
* 入口参数：无
* 出口参数：无
* 说	明：EtherNet
因RMII数据线少MII一半，故需要在时钟频率上提高一倍才能和MII达到同样的数据收发
因GMII数据线多MII一倍，故在同样数据量收发的情况下所需的时钟频率只要MII的一半
RMII:2根线传数据		10M线速(RMII=5M)		100M线速(RMII=50M)
MII:需要16个信号,4根线传数据		10M线速(MII=2.5M)		100M线速(MII=25M)
CRS,RX_CLK,COL,RX_DV,RXD2,RXD3,TXD3,RX_EX,TX_EN,TXD0,TXD2,TX_CLK,RXD0,RXD1,TXD1,TX-ER
GMII:8根线传数据 	10M线速(GMII=1.25M)		100M线速(GMII=12.5M)		1000M线速(GMII=125M)

//PA0-WKUP			ETH_MII_CRS									//PH2		ETH_MII_CRS			(PB0)				
//冲突检测
//PA3			ETH_MII_COL												//PH3		ETH_MII_COL			(PH3)
//接收数据
//PC4			ETH_MII_RXD0 / ETH_RMII_RXD0
//PC5			ETH_MII_RXD1 / ETH_RMII_RXD1
//PB0			ETH_MII_RXD2											//PH6		ETH_MII_RXD2		(PB0)
//PB1			ETH_MII_RXD3											//PH7		ETH_MII_RXD3		(PB1)	
//接收时钟
//PA1			ETH_MII_RX_CLK / ETH_RMII_REF_CLK
//接收数据出错指示
//PB10		ETH_MII_RX_ER											//PI10	ETH_MII_RX_ER		(PB10)
//接收数据有效指示
//PA7			ETH_MII_RX_DV / ETH_RMII_CRS_DV
//发送数据
//PB12		ETH_MII_TXD0 / ETH_RMII_TXD0			//PG13	ETH_MII_TXD0 / ETH_RMII_TXD0		(PB12)	
//PG14		ETH_MII_TXD1 / ETH_RMII_TXD1			//PB13	ETH_MII_TXD1 / ETH_RMII_TXD1		(PB14)	
//PC2			ETH_MII_TXD2
//PB8			ETH_MII_TXD3											//PE2		ETH_MII_TXD3		(PB8)
//发送器使能
//PB11		ETH_MII_TX_EN / ETH_RMII_TX_EN		//PG11	ETH_MII_TX_EN / ETH_RMII_TX_EN	(PB11)		
//发送器时钟
//PC3			ETH_MII_TX_CLK
//配置接口时钟
//PC1			ETH_MDC
//配置接口IO
//PA2			ETH_MDIO
	//PB5		ETH_PPS_OUT											没用
	//PG8		ETH_PPS_OUT											没用		
	
	
****************************************************************************/
void ETH_GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_MII);

  //SYSCFG_ETH_MediaInterfaceConfig(SYSCFG_ETH_MediaInterface_RMII);

  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 |  GPIO_Pin_7;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource0, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource7, GPIO_AF_ETH);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_8 | GPIO_Pin_10| GPIO_Pin_11 |  GPIO_Pin_12;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource0, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource8, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource10, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource11, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOB, GPIO_PinSource12, GPIO_AF_ETH);

  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource1, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource2, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource3, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource4, GPIO_AF_ETH);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource5, GPIO_AF_ETH);
                          
  GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_14;
  GPIO_Init(GPIOG, &GPIO_InitStructure);
  GPIO_PinAFConfig(GPIOG, GPIO_PinSource14, GPIO_AF_ETH);
}

/****************************************************************************
* 名	称：void ETH_NVIC_Config(void)
* 功	能：以太网中断初始化
* 入口参数：无
* 出口参数：无
* 说	明：EtherNet					 
****************************************************************************/
void ETH_NVIC_Config(void)
{
  NVIC_InitTypeDef   NVIC_InitStructure;
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); 
	
  NVIC_InitStructure.NVIC_IRQChannel = ETH_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);    
}

/****************************************************************************
* 名	称：void ETH_MACDMA_Config(u32* Flag)
* 功	能：以太网MAC媒体接入控制器DMA初始化
* 入口参数：u32* EthFlag	初始化成功标志	
* 出口参数：无
* 说	明：EtherNet					 
****************************************************************************/
void ETH_MACDMA_Config(u32* Flag)
{
  ETH_InitTypeDef ETH_InitStructure;

  RCC_AHB1PeriphClockCmd(	RCC_AHB1Periph_ETH_MAC | 
													RCC_AHB1Periph_ETH_MAC_Tx |
													RCC_AHB1Periph_ETH_MAC_Rx, ENABLE);                                             

  RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_ETH_MAC, ENABLE);
  RCC_AHB1PeriphResetCmd(RCC_AHB1Periph_ETH_MAC, DISABLE);					//对以太网AHB总线进行复位

  ETH->DMABMR |= ETH_DMABMR_SR;																			//重置所有MAC内部寄存器

  while((ETH->DMABMR & ETH_DMABMR_SR) != RESET)
      SysTimeDly(100);										//等待重置动作完成

  ETH_StructInit(&ETH_InitStructure);																//获取所有ETH_InitStructure配置参数

  /*------------------------   修改MAC参数   -----------------------------------*/
  ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Enable;
  //ETH_InitStructure.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable; 
  //ETH_InitStructure.ETH_Speed = ETH_Speed_10M;
  //ETH_InitStructure.ETH_Mode = ETH_Mode_FullDuplex;   

  ETH_InitStructure.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
  ETH_InitStructure.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
  ETH_InitStructure.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
  ETH_InitStructure.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
  ETH_InitStructure.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
  ETH_InitStructure.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
  ETH_InitStructure.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
  ETH_InitStructure.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
  ETH_InitStructure.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif

  /*------------------------   修改DMA参数   -----------------------------------*/  
  
  /* When we use the Checksum offload feature, we need to enable the Store and Forward mode: 
  the store and forward guarantee that a whole frame is stored in the FIFO, so the MAC can insert/verify the checksum, 
  if the checksum is OK the DMA can handle the frame otherwise the frame is dropped */
  ETH_InitStructure.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
  ETH_InitStructure.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
  ETH_InitStructure.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
 
  ETH_InitStructure.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
  ETH_InitStructure.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
  ETH_InitStructure.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;
  ETH_InitStructure.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
  ETH_InitStructure.ETH_FixedBurst = ETH_FixedBurst_Enable;                
  ETH_InitStructure.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
  ETH_InitStructure.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;
  ETH_InitStructure.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

  *Flag = ETH_Init(&ETH_InitStructure, DP83848_PHY_ADDRESS);		//万维网初始化

  ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);				//开启DMA接收中断(需要开启正常中断)
}

/****************************************************************************
* 名	称：u32 Eth_Link_PHYITConfig(u16 PHYAddress)
* 功	能：以太网物理层接口的芯片PHY设置
* 入口参数：uu16 PHYAddress		PHY地址
* 出口参数：无
* 说	明：EtherNet					 
****************************************************************************/
u32 Eth_Link_PHYITConfig(u16 PHYAddress)
{
  u32 tmpreg = 0;

  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MICR);						//读取MICR寄存器

  tmpreg |= (u32)PHY_MICR_INT_EN | PHY_MICR_INT_OE;
  if(ETH_WritePHYRegister(PHYAddress, PHY_MICR, tmpreg) == 0)		//开启INT引脚输出中断信号
    return ETH_ERROR;

  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MISR);						//读取MISR寄存器

  tmpreg |= (uint32_t)PHY_MISR_LINK_INT_EN;
  if(ETH_WritePHYRegister(PHYAddress, PHY_MISR, tmpreg) == 0)		//开启中断和连接状态
    return ETH_ERROR;
	
  return ETH_SUCCESS;   
}

/****************************************************************************
* 名	称：void Eth_Link_EXTIConfig(void)
* 功	能：配置以太网连接状态的外部EXTI通知
* 入口参数：无
* 出口参数：无
* 说	明：EtherNet					 
****************************************************************************/
void Eth_Link_EXTIConfig(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  EXTI_InitTypeDef EXTI_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;

  /* Enable the INT (PB14) Clock */
  RCC_AHB1PeriphClockCmd(ETH_LINK_GPIO_CLK, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

  /* Configure INT pin as input */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Pin = ETH_LINK_PIN;
  GPIO_Init(ETH_LINK_GPIO_PORT, &GPIO_InitStructure);

  /* Connect EXTI Line to INT Pin */
  SYSCFG_EXTILineConfig(ETH_LINK_EXTI_PORT_SOURCE, ETH_LINK_EXTI_PIN_SOURCE);

  /* Configure EXTI line */
  EXTI_InitStructure.EXTI_Line = ETH_LINK_EXTI_LINE;
  EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
  EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;  
  EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  EXTI_Init(&EXTI_InitStructure);

  /* Enable and set the EXTI interrupt to the highest priority */
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  
  NVIC_InitStructure.NVIC_IRQChannel = ETH_LINK_EXTI_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
}

/****************************************************************************
* 名	称：void EXTI0_IRQHandler(void)
* 功	能：网线拔插中断
* 入口参数：无
* 出口参数：无
* 说	明：网线拔出或插入均触发该中断			 
****************************************************************************/
void ETHPIN_IRQHandler(void)
{
	if(EXTI_GetITStatus(ETH_LINK_EXTI_LINE) != RESET)			//中断形成
  {
		if(((ETH_ReadPHYRegister(DP83848_PHY_ADDRESS, PHY_MISR)) & PHY_LINK_STATUS) != 0)	//检查是否已经发生或者链接中断
			EthLinkStatus = ~EthLinkStatus;										//插拔让状态标志翻转
		
    EXTI_ClearITPendingBit(ETH_LINK_EXTI_LINE);					//清除中断标志
  }
}

extern SYS_EVENT *s_xSemaphore;
void ETH_IRQHandler(void)
{
	if(ETH_GetDMAFlagStatus(ETH_DMA_FLAG_R) == SET) 		//接收到数据
    SysSemPost(s_xSemaphore);						//发送信号量

    ETH_DMAClearITPendingBit(ETH_DMA_IT_R);		//清除接收标志
    ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);	//清除正常中断标志
}

/****************************************************************************
* 名	称：u8 ETH_Config(void)
* 功	能：以太网初始化
* 入口参数：无
* 出口参数：初始化是否成功
* 说	明：EtherNet					 
****************************************************************************/
u8 ETH_Config(void)
{
	u32 EthInitFlag = 0;								//初始化成功标志	1成功	0失败
	
  ETH_GPIO_Config();									//以太网EtherNet引脚初始化
	ETH_NVIC_Config();									//以太网EtherNet中断初始化	

  while(EthInitFlag == ETH_ERROR)			//如果以太网初始化错误
    ETH_MACDMA_Config(&EthInitFlag);												//设置以太网EtherNet的MAC和DMA

  Eth_Link_PHYITConfig(DP83848_PHY_ADDRESS);	//配置PHY在改变链接状态时生成一个中断

  Eth_Link_EXTIConfig();											//配置以太网连接状态的外部EXTI通知
	
	//中断注册
	ETH_IRQ.EXTI_No = 0;
	ETH_IRQ.EXTI_Flag = 1;
	ETH_IRQ.IRQHandler = ETHPIN_IRQHandler;
	if(EXTIIRQHandlerInstall(&ETH_IRQ) != HVL_NO_ERR)					//中断注册
			return 0;		//失败
	return 1;
}
