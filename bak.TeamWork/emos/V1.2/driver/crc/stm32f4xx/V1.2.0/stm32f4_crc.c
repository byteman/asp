#include "stm32f4_crc.h"

//硬件CRC32
/****************************************************************************
* 名	称：void CRC32_init(void)		   //初始化CRC32
* 功	能：为CRC32分配时钟源	   
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void CRC32_init(void)		   //初始化CRC32
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_CRC, ENABLE);  //分配CRC32时钟
}

/****************************************************************************
* 名	称：u32 CRC32H_Chack8(u8 flagC,u8 *pBuf, u32 nSize)		  //取CRC32效验值
* 功	能：硬件CRC32效验	   
* 入口参数：u8 flagC	是否清空CRC		0不复位	1复位
			u8 *pBuf 	进入8位数据
			u32 nSize	数据个数
* 出口参数：32位的	CRC32效验码
* 说	明：进入的数据会累计计算CRC32效验码，CRC_ResetDR()为清空上次的效验码
****************************************************************************/
u32 CRC32H_Chack8(u8 flagC,u8 *pBuf, u32 nSize)		  //取CRC32效验值
{
	u32 index = 0;

	if(flagC == 1)
		CRC_ResetDR();		//清空CRC

	for(index = 0; index < nSize; index++)
		CRC->DR = (u32)pBuf[index];

	return (CRC->DR);
}

/****************************************************************************
* 名	称：u32 CRC32H_Chack32(u8 flagC,u32* pBuf, u32 nSize)		  //取CRC32效验值
* 功	能：硬件CRC32效验	   
* 入口参数：u8 flagC	是否清空CRC		0不复位	1复位
			u32* pBuf 	进入32位的数据
			u32 nSize	数据个数
* 出口参数：32位的	CRC32效验码
* 说	明：进入的数据会累计计算CRC32效验码，CRC_ResetDR()为清空上次的效验码
****************************************************************************/
u32 CRC32H_Chack32(u8 flagC,u32* pBuf, u32 nSize)
{
	u32 i = 0;
	
	if (flagC == 1)		
		CRC_ResetDR();		//复位CRC

	for(i = 0; i < nSize; i++)
		CRC->DR = *(pBuf + i);

	return (CRC->DR);
}

