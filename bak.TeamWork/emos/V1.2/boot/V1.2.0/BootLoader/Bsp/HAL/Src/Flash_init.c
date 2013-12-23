#include "Flash_init.h"
#include "String.h"			//memcpy

/****************************************************************************
* FLASH编程说明：
			-------------------------------------------------------------
			主存储器起始地址	0x08000000
			-------------------------------------------------------------
								   
FLASH_Unlock();															//解锁
FLASH_DataSpace_Erase(0x08000000,Size);			//擦除 只能按扇区擦除，扇区数和扇区大小见FindSector函数
FLASH_Lock();																//上锁
FLASH_ProgramWord(0x08030000,0x12345678);		//在指定的位置编程指定数据	按32位数据存入	必须先擦除再存入
****************************************************************************/

/****************************************************************************
* 名	称：u32 FindSector(const u32 Address)
* 功	能：根据地址查找地址所在扇区
* 入口参数：const u32 Address			地址
* 出口参数：u32 扇区号
* 说	明：1M ROM共分12个扇区
0扇区:		16K
1扇区:		16K
2扇区:		16K
3扇区:		16K
4扇区:		64K
5扇区:		128K
6扇区:		128K
7扇区:		128K
8扇区:		128K
9扇区:		128K
10扇区:		128K
11扇区:		128K
16 + 16 + 16 + 16 + 64 + 128 + 128 + 128 + 128 + 128 + 128 + 128 = 1024
****************************************************************************/
u32 FindSector(const u32 Address)
{
  u32 sector = 0;
  
  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
    sector = FLASH_Sector_0;  
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
    sector = FLASH_Sector_1;  
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
    sector = FLASH_Sector_2;  
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
    sector = FLASH_Sector_3;  
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
    sector = FLASH_Sector_4;  
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
    sector = FLASH_Sector_5;  
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
    sector = FLASH_Sector_6;  
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
    sector = FLASH_Sector_7;  
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
    sector = FLASH_Sector_8;  
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
    sector = FLASH_Sector_9;  
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
    sector = FLASH_Sector_10;  
  else
    sector = FLASH_Sector_11;  
	
	return sector;
}

/****************************************************************************
* 名	称：void FLASH_DataSpace_Erase(const u32 StartAddr,const u32 DataSize)
* 功	能：擦除数据空间
* 入口参数：const u32 StartAddr	开始地址
						const u32 DataSize	8位数据大小
* 出口参数：u8		擦除是否成功 1成功 0失败
* 说	明：无
****************************************************************************/
u8 FLASH_DataSpace_Erase(const u32 StartAddr,const u32 DataSize)
{
	u32 i = 0;				//累加器
  u32 UserStartSector = 0;			//开始扇区
  u32 UserEndSector = 0;				//结束扇区

  UserStartSector = FindSector(StartAddr);										//根据地址找开始扇区
  UserEndSector   = FindSector(StartAddr + DataSize - 1);			//根据地址找结束扇区

	FLASH_Unlock(); 	//擦除前解除FLASH保护
  for(i = UserStartSector; i <= UserEndSector; i += 8)		//指向开始扇区中的开始地址进行数据大小空间的擦除					
  {
    if (FLASH_EraseSector(i, VoltageRange_3) != FLASH_COMPLETE)
		{
			FLASH_Lock();	  				//上锁
      return 0;			//擦除失败
		}
  }
  FLASH_Lock();	  				//上锁
  return 1;					//擦除成功
}	  

/****************************************************************************
* 名	称：u8 Flash_DataWrite(const u32 StartAddr,const u32 DataSize,u8* Data_temp)
* 功	能：将数据写到指定的FLASH空间
* 入口参数onst 簎32 StartAddr	开始地址
			const u32 DataSize	数据大小
			u8* Data_temp	数据
* 出口参数：u8	0存储失败
				1存储成功
* 说	明：按32位数据长度进行存储	 必须要先擦除才能改写
****************************************************************************/
u8 Flash_DataWrite(const u32 StartAddr,const u32 DataSize,u8* Data_temp)
{
	u32 i = 0;										//累加器
	u32	DataSize32 = 0;						//32位的数据长度
	u32 err = 0;									//错误状态
  u8 ucTemp[4] = {0};
    
	FLASH_Status status = FLASH_COMPLETE;	  	//定义一个FLASH标志

	DataSize32 = DataSize / 4;					//8位的数据长度转成32位的数据长度
	
	while(1)
	{
		FLASH_Unlock();	  			//解锁
		status = FLASH_ProgramWord(StartAddr + i * 4, *(((u32*)Data_temp) + i));   //调用写操作
		FLASH_Lock();	  				//上锁
		if(status == FLASH_COMPLETE) 	//FLASH不忙
		{
      err = 0;
			if((*((u32*)(StartAddr + i * 4))) != *(((u32*)Data_temp) + i))	 //数据效验
				return 0;					 //数据效验错误

			i++;							  //下一次写入操作
			if(i >= DataSize32)				  //如果下一次操作超出了所需要的范围
				break;						  //退出写入操作
		}
		else 
		{
			if (++err > 50)
				return 0;
		}
	}	
  i = DataSize32 * 4;

	while(i < DataSize) 
	{
		memcpy(ucTemp,Data_temp + i,(DataSize - i));
		FLASH_Unlock();	  			//解锁
		status = FLASH_ProgramWord(StartAddr + i,*((u32*)ucTemp));   //调用写操作
		FLASH_Lock();	  			//上锁
		if(status == FLASH_COMPLETE) 	//FLASH不忙
		{
			err = 0;
			if((*((u32*)(StartAddr + i))) != *((u32*)ucTemp))	 //数据效验
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

/*
	u8 DATA[128]={0};			//需要存放的数据
	u8 i=0;

	for(i=0;i<128;i++)			//数据赋值
		DATA[i]=i;

	//方式一：
	FLASH_DataSpace_Erase(0x08030000,128);	//擦除	//从FLASH的 0x08030000 地址开始擦除128个8位数据位置		128*8=1024
	Flash_DataWrite(0x08030000,128,DATA);	//写入	//从FLASH的 0x08030000 地址开始存放128个8位DATA数据
	FLASH_DataShow(0x08030000,128);			//读出	//从FLASH的 0x08030000 地址开始读取并显示32个32位数据	32*32=1024	4个8位=32位

	//方式二：
	Flash_SAVEData(0x08030000,DATA,128);	//擦写读
*/
