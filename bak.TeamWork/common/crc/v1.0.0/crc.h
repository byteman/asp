#ifndef _CRC_INIT_H_	
#define _CRC_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif


//软件CRC32
/****************************************************************************
* 名	称：void CRC32S_Clear(void)
* 功	能：复位软件CRC32值	   
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void CRC32S_Clear(void);

/****************************************************************************
* 名	称：u32 CRC32S_Chack8(u8 flagC,u8* pBuf, u32 nSize)		  //取CRC32效验值
* 功	能：软件CRC32效验
* 入口参数：u8 flagC	是否清空CRC		0不复位	1复位
			u8* pBuf 	进入8位的数据
			u32 nSize	数据个数
* 出口参数：32位的	CRC32效验码
* 说	明：CRC polynomial 0xedb88320
****************************************************************************/
unsigned int CRC32S_Chack8(unsigned char flagC, unsigned char *pBuf, unsigned int nSize);

//软件CRC16
/****************************************************************************
* 名	称：u16 CRC16(u8 flag,u8 *pData, u32 nData)
* 功	能：CRC16效验
* 入口参数：u8 flag		是否复位CRC16值		0不复位	1复位
			u8 *pData	多数据计算校验值
			u32 nData	数据个数
* 出口参数：u16	CRC16校验值
* 说	明：每次自动复位校验值	
****************************************************************************/
unsigned short CRC16(unsigned char flag,unsigned char *pData, unsigned int nData);

unsigned short Reentrent_CRC16(unsigned char *pData, unsigned int nData);


/****************************************************************************
* 名	称：void CRC16_ResetDR(void)
* 功	能：CRC16效验值复位
* 入口参数：无
* 出口参数：无
* 说	明：清空CRC16值	
****************************************************************************/
void CRC16_ResetDR(void);

/****************************************************************************
* 名	称：void PushCrc16Byte(u8 nData)
* 功	能：CRC16效验
* 入口参数：u8 nData	单数据计算校验值
* 出口参数：无
* 说	明：一个数据计算校验值，不复位校验值
			一个数据计算完成后生成一个校验值，当把这个校验值在带入此函数中，正常结果会为0
****************************************************************************/
void PushCrc16Byte(unsigned char nData);

/****************************************************************************
* 名	称：u16 GetCrcCode(void)
* 功	能：获取CRC16校验值并验证，验证通过返回0，验证失败返回非0
* 入口参数：无
* 出口参数：u16 验证通过返回0，验证失败返回非0
* 说	明：无	
****************************************************************************/
unsigned short GetCrc16Code(void);



unsigned int modrtuChecksum ( unsigned char *chkbuf, unsigned char len );



#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

