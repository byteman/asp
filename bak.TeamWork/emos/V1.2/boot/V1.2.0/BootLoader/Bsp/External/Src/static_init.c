#include "static_init.h"

/****************************************************************************
* 名	称：void Delay(const u32 nCount)
* 功	能：一个工作的频率延时
* 入口参数：const u32 nCount	延时量
* 出口参数：无
* 说	明：无
****************************************************************************/
void Delay(const u32 nCount)
{
	u32 i = nCount;
  while(i--);
}

/****************************************************************************
* 名	称：void Delay_MS(const u32 nCount_temp)
* 功	能：毫秒级
* 入口参数：const u32 nCount_temp	延时量
* 出口参数：无
* 说	明：无
****************************************************************************/
void Delay_MS(const u32 nCount_temp)
{
	u32 nCount = nCount_temp * 8000;
	while(nCount--);
}

/****************************************************************************
* 名	称：void Delay_S(const u32 nCount_temp)
* 功	能：秒级
* 入口参数：const u32 nCount_temp	延时量
* 出口参数：无
* 说	明：无
****************************************************************************/
void Delay_S(const u32 nCount_temp)
{
	u32 nCount = nCount_temp * 8000000;
	while(nCount--);
}

/****************************************************************************
* 名	称：void GetCpuIdHex(void)
* 功	能：获取CPU芯片ID值16进制   
* 入口参数：无
* 出口参数：无
* 说	明：6个u16的值存放CPUID
			u16 x[6] = {0};
			u8 x[12] = {0};
****************************************************************************/
void GetCpuIdHex(u8* HCpuId)
{
	u32 CpuId[3] = {0};

	CpuId[0] = (*(u32*)(0x1FFF7A10));
	CpuId[1] = (*(u32*)(0x1FFF7A14));
	CpuId[2] = (*(u32*)(0x1FFF7A18));
	
	HCpuId[0] = (u8)(CpuId[0] >> 24);
	HCpuId[1] = (u8)(CpuId[0] >> 16); 
	HCpuId[2] = (u8)(CpuId[0] >> 8);
	HCpuId[3] = (u8)(CpuId[0]);

	HCpuId[4] = (u8)(CpuId[1] >> 24);
	HCpuId[5] = (u8)(CpuId[1] >> 16); 
	HCpuId[6] = (u8)(CpuId[1] >> 8);
	HCpuId[7] = (u8)(CpuId[1]);

	HCpuId[8] = (u8)(CpuId[2] >> 24);
	HCpuId[9] = (u8)(CpuId[2] >> 16); 
	HCpuId[10] = (u8)(CpuId[2] >> 8);
	HCpuId[11] = (u8)(CpuId[2]);
}

/****************************************************************************
* 名	称：u32 BeiNumber(const u32 num_temp,const u8 len_temp)
* 功	能：10倍数
* 入口参数：const u32 num_temp	需要翻倍的数
			const u8 len_temp	翻多少倍 即1后面多少个0
* 出口参数：u32 翻倍后的倍数值
* 测试代码：
	u32 x = 2;
	x = BeiNumber(x,3);
	//x = 2000;
****************************************************************************/
u32 BeiNumber(const u32 num_temp,const u8 len_temp)
{
	u8 i = len_temp;
	u32 Bei = 1;		 					 //默认倍数值

	while(i--)								//按倍数进行翻倍
		Bei *= 10;	  					//翻倍增加
	
	return Bei * num_temp;	  //返回倍数
}

/****************************************************************************
* 名	称：u32 ASCIItoNumber(u8* data_temp,const u8 len_temp)
* 功	能：将ASCII码数字转换成为u32型数字
* 入口参数：u8* data_temp	ASCII码数字
			const u8 len_temp		ASCII码数字个数
* 出口参数：u32 unsigned int型数字
* 测试代码：
	u32 num=0;
	num=ASCIItoNumber("1234",4);
	//num=1234;
****************************************************************************/
u32 ASCIItoNumber(u8* data_temp,const u8 len_temp)
{
    u8 i = len_temp;													//长度
    u32 val = 0; 
    u32 len = 0;                      				//最终长度 
    
    while(1)
    {
			val = data_temp[i - 1] - 0x30;     			//取个数 
			val = BeiNumber(val,len_temp - i);     	//得到位上数据
			len += val;                    					//位上数据组合 
			
			i--;
			if(i == 0)
				 return len;
    } 
}

/****************************************************************************
* 名	称：u32 Get_SyS_CLK(const u8 temp)
* 功	能：当前系统各个频率状态获取	   
* 入口参数：const u8 temp 选择频率参数
* 出口参数：时钟频率
* 说	明：HCLK频率(1) PCLK1频率(2) PCLK2频率(3) SYSTEM频率(0其它)
****************************************************************************/
u32 Get_SyS_CLK(const u8 temp)
{
	RCC_ClocksTypeDef RCC_ClockFreq;
	RCC_GetClocksFreq(&RCC_ClockFreq);	//为时钟变量赋予片上各时钟
	switch (temp)
	{
		case 1:	return RCC_ClockFreq.HCLK_Frequency; 		//HCLK频率			168MHZ
		case 2:	return RCC_ClockFreq.PCLK1_Frequency; 	//PCLK1频率			42MHZ
		case 3:	return RCC_ClockFreq.PCLK2_Frequency; 	//PCLK2频率			84MHZ
		default: return RCC_ClockFreq.SYSCLK_Frequency;	//SYSTEM频率		168MHZ
	}	
}
