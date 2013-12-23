#include "driver.h"
#include "ccm_mem.h"

typedef struct
{
	u32 					ver;					//驱动版本			自定义
	u8* 					pDisc;				//驱动描述			自定义
	u8 						canShared;  	//是否共享			0:不可共享使用, 1:可共享使用
	u8						OpenMax; 			//共享设备最大打开次数
	u8 						OpenCount; 		//打开计数
	u8 						ReadMax; 			//最大读次数
	u8 						ReadCount;		//读计数
	u8 						WriteMax; 		//最大写次数
	u8 						WriteCount;		//写计数
	u8 						UseFlag;  		//设备启用标志				0：未启用	1：启用
	u32*					devOpen;    	//设备打开函数指针
	u32*			 		devClose;   	//设备关闭函数指针
	u32*		  		devRead;			//设备读函数映射指针
	u32*			 		devWrite;			//设备写函数映射指针
	u32*			 		devIoctl;			//设备控制函数映射指针
}DEV_SPACE;										//设备空间信息

//设备注册管理单元
//static DEV_SPACE DEV_List[DEV_MAX - 1];			//设备空间	有个边界值
static DEV_SPACE *DEV_List = NULL; 				

//中断注册管理单元
static SDEV_IRQ_REGS EXTI_List[16] = 				//中断映射表		设备ID，中断线号，中断函数指向		//字符和块共用此中断注册
{
	{0,0,(pIRQHandle)0},			//0
	{0,0,(pIRQHandle)0},			//1
	{0,0,(pIRQHandle)0},			//2
	{0,0,(pIRQHandle)0},			//3
	{0,0,(pIRQHandle)0},			//4
	{0,0,(pIRQHandle)0},			//5
	{0,0,(pIRQHandle)0},			//6
	{0,0,(pIRQHandle)0},			//7
	{0,0,(pIRQHandle)0},			//8
	{0,0,(pIRQHandle)0},			//9
	{0,0,(pIRQHandle)0},			//10
	{0,0,(pIRQHandle)0},			//11
	{0,0,(pIRQHandle)0},			//12
	{0,0,(pIRQHandle)0},			//13
	{0,0,(pIRQHandle)0},			//14
	{0,0,(pIRQHandle)0}				//15
};								

static Dev_Info QDevInfo = 					//设备查询表
{
	0,				//驱动版本			自定义
	(u8*)0,		//驱动描述地址	自定义
	0,  			//是否共享			0:不可共享使用, 1:可共享使用
	0, 				//对于共享设备最大打开次数
	0, 				//对于共享设备目前打开次数
	0, 				//最大读次数
	0,				//目前读次数
	0, 				//最大写次数
	0					//目前写次数
};

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceInstall(DEV_REG *dev)
* 功	能：设备注册
* 入口参数：DEV_REG *dev		设备注册信息
* 出口参数：HVL_ERR_CODE		是否注册成功  成功HVL_NO_ERR		其它失败
* 说	明：对所有类型的设备注册		
****************************************************************************/
HVL_ERR_CODE DeviceInstall(DEV_REG *dev)
{
	if(dev->ID > (DEV_MAX - 1))										//超过最大设备ID号
		return HVL_INSTALL_FAIL;										//注册失败
		
	if(dev->ID > CHAR_BLOCK_BOUNDARY)							//块设备区
		dev->ID -= 1;																//减去一个设备占位符号
	
	if(DEV_List[dev->ID].UseFlag > 0)							//如果已经注册了该设备
		return HVL_INSTALL_FAIL;										//注册失败
	
	DEV_List[dev->ID].canShared = dev->canShared;	//是否共享			0:不可共享使用, 1:可共享使用
	DEV_List[dev->ID].OpenMax = dev->OpenMax;			//共享设备最大打开次数
	DEV_List[dev->ID].ReadMax = dev->ReadMax;			//最大读次数
	DEV_List[dev->ID].WriteMax = dev->WriteMax;		//最大写次数
	DEV_List[dev->ID].pDisc = dev->pDisc;					//驱动描述			自定义
	DEV_List[dev->ID].ver = dev->ver;							//驱动版本			自定义
	DEV_List[dev->ID].devOpen = (u32*)dev->devOpen;			//打开函数
	DEV_List[dev->ID].devClose = (u32*)dev->devClose;		//关闭函数
	DEV_List[dev->ID].devRead = (u32*)dev->devRead;			//读函数
	DEV_List[dev->ID].devWrite = (u32*)dev->devWrite;		//写函数
	DEV_List[dev->ID].devIoctl = (u32*)dev->devIoctl;		//控制函数
	DEV_List[dev->ID].OpenCount = 0;								//打开计数
	DEV_List[dev->ID].ReadCount = 0;								//读计数
	DEV_List[dev->ID].WriteCount = 0;								//写计数
	DEV_List[dev->ID].UseFlag = 1;									//启用标志			//1启用设备		0没启用设备

	return HVL_NO_ERR;															//注册成功
}

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数
						HVL_ERR_CODE* err		错误类型
* 出口参数：HANDLE							设备句柄
* 说	明：如果是非共享设备，有过一次打开后就不能再打开了	
****************************************************************************/
HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
{
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return HVL_OPEN_DEV_FAIL;										//打开失败
	
	if(ID > CHAR_BLOCK_BOUNDARY)									//块设备区
		ID -= 1;																		//减去一个设备占位符号
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//已经打开过设备
		{
			if((DEV_List[ID].canShared == 0) || (DEV_List[ID].OpenCount == DEV_List[ID].OpenMax))			//非共享和最大打开次数
			{
				*err = HVL_OPEN_DEV_FAIL;							//已打开非共享设备返回错误	
				return HANDLENULL;										//返回空句柄
			}
			
			DEV_List[ID].OpenCount++;								//设备打开次数增加
			*err = HVL_NO_ERR;											//没有错误
			return (HANDLE)ID;											//返回用户句柄
		}

		if(((pDevOpen)(DEV_List[ID].devOpen))(lParam) == 0)			//调用设备的打开函数		设备Open返回为0表示成功
		{
			DEV_List[ID].OpenCount++;								//设备打开次数增加
			*err = HVL_NO_ERR;											//没有错误
			return (HANDLE)ID;											//返回设备句柄
		}
	}
	*err = HVL_OPEN_DEV_FAIL;										//设备打开失败
	return HANDLENULL;													//返回空句柄
}

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误类型
* 说	明：无			
****************************************************************************/
HVL_ERR_CODE DeviceClose(HANDLE IDH)
{
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return HVL_CLOSE_DEV_FAIL;									//关闭失败
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//检查是否已经打开
		{
			DEV_List[ID].OpenCount--;									//打开次数减少
			
			if(DEV_List[ID].OpenCount == 0)						//没有打开或关闭了最后的一次打开
			{
				if(((pDevClose)(DEV_List[ID].devClose))() > 0)				//调用设备的关闭函数		设备Close返回为0标识成功
					return HVL_CLOSE_DEV_FAIL;						//设备关闭失败
			}
			return HVL_NO_ERR;												//关闭成功
		}
	}
	return HVL_CLOSE_DEV_FAIL;										//设备关闭失败
}

/****************************************************************************
* 名	称：s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：字符设备读
* 入口参数：HANDLE IDH					设备句柄
						u8* Buffer					读取存放缓冲区
						u32 len							希望读取的数据个数
						u32* Reallen				实际读取的数据个数
						u32 lParam					参数
* 出口参数：s32									是否成功		1成功	0失败
* 说	明：无			
****************************************************************************/
s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len)
{
	s32 ReturnVal = 0;
	
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return 0;																		//READ失败
	
	if(ID > CHAR_BLOCK_BOUNDARY)									//块设备区ID
		return 0;																		//READ失败
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//设备打开过
		{
			if(DEV_List[ID].ReadCount >= DEV_List[ID].ReadMax)
				return 0;																//读操作失败
			
			DEV_List[ID].ReadCount++;									//读操作次数增加

			ReturnVal = ((pCDevRead)(DEV_List[ID].devRead))(Buffer,len);		//设备读操作
			
			DEV_List[ID].ReadCount--;									//读操作次数减少
			
			return ReturnVal;													//操作完成
		}
	}
	return 0;
}

/****************************************************************************
* 名	称：s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：字符设备写
* 入口参数：HANDLE IDH			设备句柄
						u8* Buffer			设备写数据源缓冲区
						u32 len					希望写入的长度
						u32* Reallen		实际写入的数据个数
						u32 lParam			参数
* 出口参数：s32 						实际写入的数据个数
* 说	明：无			
****************************************************************************/
s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len)
{
	s32 ReturnVal = 0;
	
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return 0;																		//READ失败
	
	if(ID > CHAR_BLOCK_BOUNDARY)									//块设备区ID
		return 0;																		//READ失败
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//设备打开过
		{
			if(DEV_List[ID].WriteCount >= DEV_List[ID].WriteMax)
				return 0;																//写操作失败
			
			DEV_List[ID].WriteCount++;								//写操作次数增加

			ReturnVal = ((pCDevWrite)(DEV_List[ID].devWrite))(Buffer,len);		//设备写操作
			
			DEV_List[ID].WriteCount--;								//写操作次数减少
			
			return ReturnVal;													//操作完成
		}
	}
	return 0;
}

/****************************************************************************
* 名	称：s32 BlockDeviceRead(HANDLE IDH,u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备读
* 入口参数：HANDLE IDH					设备句柄
						u32 offset					开始地址
						u8* Buffer					读取存放缓冲区
						u32 len							希望读取的数据个数
						u32* Reallen				实际读取的数据个数
						u32 lParam					参数
* 出口参数：s32									实际读取的数据个数
* 说	明：无			
****************************************************************************/
s32 BlockDeviceRead(HANDLE IDH,u32 offset,u8* Buffer,u32 len)
{
	s32 ReturnVal = 0;
	
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return 0;																		//READ失败
	
	if(ID <= (CHAR_BLOCK_BOUNDARY - 1))						//字符设备区ID
		return 0;																		//READ失败
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//设备打开过
		{
			if(DEV_List[ID].ReadCount >= DEV_List[ID].ReadMax)
				return 0;																//读操作失败
			
			DEV_List[ID].ReadCount++;									//读操作次数增加

			ReturnVal = ((pBDevRead)(DEV_List[ID].devRead))(offset,Buffer,len);		//设备读操作
			
			DEV_List[ID].ReadCount--;									//读操作次数减少
			
			return ReturnVal;													//操作完成
		}
	}
	return 0;
}

/****************************************************************************
* 名	称：s32 BlockDeviceWrite(HANDLE IDH,u32 offset,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：块设备写
* 入口参数ANDLE IDH						设备句柄
						u32 offset					开始地址
						u8* Buffer					设备写数据源缓冲区
						u32 len							希望写入的数据长度
						u32* Reallen				实际写入的数据长度
						u32 lParam					参数
* 出口参数：s32									实际写入的数据长度
* 说	明：无			
****************************************************************************/
s32 BlockDeviceWrite(HANDLE IDH,u32 offset,u8* Buffer,u32 len)
{
	s32 ReturnVal = 0;
	
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return 0;																		//READ失败
	
	if(ID <= (CHAR_BLOCK_BOUNDARY - 1))						//字符设备区ID
		return 0;																		//READ失败
	
	if(DEV_List[ID].UseFlag > 0)									//注册了该设备
	{
		if(DEV_List[ID].OpenCount > 0)							//设备打开过
		{
			if(DEV_List[ID].WriteCount >= DEV_List[ID].WriteMax)
				return 0;																//写操作失败
			
			DEV_List[ID].WriteCount++;								//写操作次数增加

			ReturnVal = ((pBDevWrite)(DEV_List[ID].devWrite))(offset,Buffer,len);		//设备写操作
			
			DEV_List[ID].WriteCount--;								//写操作次数减少
			
			return ReturnVal;													//操作完成
		}
	}
	return 0;
}

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备操作
* 入口参数：HANDLE IDH					设备句柄
						u32 cmd							命令
						u32 lParam					参数		根据命令的不同参数值不同
* 出口参数：s32									返回值	根据命令的不同返回值不同
* 说	明：无			
****************************************************************************/
s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
{
	DEV_ID ID = (DEV_ID)IDH;
	
	if(ID > (DEV_MAX - 1))												//超过最大设备ID号
		return 0;																		//READ失败		
	
	if(DEV_List[ID].UseFlag > 0)																	//注册了该设备
		return ((pDevIoctl)(DEV_List[ID].devIoctl))(cmd,lParam);		//设备操作
	return 0;
}

/****************************************************************************
* 名	称：s8 EXTIIRQHandlerInstall(SDEV_IRQ_REGS *irq)
* 功	能：中断注册
* 入口参数：SDEV_IRQ_REGS *irq				中断注册信息
* 出口参数：s8 中断注册是否成功		0成功 非0失败
* 说	明：字符设备和块设备共用中断注册管理		
****************************************************************************/
s8 EXTIIRQHandlerInstall(SDEV_IRQ_REGS *irq)
{
  if(irq->EXTI_No > 15) 				//0-15
		return -1;                  //超出范围
	
	if(EXTI_List[irq->EXTI_No].EXTI_Flag == 1) 	//已存在
			return -1;          			
	
	EXTI_List[irq->EXTI_No].EXTI_No = irq->EXTI_No; 				//中断号
	EXTI_List[irq->EXTI_No].EXTI_Flag = irq->EXTI_Flag; 		//标志
	EXTI_List[irq->EXTI_No].IRQHandler = irq->IRQHandler; 	//中断函数

	return 0;
}

/****************************************************************************
* 名	称：SDEV_IRQ_REGS* GetExti_list(void)
* 功	能：获取中断映射管理单元
* 入口参数：无
* 出口参数：SDEV_IRQ_REGS		中断映射表地址
* 说	明：无			
****************************************************************************/
SDEV_IRQ_REGS* GetExti_list(void)
{
	return EXTI_List;		//返回中断映射表
}

/****************************************************************************
* 名	称：void DevList_init(void)
* 功	能：设备管理单元列表初始化
* 入口参数：无
* 出口参数：无
* 说	明：无			
****************************************************************************/
void DevList_init(void)
{
	u8 i;

	DEV_List = (DEV_SPACE *)Ccm_Malloc((DEV_MAX - 1) * sizeof(DEV_SPACE));
	if(DEV_List == NULL) while(1);
	
	for(i = 0;i < (DEV_MAX - 1);i++)							//有个标志位
	{
		DEV_List[i].ver					= 0;								//驱动版本			自定义
		DEV_List[i].pDisc				= (u8*)0;						//驱动描述			自定义
		DEV_List[i].canShared		= 0;  							//是否共享			0:不可共享使用, 1:可共享使用
		DEV_List[i].OpenMax			= 0; 								//共享设备最大打开次数
		DEV_List[i].OpenCount		= 0; 	 							//打开计数
		DEV_List[i].ReadMax			= 0; 								//最大读次数
		DEV_List[i].ReadCount		= 0;								//读计数
		DEV_List[i].WriteMax		= 0; 								//最大写次数
		DEV_List[i].WriteCount	= 0;								//写操作计数
		DEV_List[i].UseFlag			= 0;  							//设备启用标志				0：未启用	1：启用
		DEV_List[i].devOpen			= (u32*)0;  				//块设备打开函数指针
		DEV_List[i].devClose		= (u32*)0;  				//块设备关闭函数指针
		DEV_List[i].devRead			= (u32*)0;					//块设备读函数映射指针
		DEV_List[i].devWrite		= (u32*)0;					//块设备写函数映射指针
		DEV_List[i].devIoctl		= (u32*)0;					//块设备控制函数映射指针
	}
}

/****************************************************************************
* 名	称：Dev_Info* QueryDev_Info(DEV_ID ID)
* 功	能：查看设备信息
* 入口参数：DEV_ID ID								设备ID号
* 出口参数：Dev_Info*								设备信息		没有则返回0									
* 说	明：有自己的查询缓冲区，修改不会影响到设备
****************************************************************************/
Dev_Info* QueryDev_Info(DEV_ID ID)
{
	if(ID > (DEV_MAX - 1))														//超过最大设备ID号
		return (Dev_Info*)0;
	
 	if(ID > CHAR_BLOCK_BOUNDARY)											//在块设备区
		ID -= 1;																				//有一个占位减除
	
	if(DEV_List[ID].UseFlag > 0)											//启用了该设备
	{
		QDevInfo.ver = DEV_List[ID].ver;								//设备版本号
		QDevInfo.pDisc = DEV_List[ID].pDisc;						//设备描述
		QDevInfo.canShared = DEV_List[ID].canShared;		//设备是否共享	0:不可共享使用, 1:可共享使用
		QDevInfo.OpenMax = DEV_List[ID].OpenMax;				//设备最大打开次数
		QDevInfo.OpenCount = DEV_List[ID].OpenCount;		//设备打开次数
		QDevInfo.ReadMax = DEV_List[ID].ReadMax;				//设备最大读次数
		QDevInfo.ReadConut = DEV_List[ID].ReadCount;		//目前Read次数
		QDevInfo.WriteMax = DEV_List[ID].WriteMax;			//设备最大写次数
		QDevInfo.WriteConut = DEV_List[ID].WriteCount;	//目前Write次数
		
		return &QDevInfo;																//返回查询的信息
	}

	return (Dev_Info*)0;															//没找到设备值
}
