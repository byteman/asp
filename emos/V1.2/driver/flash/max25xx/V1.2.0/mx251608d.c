#include "mx251608d.h"
#include "SPIx.h"
#include "C2.h"
#include "IO.h"

/*
MX25Lxx08D
一页		256		Byte
一扇区	4096	Byte		256*16页
一块		65536	Byte		4096*16扇区		256*256页
1608D = 2M = 2097152 Byte
*/

#define PageSize 			256													//页大小
#define PageNo				8192												//页数
#define SectorSize		4096												//扇区大小
#define SectorNo			512													//扇区数量
#define BlockSize			65536												//块大小
#define BlockNo				32													//块数量
#define CapacitySize 	2097152											//总容量

#define FLASH_FCE_PIN		GPIO_Pin_9
#define FLASH_FCE_PORT	GPIOC

typedef struct
{
	uint16_t 				FCE_Pin;					//片选引脚
	GPIO_TypeDef* 	FCE_Port;					//片选引脚端口号
	SPI_Cfg 				FSPI;							//设备所用SPI接口
}MX251608D_Cfg;		//DATAFLASH结构体

static SYS_EVENT *MX251608D_Mutex = 0;		 			//定义存取互斥量

#define MX251608DA				1					//MX251608DA					1启用		0关闭

#if(MX251608DA == 1)
static u8 MX25A_pDisc[] = "LuoHuaiXiang_MX25A\r\n";		//设备描述字符
// static MX251608D_Cfg MX25A =
// {
// 	GPIO_Pin_9,					//FCE_PIN
// 	GPIOC,							//FCE_PORT
// 	{	
// 		0,								//IO
// 		GPIO_Pin_10,			//SCK_PIN
// 		GPIOC,						//SCK_PORT
// 		GPIO_Pin_11,			//MISO_PIN
// 		GPIOC,						//MISO_PORT
// 		GPIO_Pin_12,			//MOSI_PIN
// 		GPIOC,						//MOSI_PORT
// 		SPI_BaudRatePrescaler_32		//baudrate
// 	}
// };
#endif

/****************************************************************************
* 名	称：void MX251608D_CEH(MX251608D_Cfg* Fx)
* 功	能：设备片选拉高
* 入口参数：MX251608D_Cfg* Fx		设备
* 出口参数：无
* 说	明：无
****************************************************************************/
void MX251608D_CEH()	
{
	IoHi(FLASH_FCE);
	
}

/****************************************************************************
* 名	称：void MX251608D_CEL(MX251608D_Cfg* Fx)
* 功	能：设备片选拉低
* 入口参数：MX251608D_Cfg* Fx		设备
* 出口参数：无
* 说	明：无
****************************************************************************/
void MX251608D_CEL()	
{
	IoLo(FLASH_FCE);
}

/****************************************************************************
* 名	称：void GetMX251608Dx_ID(u8* ID,MX251608D_Cfg* Fx)
* 功	能：获取设备ID
* 入口参数：u8* ID							设备ID
						MX251608D_Cfg* Fx		设备
* 出口参数：无
* 说	明：u8 ID[3] = {0};
****************************************************************************/
void GetMX251608Dx_ID(u8* ID)
{
	if(ID == (u8*)0) 														//长度保护
		return;			
	
	MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x9F);
	ID[0] = SPIbyteSendGet(0);
	ID[1] = SPIbyteSendGet(0);
	ID[2] = SPIbyteSendGet(0);
	
	MX251608D_CEH();													//关闭设备
}

/****************************************************************************
* 名	称：u8 MX251608D_Read(MX251608D_Cfg* Fx,u32 ReadAddr,u8* pBuffer,u32 Len,u32* RealLen)
* 功	能：Flash的线性读操作
* 入口参数：
						u32 ReadAddr					开始读取位置
						u8* pBuffer						读取数据存放地址
						u32 Len								希望读取的数据个数
* 出口参数：s32 										数据读取是否成功		
* 说	明：无
****************************************************************************/
s32 MX251608D_Read(u32 ReadAddr,u8* pBuffer,u32 Len)
{
	u8 err = 0;																	//错误状态
  u32 realLen = 0;														//实际读到的数据个数

	if(pBuffer == 0) 														//指针保护
		return -1;	
	if(Len == 0) 																//长度保护
		return -1;			
// 	if(Fx == (MX251608D_Cfg*)0) 								//控制保护
// 		return 0;			
	
	if(MX251608D_Mutex == 0)		//互斥量未创建
		return -1;
	SysMutexPend(MX251608D_Mutex, 0, &err);		//获取互斥量		未获取到会一直等待
	
  MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x03);
	SPIbyteSendGet((ReadAddr & 0xFF0000) >> 16);
	SPIbyteSendGet((ReadAddr & 0xFF00) >> 8);
	SPIbyteSendGet(ReadAddr & 0xFF);
	
	while (Len--) 
	{
		*pBuffer = SPIbyteSendGet(0);					//获取数据
		pBuffer++;																			//下一地址
		realLen++;																			//实际读到的数据个数
	}
		
  MX251608D_CEH();												//关闭设备
	
	if(MX251608D_Mutex == 0)									//互斥量未创建
		return -1;
	SysMutexPost(MX251608D_Mutex);						//释放互斥量
	
// 	*RealLen = realLen;												//实际获取的数据个数
  return realLen;																	//读取完成
}

/****************************************************************************
* 名	称：u8 MX251608D_IsBusy(MX251608D_Cfg* Fx)
* 功	能：忙状态查询
* 入口参数：MX251608D_Cfg* Fx		设备
* 出口参数：u8		1忙	0闲
* 说	明：无
****************************************************************************/
u8 MX251608D_IsBusy()
{
	u8 Bflag = 0;																//忙标志
  u8 flashstatus = 0;
		
	MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x05);										//查询状态
	flashstatus = SPIbyteSendGet(0);
	
 	Bflag = flashstatus & 0x01;									//获取忙标志	[0]位上的标志是忙状态 1忙 0不忙
	MX251608D_CEH();													//关闭设备
	
	return Bflag;																//返回忙标志
}

/****************************************************************************
* 名	称：void MX251608D_BusyWait(MX251608D_Cfg* Fx)
* 功	能：忙等待
* 入口参数：MX251608D_Cfg* Fx		设备
* 出口参数：无
* 说	明：直至忙消除才会退出
****************************************************************************/
void MX251608D_BusyWait()
{
  u8 flashstatus = 1;													//初始化为忙状态
		
	MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x05);							//查询状态
	
	while ((flashstatus & 0x01) == 1)
		flashstatus = SPIbyteSendGet(0);
	
	MX251608D_CEH();													//关闭设备
}

/****************************************************************************
* 名	称：void MX251608D_Erase(MX251608D_Cfg* Fx)
* 功	能：擦除芯片
* 入口参数：MX251608D_Cfg* Fx	设备
* 出口参数：无
* 说	明:无
****************************************************************************/
void MX251608D_Erase()
{
	MX251608D_CEL();													//开启设备
	SPIbyteSendGet(0x06);							//写使能
	MX251608D_CEH();													//关闭设备
		
	MX251608D_CEL();													//开启设备
	SPIbyteSendGet(0xC7);							//芯片擦除
	
	MX251608D_CEH();													//关闭设备
	
	MX251608D_CEL();													//开启设备
	SPIbyteSendGet(0x04);							//写失能
	
	MX251608D_CEH();													//关闭设备
}

/****************************************************************************
* 名	称：void MX251608D_EraseSector(MX251608D_Cfg* Fx,u32 SectorAddr)
* 功	能：擦除扇区 4096 Byte
* 入口参数：MX251608D_Cfg* Fx		设备
						u32 SectorAddr			内部地址，非扇区号
* 出口参数：无
* 说	明:擦除当前地址所处扇区中的数据
一扇区	4096	Byte		256*16页
****************************************************************************/
void MX251608D_EraseSector(u32 SectorAddr)
{
	MX251608D_CEL();												//开启设备
	SPIbyteSendGet(0x06);						//写使能
	MX251608D_CEH();												//关闭设备
	
	MX251608D_CEL();												//开启设备
	SPIbyteSendGet(0x20);						//扇区擦除
	SPIbyteSendGet((SectorAddr & 0xFF0000) >> 16);
	SPIbyteSendGet((SectorAddr & 0xFF00) >> 8);
	SPIbyteSendGet(SectorAddr & 0xFF);
	
	MX251608D_CEH();													//关闭设备
	
	MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x04);							//写失能
	
	MX251608D_CEH();													//关闭设备	
}

/****************************************************************************
* 名	称：void MX251608D_EraseBlock(MX251608D_Cfg* Fx,u32 BlockAddr)
* 功	能：擦除块 65536 Byte
* 入口参数：BlockAddr  块地址
* 出口参数：无
* 说	明:一块		65536	Byte		4096*16扇区		256*256页
****************************************************************************/
void MX251608D_EraseBlock(u32 BlockAddr)
{	
	MX251608D_CEL();													//开启设备
	SPIbyteSendGet(0x06);							//写使能
	MX251608D_CEH();													//关闭设备
	
	MX251608D_CEL();													//开启设备
	SPIbyteSendGet(0xD8);							//块擦除
	SPIbyteSendGet((BlockAddr & 0xFF0000) >> 16);
	SPIbyteSendGet((BlockAddr & 0xFF00) >> 8);
	SPIbyteSendGet(BlockAddr & 0xFF);
    
    
	MX251608D_CEH();													//关闭设备
	
	MX251608D_CEL();													//开启设备
	
	SPIbyteSendGet(0x04);							//写失能
	
	MX251608D_CEH();													//关闭设备
}

/****************************************************************************
* 名	称：	void MX251608D_PageWrite(MX251608D_Cfg* Fx,u16 WriteAddr,u8* pBuffer,u32 Len)
* 功	能：	Flash的页写操作
* 入口参数：MX251608D_Cfg* Fx			设备
						u32 WriteAddr					页内地址
						u8* pBuffer						要写入的数据
						u32 Len								写入的数据长度
* 出口参数：无
* 说	明：一页256字节
					超过256字节的数据会从头再开始覆盖写入
					[0][0][0][0][0] <- 从[3]开始写入12345
					[3][4][5][1][2]
****************************************************************************/
void MX251608D_PageWrite(u8* pBuffer, u32 WriteAddr, u32 Len)
{
	if(pBuffer == 0) 																	//指针保护
		return;	
	if(Len == 0) 																			//长度保护
		return;			

	MX251608D_CEL();																//开启设备
	SPIbyteSendGet(0x06);										//写使能
	MX251608D_CEH();																//关闭设备
	
	MX251608D_CEL();																//开启设备
	SPIbyteSendGet(0x02);										//选择页
	SPIbyteSendGet((WriteAddr & 0xFF0000) >> 16);	//地址
	SPIbyteSendGet((WriteAddr & 0xFF00) >> 8);
	SPIbyteSendGet(WriteAddr & 0xFF);
	
	while (Len--)		//数据
	{
		SPIbyteSendGet(*pBuffer);
		pBuffer++;
	}
		
  MX251608D_CEH();															//关闭设备
	
	MX251608D_BusyWait();													//忙等待
	
	MX251608D_CEL();															//开启设备
	
	SPIbyteSendGet(0x04);									//写失能
	
	MX251608D_CEH();															//关闭设备
}

/****************************************************************************
* 名	称：u8 MX251608D_Write(MX251608D_Cfg* Fx,u32 Addr,u8* Buffer,u32 Len,u32* RealLen)
* 功	能：线性存储
* 入口参数：MX251608D_Cfg* Fx		设备
						u32 Addr						开始地址
						u8* Buffer					缓冲区数据
						u32 Len							数据长度
						u8 Num	 						器件号
* 出口参数：s32 存储是否成功	
* 说	明：
****************************************************************************/
s32 MX251608D_Write(u32 Addr,u8* Buffer,u32 Len)
{
	u8 err;
	u16 PageSurplusLen = 0;  				//页剩余空间长度 最大256
	u16 WLen = 0;										//当前存数据长度
	u32 RCnt = 0;										//实际写入的数据个数
	
	if(Buffer == 0) 								//指针保护
		return -1;	
	if(Len == 0) 										//长度保护
		return -1;			
// 	if(Fx == (MX251608D_Cfg*)0) 		//控制保护
// 		return 0;			
	
	//超过最大存储范围
	if((Addr + Len) > CapacitySize)   //0 - 2097152 (256 * 256)
		return -1;
	
	//写操作	 	
	while(Len > 0)		//数据没处理完
	{
		PageSurplusLen = PageSize - Addr % PageSize;					//计算从所处页的所处页内地址到页结束可以存多少个数据
		
		if(Len > PageSurplusLen)		//希望存储的数据个数是否超过当前所处页剩余容量
			WLen = PageSurplusLen;		//按剩余容量存储
		else
			WLen = Len;								//按希望存储的数据个数存储
		
		if(MX251608D_Mutex == 0)		//互斥量未创建
			return -1;
		SysMutexPend(MX251608D_Mutex, 0, &err);		//获取互斥量	未获取到会一直等待获取
		
		MX251608D_PageWrite(Buffer,Addr,WLen);	//页存储数据
		MX251608D_BusyWait();										//忙等待
		
		if(MX251608D_Mutex == 0)		//互斥量未创建
			return -1;
		SysMutexPost(MX251608D_Mutex);						//释放互斥量
		
		RCnt += WLen;								//实际写入的数据个数
		Len -= WLen;								//更新希望存储的数据个数
		Buffer += WLen;							//存储数据源偏移
		Addr += WLen;								//开始存储地址偏移
		
		if(Len != 0)								//数据没有存储完				
		{
			if((Addr % PageSize) != 0)			//地址检查 下一次数据一定为页开头开始取
				return -1;	
		}
	}
// 	*RealLen = RCnt;							//实际写入的数据个数
	return RCnt;
}

/****************************************************************************
* 名	称：static s8 MX251608D_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 MX251608D_Open(u32 lParam)
{
	return 0;					
}

/****************************************************************************
* 名	称：static s8 MX251608D_Close(void)
* 功	能：统一接口
* 入口参数：无
* 出口参数：s8						关闭操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 MX251608D_Close(void)
{
	return 0;		
}

/****************************************************************************
* 名	称：static s32 MX251608DRead(u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备读
* 入口参数：u32 offset		偏移地址
						u8* Buffer		读取数据存放位置
						u32 len				希望读取的数据长度
						u32* Reallen	实际读取的数据长度
						u32 lParam		参数
* 出口参数：s32					
* 说	明：无
****************************************************************************/
static s32 MX251608DRead(u32 offset,u8* Buffer,u32 len)
{
	return MX251608D_Read(offset,Buffer,len);
}

/****************************************************************************
* 名	称：static s32 MX251608DWrite(u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备写
* 入口参数：u32 offset		偏移地址
						u8* Buffer		写入数据源存放位置
						u32 len				希望写入的数据长度
* 出口参数：s32						操作是否成功	
* 说	明：无
****************************************************************************/
static s32 MX251608DWrite(u32 offset,u8* Buffer,u32 len)
{
	return MX251608D_Write(offset,Buffer,len);
}

/****************************************************************************
* 名	称：static s32 MX251608D_Ioctl(u32 cmd,u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 MX251608D_Ioctl(u32 cmd,u32 lParam)
{
	switch(cmd)
	{
		case MX251608D_CMDERASE:				//芯片擦除
		{
			MX251608D_Erase();
			return 1;							
		}
		case MX251608D_CMDBLOCKERASE:		//块擦除
		{
			MX251608D_EraseBlock(lParam);
			return 1;							
		}
		case MX251608D_CMDSECTORERASE:	//扇区擦除
		{
			MX251608D_EraseSector(lParam);
			return 1;			
		}
		case MX251608D_CMDISBUSY:				//忙检测
			return MX251608D_IsBusy();
			
		case MX251608D_CMDBUSYWAIT:			//忙等待
		{
			MX251608D_BusyWait();
			return 1;
		}
	}
	return 0;	//错误命令	
}

/****************************************************************************
* 名	称：u8 MX251608D_Init(u8 prio)
* 功	能：DataFlash初始化
* 入口参数：u8 prio			存储互斥量优先级
* 出口参数：u8		是否成功		1成功	0失败
* 说	明：无
****************************************************************************/
u8 MX251608D_Init(u8 prio)
{
	u8 err = 0;
// 	GPIO_InitTypeDef   G;
	
	#if(MX251608DA == 1)
	//注册用变量
	DEV_REG mx25a = 									//设备注册信息表						不必static		
	{
		BLOCK_DATAFLASH,								//设备ID号
		0,  														//是否共享			0:不可共享使用, 1:可共享使用
		0, 															//对于共享设备最大打开次数
		1,															//最大读次数
		1,															//最大写次数
		MX25A_pDisc,										//驱动描述			自定义
		20120001,												//驱动版本			自定义
		(u32*)MX251608D_Open,						//设备打开函数指针
		(u32*)MX251608D_Close, 					//设备关闭函数指针
		(u32*)MX251608DRead,						//块读函数
		(u32*)MX251608DWrite,						//块写函数
		(u32*)MX251608D_Ioctl						//控制函数
	};
	#endif
	
	#if(MX251608DA == 1)
	SPIx_Init();						//设备SPI初始化
	
// 	else if(MX25A.FCE_Port == GPIOC)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);		//开启引脚时钟C
	
//   G.GPIO_Pin = FLASH_FCE_PIN;																//CE引脚初始化
//   G.GPIO_Mode = GPIO_Mode_OUT;
//   G.GPIO_OType = GPIO_OType_PP;
//   G.GPIO_Speed = GPIO_Speed_50MHz;
//   G.GPIO_PuPd = GPIO_PuPd_NOPULL;
//   GPIO_Init(FLASH_FCE_PORT, &G);
	
	IoSetMode(FLASH_FCE,IO_MODE_OUTPUT);
	IoPushPull(FLASH_FCE);
	IoSetSpeed(FLASH_FCE,IO_SPEED_50M);
	
	MX251608D_CEH();																		//关闭设备
	
	if(MX251608D_Mutex == 0)
		MX251608D_Mutex = SysMutexCreate(prio, &err);						//创建互斥量  //提升到的优先级，该优先级不能被占用
	if(err != SYS_ERR_NONE)																		//互斥量创建失败
		return 0;	
	
	if(DeviceInstall(&mx25a) != HVL_NO_ERR)										//设备注册
		return 0;
	#endif
	
	return 1;
}


