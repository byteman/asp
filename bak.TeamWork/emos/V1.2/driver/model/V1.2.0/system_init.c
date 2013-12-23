#include "system_init.h"	//AppRunAddr
#include "driver.h"				//DevList_init
#include <String.h>

static u8 EMOS_Versions[] = "EMOS-V001-001-001-001";				//平台版本号  //21个字符

static u32 AppRunAddr = 0x08000000;						//程序跳转地址				后期通过接口设置

//固件备份地址区
#define AppBakSpaceSize				262144 - 1				//固件备份区空间大小	256K	大小转换成空间需要-1 空间从0开始存放
#define StartAddr 						0x8060000				//应用程序备份开始地址(用于数据存放)
#define EndAddr 						0x809FFFF				//应用程序备份结束地址(用于效验、长度、来源存放)
#define AppCRCAddress					EndAddr - 3				//CRC效验码地址	0x809FFFC - 0x809FFFF		4Byte
#define AppSizeAddress					EndAddr - 7				//大小存放地址	0x809FFF8 - 0x809FFFB		4Byte
#define AppUpSrcAddress				EndAddr - 11			//固件来源  	0x809FFF4 - 0x809FFF7		4Byte	


#define DelUPFlagSector FLASH_Sector_2							//删除标志扇区
#define DelAPPbakSector1 FLASH_Sector_7						//删除应用程序备份扇区
#define DelAPPbakSector2 FLASH_Sector_8						//删除应用程序备份扇区

#define UPBOOTFlag 						0x1004				//升级标志		有此标志则有新固件包 没有则不需要升级

#define AppUPAddress					0x8008000							//固件更新标志区 					0x8008000 - 0x80BFFFF		64K

/****************************************************************************
* 名	称：u8 Get_EMOS_Versions(u8* Versions)
* 功	能：获取平台版本号
* 入口参数：Versions		版本号地址
* 出口参数：u8					版本号字符长度
* 说	明：版本号21个字符
****************************************************************************/
u8 Get_EMOS_Versions(u8* Versions)
{	 
	memcpy(Versions,EMOS_Versions,sizeof(EMOS_Versions) - 1);
	return sizeof(EMOS_Versions) - 1;
}

/****************************************************************************
* 名	称：INT32U SYS_CPU_SysTickClkFreq(void)
* 功	能：获取系统主频时钟频率
* 入口参数：无
* 出口参数：主频频率
* 说	明：Systick初始化中会用到
****************************************************************************/
INT32U SYS_CPU_SysTickClkFreq(void)
{	 
	RCC_ClocksTypeDef rcc_clocks;								//片上各类时钟频率
	RCC_GetClocksFreq(&rcc_clocks);							//获取片上时钟频率
	return ((INT32U)rcc_clocks.HCLK_Frequency);	//返回片上主频频率
}

/****************************************************************************
* 名	称：uint32_t Get_CPUSysTime(void)
* 功	能：获取系统各时钟频率
* 入口参数：无
* 出口参数：uint32_t	各时钟频率
* 说	明：
0 = SYSCLK clock frequency expressed in Hz
1 = HCLK clock frequency expressed in Hz
2 = PCLK1 clock frequency expressed in Hz
3 = PCLK2 clock frequency expressed in Hz
其它返回0
****************************************************************************/
uint32_t Get_CPUSysTime(u8 Flag)
{
	RCC_ClocksTypeDef rcc_clocks;								//片上各类时钟频率
	RCC_GetClocksFreq(&rcc_clocks);							//获取片上时钟频率
	
	switch(Flag)
	{
		case 0:
			return rcc_clocks.SYSCLK_Frequency;			/*!<  SYSCLK clock frequency expressed in Hz */
		case 1:
			return rcc_clocks.HCLK_Frequency;   		/*!<  HCLK clock frequency expressed in Hz */
		case 2:
			return rcc_clocks.PCLK1_Frequency;  		/*!<  PCLK1 clock frequency expressed in Hz */
		case 3:
			return rcc_clocks.PCLK2_Frequency;  		/*!<  PCLK2 clock frequency expressed in Hz */
	}
	return 0;
}

/****************************************************************************
* 名	称：void SetAppRunAddr(u32 addr)
* 功	能：设置跳转地址
* 入口参数：u32 addr				跳转地址
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetAppRunAddr(u32 addr)
{
	AppRunAddr = addr;
}

/****************************************************************************
* 名	称：u32 LookAppRunAddr(void)
* 功	能：获取跳转地址
* 入口参数：无
* 出口参数：u32 						跳转地址
* 说	明：无
****************************************************************************/
u32 LookAppRunAddr(void)
{
	return AppRunAddr;
}

/****************************************************************************
* 名	称：void Nvic_init(void)
* 功	能：中断管理初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Nvic_init(void)
{
	#ifdef  VECT_TAB_RAM	//中断向量表存放地址到SRAM 0x20000000  
	  NVIC_SetVectorTable(NVIC_VectTab_RAM, 0x0); 
	#else					//中断向量表存放地址到FLASH	0x08000000
		NVIC_SetVectorTable(NVIC_VectTab_FLASH, AppRunAddr);   
	#endif
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
* 名	称：void System_Init(void)
* 功	能：系统初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void System_Init(void)
{
	Nvic_init();							//中断管理初始化
	SYS_CPU_SysTickInit();		//初始化系统滴答时间	(10毫秒级)
	SysInit();								//系统初始化
	DevList_init();						//设备管理单元初始化
}

/****************************************************************************
* 名	称：u32 BeiNumber(u32 num_temp,u8 len_temp)
* 功	能：10倍数
* 入口参数：u32 num_temp	需要翻倍的数
						u8 len_temp		翻多少倍 即1后面多少个0
* 出口参数：u32 					翻倍后的倍数值
* 测试代码：
	u32 x = 2;
	x = BeiNumber(x,3);
	//x = 2000;
****************************************************************************/
u32 BeiNumber(u32 num_temp,u8 len_temp)
{
	u32 Bei = 1;		  			//默认倍数值

	while(len_temp--)				//按倍数进行翻倍
		Bei *= 10;	  				//翻倍增加
	return Bei * num_temp;	//返回倍数
}

/****************************************************************************
* 名	称：u32 ASCIItoNumber(u8* data_temp,u8 len_temp)
* 功	能：将ASCII码数字转换成为u32型数字
* 入口参数：u8* data_temp	ASCII码数字
						u8 len_temp		ASCII码数字个数
* 出口参数：u32 unsigned 	int型数字
* 测试代码：
u32 num = 0;
num = ASCIItoNumber("1234",4);
//num = 1234;
****************************************************************************/
u32 ASCIItoNumber(u8* data_temp,u8 len_temp)
{
	u8 i = len_temp;											//长度
	u32 val = 0; 
	u32 len = 0;                      		//最终长度 
	
	while(1)
	{
		val = data_temp[i - 1] - 0x30;     	//取个数 
		val = BeiNumber(val,len_temp - i);  //得到位上数据
		len += val;                    			//位上数据组合 
		
		i--;
		if(i == 0)
			return len;
	} 
}

/****************************************************************************
* 名	称：u32 BCDToBIN(u32 BCD)
* 功	能：将十进制数据转二进制码10进制数
* 入口参数：u32 十进制数据
* 出口参数：u32 二进制数据
* 测试代码：
u32 BcdVal = 101;
u32 BinVal = 0;
BinVal = BCDToBIN(BcdVal);
//BinVal = 65;
****************************************************************************/
u32 BCDToBIN(u32 BCD)
{
	return (BCD & 0x0f) + (BCD >> 4)	* 10;
}

/****************************************************************************
* 名	称：u32 BINToBCD(u32 BIN)
* 功	能：将二进制码10进制数转十进制数据
* 入口参数：u32 二进制数据
* 出口参数：u32 十进制数据
* 测试代码：
u32 BinVal = 65;
u32 BcdVal = 0;
BcdVal = BINToBCD(BinVal);
//BcdVal = 101;
****************************************************************************/
u32 BINToBCD(u32 BIN)
{
	return ((BIN / 10) << 4) + BIN % 10;
}

/****************************************************************************
* 名	称：void FLASH_Erase_Delay(u32 cnt)
* 功	能：延时
* 入口参数：无
* 出口参数：u32		延时时间
* 说	明：一定要检查是否擦除成功，不成功则再来一次。
****************************************************************************/
void FLASH_Erase_Delay(u32 cnt)
{
	while(cnt--);
}

/****************************************************************************
* 名	称：u8 FLASH_APPBakOne_Erase(void)
* 功	能：擦除应用数据空间1
* 入口参数：无
* 出口参数：u8		擦除是否成功 1成功 0失败
* 说	明：一定要检查是否擦除成功，不成功则再来一次。
****************************************************************************/
u8 FLASH_APPBakOne_Erase(void)
{
	FLASH_Unlock(); 	//擦除前解除FLASH保护
	//2013-05-10 增加擦除flash标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
						FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
						FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
 	if(FLASH_EraseSector(DelAPPbakSector1, VoltageRange_3) != FLASH_COMPLETE)
 	{
 		FLASH_Lock();	  //上锁
 		return 0;				//擦除失败
 	}
  FLASH_Lock();	  	//上锁
  return 1;					//擦除成功
}	

/****************************************************************************
* 名	称：u8 FLASH_APPBakTwo_Erase(void)
* 功	能：擦除应用数据空间2
* 入口参数：无
* 出口参数：u8		擦除是否成功 1成功 0失败
* 说	明：一定要检查是否擦除成功，不成功则再来一次。
****************************************************************************/
u8 FLASH_APPBakTwo_Erase(void)
{
	FLASH_Unlock(); 	//擦除前解除FLASH保护
	//2013-05-10 增加擦除flash标志位
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR |
						FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR |
						FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
	if(FLASH_EraseSector(DelAPPbakSector2, VoltageRange_3) != FLASH_COMPLETE)
	{
		FLASH_Lock();	  //上锁
		return 0;				//擦除失败
	}
  FLASH_Lock();	  	//上锁
  return 1;					//擦除成功
}	

/****************************************************************************
* 名	称：u8 FLASH_UPFlag_Erase(void)
* 功	能：擦除更新标志空间
* 入口参数：无
* 出口参数：u8		擦除是否成功 1成功 0失败
* 说	明：一定要检查是否擦除成功，不成功则再来一次。
****************************************************************************/
u8 FLASH_UPFlag_Erase(void)
{
	FLASH_Unlock(); 	//擦除前解除FLASH保护
 	if(FLASH_EraseSector(DelUPFlagSector, VoltageRange_3) != FLASH_COMPLETE)
 	{
 		FLASH_Lock();	  //上锁
 		return 0;				//擦除失败
 	}
  FLASH_Lock();	  	//上锁
  return 1;					//擦除成功
}	

/****************************************************************************
* 名	称：u8 Flash_APPBak_Store(u32 Addri,u8* Data,u32 DataSize)
* 功	能：将数据写到指定的FLASH空间
* 入口参数：u32 Addri			偏移地址	4的倍数(Flash4字节操作)
						u8* Data数据
						u32 DataSize	数据大小
* 出口参数：u8	0存储失败
				1存储成功
* 说	明：按32位数据长度进行存储	 必须要先擦除才能改写
****************************************************************************/
u8 Flash_APPBak_Store(u32 Addri,u8* Data,u32 DataSize)
{
	u32 i = 0;										//累加器
	u32	DataSize32 = 0;						//32位的数据长度
	u32 err = 0;									//错误状态
  u8 ucTemp[4] = {0};
  u32 startOffsetAddr = 0;			//加偏移量的开始地址
  
	FLASH_Status status = FLASH_COMPLETE;	  	//定义一个FLASH标志

	if(Addri % 4 != 0)												//FLASH必须4字节操作
		return 0;
	
	startOffsetAddr = StartAddr + Addri;			
	if(startOffsetAddr > EndAddr)							//超过最大范围
		return 0;
	
	DataSize32 = DataSize / 4;								//8位的数据长度转成32位的数据长度
	
	while(1)
	{
		FLASH_Unlock();	  						//解锁
		status = FLASH_ProgramWord(startOffsetAddr + i * 4, *(((u32*)Data) + i));   //调用写操作
		FLASH_Lock();	  							//上锁
		if(status == FLASH_COMPLETE) 	//FLASH不忙
		{
			err = 0;
			if((*((u32*)(startOffsetAddr + i * 4))) != *(((u32*)Data) + i))	 //数据效验
				return 0;								 	//数据效验错误

			i++;							  				//下一次写入操作
			if(i >= DataSize32)				  //如果下一次操作超出了所需要的范围
				break;						  			//退出写入操作
		}
		else 
		{
			if(++err > 50)
				return 0;
		}
	}	
  i = DataSize32 * 4;

	while(i < DataSize) 
	{
		memcpy(ucTemp,Data + i,(DataSize - i));
		FLASH_Unlock();	  			//解锁
		status = FLASH_ProgramWord(startOffsetAddr + i,*((u32*)ucTemp));   //调用写操作
		FLASH_Lock();	  			//上锁
		if(status == FLASH_COMPLETE) 	//FLASH不忙
		{
			err = 0;
			if((*((u32*)(startOffsetAddr + i))) != *((u32*)ucTemp))	 //数据效验
				return 0;					 //数据效验错误
			break;							  //下一次写入操作
		}
		else
		{
			if (++err > 50)
				return 0;
		}
	}
	return 1;				
}

/****************************************************************************
* 名	称：u8 Flash_APPInfo_Store(u32 FILECRC,u32 FILESize)
* 功	能：将数据写到指定的FLASH空间
* 入口参数：u8* Data数据
						u32 DataSize	数据大小
* 出口参数：u8	0存储失败
				1存储成功
* 说	明：按32位数据长度进行存储	 必须要先擦除才能改写
****************************************************************************/
u8 Flash_APPInfo_Store(u32 FILECRC,u32 FILESize, u32 flag)
{
	u32 FileUpSrc = flag; //NETBOOT;	
	
	FLASH_Unlock();	  			//解锁
	if(FLASH_ProgramWord(AppCRCAddress,FILECRC) != FLASH_COMPLETE)							//更新标志
	{
		FLASH_Lock();	  			//上锁
		return 0;
	}
	if(FLASH_ProgramWord(AppSizeAddress,FILESize) != FLASH_COMPLETE)						//更新标志
	{
		FLASH_Lock();	  			//上锁
		return 0;
	}
	if(FLASH_ProgramWord(AppUpSrcAddress,FileUpSrc) != FLASH_COMPLETE)					//更新标志
	{
		FLASH_Lock();	  			//上锁
		return 0;
	}
	FLASH_Lock();	  			//上锁
	return 1;
}

/****************************************************************************
* 名	称：u8 Flash_UPFlag(void)
* 功	能：修改更新标志
* 入口参数：无
* 出口参数：u8	0存储失败
								1存储成功
* 说	明：无
****************************************************************************/
u8 Flash_UPFlag(void)
{
	u8 err = 0;
	u32 newbootflag = UPBOOTFlag;			//更新标志
	
	while(FLASH_UPFlag_Erase() == 0)	//擦除
	{
		err++;
		
		if(err > 5)
			return 0;
		
		FLASH_Erase_Delay(1000);				//延时后进行下一次擦除
	}
	
	FLASH_Unlock();	  			//解锁
	if(FLASH_ProgramWord(AppUPAddress,newbootflag) != FLASH_COMPLETE)					//更新标志
	{
		FLASH_Lock();	  			//上锁
		return 0;
	}
	FLASH_Lock();	  			//上锁
	return 1;
}
