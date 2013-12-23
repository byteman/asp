#include "f24cl64.h"
#include "C2.h"						//互斥量
#include "driver.h"				//注册
#include "stm32f4_crc.h"			//crc16
#include "String.h"				//memset

#define EEPROM_ADDR		0xA0

static SYS_EVENT *EEPROM_Mutex = 0;		 			//定义存取互斥量

static u8 FM24_pDisc[] = "LuoHuaiXiang_FM24CL64\r\n";		//设备描述字符

/****************************************************************************
* 名	称：u8 FM24CL64_Write(u32 Addr,u8* pData,u32 len)
* 功	能：单芯片连续页写操作
* 入口参数
						u32 Addr						芯片存放地址				0-65534		128/页 * 512页
						u8* pData						需要存储的数据
						u32 len							存储的数据长度
* 出口参数：s32 是否存储成功	-1:失败  正数或0：实际写入数据长度
* 说	明：无
****************************************************************************/
s32 FM24CL64_Write(u32 Addr,u8* pData,u32 len)
{
	u8 err;						//获取错误代码
	s32 reallen = 0;
	
	if(EEPROM_Mutex == 0) 
		return -1;
	SysMutexPend(EEPROM_Mutex,0,&err);
	if(err != SYS_ERR_NONE)
		return -1;
	
	reallen = I2CSendWordADDRData(EEPROM_ADDR,Addr,pData,len);		//接收数据
// 	if(*Reallen != len)
// 		return 0;

	if(EEPROM_Mutex == 0) 
		return -1;
	SysMutexPost(EEPROM_Mutex);

	return reallen;
}

/****************************************************************************
* 名	称：u32 FM24CL64_Erase(u32 Addr,u8 pData,u32 len)
* 功	能：单芯片连续页写操作
* 入口参数：
						u32 Addr						芯片存放地址				0-65534		128/页 * 512页
						u8* pData						需要存储的数据
						u32 len							存储的数据长度
* 出口参数：s32									实际擦除的数据长度 返回-1失败
* 说	明：无
****************************************************************************/
s32 FM24CL64_Erase(u32 Addr,u8 pData,u32 len)
{
	u8 err;						//获取错误代码
	u32 i = 0;				//累加器
	u32 realcnt = 0;	//实际写入的数据个数
	
	if(EEPROM_Mutex == 0) 
		return -1;
	SysMutexPend(EEPROM_Mutex,0,&err);
	if(err != SYS_ERR_NONE)
		return -1;
	
	for(i = 0; i < len ; i++)  //一个数据写
	{
// 		Reallen = I2CSendWordADDRData(EEPROM_ADDR,Addr + i,&pData,1);		//接收数据
		if(I2CSendWordADDRData(EEPROM_ADDR,Addr + i,&pData,1) != 1)
			return -1;
// 		Reallen = 0;
		realcnt++;
	}
	
	if(EEPROM_Mutex == 0) 
		return -1;
	SysMutexPost(EEPROM_Mutex);
	
	return realcnt;
}

/****************************************************************************
* 名	称：u8 FM24CL64_Read(EEPROM_Cfg* Ex,u32 Addr,u8* pData,u32 len,u32* Reallen)
* 功	能：单芯片连续页读操作
* 入口参数：EEPROM_Cfg* Ex			EEPROM设备配置
						u32 Addr						芯片存放地址				0-65534		128/页 * 512页
						u8* pData						需要读取的数据
						u32 len							读取的数据长度
* 出口参数：s32 是否读取成功	成功返回实际读取的数据长度  失败返回-1
* 说	明：无
****************************************************************************/
s32 FM24CL64_Read(u32 Addr,u8* pData,u32 len)
{
	u8 err = 0;					//错误状态
	s32 reallen = 0;
	
	if(EEPROM_Mutex == 0)									//互斥量未创建
		return -1;
	SysMutexPend(EEPROM_Mutex, 0, &err);	//获取互斥量
	if(err != SYS_ERR_NONE)
		return -1;
	
	reallen = I2CReceiveWordADDRData(EEPROM_ADDR,Addr,pData,len);		//接收数据
// 	if(*Reallen != len)										//如果接收的数据个数不等于希望接收的数据个数
// 		return -1;
	
	if(EEPROM_Mutex == 0)									//互斥量未创建
		return -1;
	SysMutexPost(EEPROM_Mutex);						//释放互斥量
	
// 	*Reallen = len;
	return reallen;
}

/****************************************************************************
* 名	称：static s8 FM24CL64_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 FM24CL64_Open(u32 lParam)
{
	return 0;					
}

/****************************************************************************
* 名	称：static s8 FM24CL64_Close(void)
* 功	能：统一接口
* 入口参数：无
* 出口参数：s8						关闭操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 FM24CL64_Close(void)
{
	return 0;		
}

/****************************************************************************
* 名	称：static static s32 FM24CL64Read(u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备读
* 入口参数：u32 offset		偏移地址
						u8* Buffer		读取数据存放位置
						u32 len				希望读取的数据长度
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 FM24CL64Read(u32 offset,u8* Buffer,u32 len)
{
	return FM24CL64_Read(offset,Buffer,len);
}

/****************************************************************************
* 名	称：static s32 FM24CL64Write(u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备写
* 入口参数：u32 offset		偏移地址
						u8* Buffer		写入数据源存放位置
						u32 len				希望写入的数据长度
* 出口参数：s32						-1:失败  正数或0：实际写入数据长度
* 说	明：参数长度+2(crc)长度
****************************************************************************/
static s32 FM24CL64Write(u32 offset,u8* Buffer,u32 len)
{	
	return FM24CL64_Write(offset,Buffer,len);							//存储成功
}

/****************************************************************************
* 名	称：static s32 FM24CL64_Ioctl(u32 cmd,u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 FM24CL64_Ioctl(u32 cmd,u32 lParam)
{
	switch(cmd)
	{
		case FM24CL64_CMDERASE:				//芯片擦除
		{
			return FM24CL64_Erase(((EEPROM_EraseCfg*)lParam)->StartAddr,
																((EEPROM_EraseCfg*)lParam)->DefaultVal,
																((EEPROM_EraseCfg*)lParam)->Len);
		}
	}
	return 0;	//错误命令	
}

/****************************************************************************
* 名	称：u8 FM24CL64_Init(u8 prio)
* 功	能：铁电初始化
* 入口参数：u8 prio								优先级
* 出口参数：u8 										是否成功		1成功	0失败
* 说	明：无
****************************************************************************/
u8 FM24CL64_Init(u8 prio)
{
	u8 err = 0;

	//注册用变量
	DEV_REG fm24 = 										//设备注册信息表						不必static		
	{
		BLOCK_FRAMEEPROM,								//设备ID号
		0,  														//是否共享			0:不可共享使用, 1:可共享使用
		0, 															//对于共享设备最大打开次数
		1,															//最大读次数
		1,															//最大写次数
		FM24_pDisc,											//驱动描述			自定义
		20120001,												//驱动版本			自定义
		(u32*)FM24CL64_Open,						//设备打开函数指针
		(u32*)FM24CL64_Close, 					//设备关闭函数指针
		(u32*)FM24CL64Read,							//块读函数
		(u32*)FM24CL64Write,						//块写函数
		(u32*)FM24CL64_Ioctl						//控制函数
	};
	
	if(I2Cx_Init() == 0)		//EEPROM初始化
		return 0;									//失败
	
	if(EEPROM_Mutex == 0)
		EEPROM_Mutex = SysMutexCreate(prio, &err);							//创建互斥量  //提升到的优先级，该优先级不能被占用
	if(err != 0)																							//互斥量创建失败
		return 0;
	
	if(DeviceInstall(&fm24) != HVL_NO_ERR)										//设备注册
		return 0;
	
	CRC32_init();
	return 1;										//成功
}
