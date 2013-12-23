/*
 * 2013-3-16 存储数据时不再考虑数据类型, 数据是否有效由存储模块判断信息的校验和实现; 
 *		   校验和的存储地址存放在存储信息最后两字节, 存储空间由传入的存储信息提供
 */

#include "data_store_module.h"
#include "mx251608d.h"
#include "task_def.h"							//priorty
#include "sys_param.h"

#define START_ADDR_OF_NORMAL_VEHICLE					0x100000				//存储正常车辆数据的起始地址
#define END_ADDR_OF_NORMAL_VEHICLE					0x1FFFFF				//存储正常车辆数据的结束地址
#define START_ADDR_OF_OVERWEIGHT_VEHICLE			0x0						//存储超载车辆数据的起始地址
#define END_ADDR_OF_OVERWEIGHT_VEHICLE				0xFFFFF					//存储超载车辆数据的结束地址

static HVL_ERR_CODE err = HVL_NO_ERR;		//错误状态
static HANDLE MX251608D_Dev = 0;			//设备句柄

static uint16 gOverWeight_Num = 0;		//当前存储的超载车辆数

//计算校验和
static uint16 Calc_Check_Sum(uint8 *data, uint8 len)
{
	int i = 0;
	uint16 tmp = 0;

	for(; i < len; i++)
	{
		tmp += data[i];
	}

	return tmp;
}

//根据地址存储数据到flash
static BOOL Store_CarInfo_Flash(u32 addr, const void *data, uint8 len)
{
	int rlen = 0;	//实际读写Flash的长度
	uint16 verify = 0;
	u8 *pdata = (u8 *)data;

	verify = Calc_Check_Sum(pdata, len - 2);
	memcpy(&pdata[len-2], &verify, 2);
	
	rlen = BlockDeviceWrite(MX251608D_Dev, addr, pdata, len);
	if(rlen < 0)		//写入数据
	{
		debug(Debug_Error,"写Flash操作出错,addr:0x%06x\r\n",addr);
		return FALSE;																																//存储失败返回																									
	}	
	
	if(rlen != len)																						//写入的数据个数和希望写入的数据个数不匹配
	{
		debug(Debug_Notify,"写Flash数据不匹配出错\r\n");
		return FALSE;																																//存储失败返回
	}

	return TRUE;
}

//擦除flash
static BOOL Erase_CarInfo_Flash(u32 erase_addr)
{
	DeviceIoctl(MX251608D_Dev,MX251608D_CMDSECTORERASE,erase_addr);

	//擦除扇区60ms
	while(DeviceIoctl(MX251608D_Dev,MX251608D_CMDISBUSY,0))		//忙等待
	{
		SysTimeDly(2);																	
	}
	
	return TRUE;
}

static BOOL Check_Data_Valid(uint8 *buf, uint8 len)
{
	uint16 verify = 0, tverify = 0xff;

	verify = Calc_Check_Sum(buf, len - 2);
	memcpy(&tverify, &buf[len-2], 2);

	if(tverify != verify) return FALSE;

	return TRUE;
}

/*******   FLASH设备初始化函数   ********/
void init_flash(void)
{
	if(MX251608D_Init(FlashMutexPrio) == 0)								//设备A初始化				互斥量优先级9
	{
		debug(Debug_None,"初始化Flash失败\r\n");
		SysTaskSuspend(SYS_PRIO_SELF);
	}
	else
	{
		debug(Debug_Notify,"初始化flash成功!\r\n");
	}
	
	MX251608D_Dev = DeviceOpen(BLOCK_DATAFLASH,0,&err);								//设备打开
	if(err != HVL_NO_ERR)
	{
		debug(Debug_Notify,"Flash设备打开失败\r\n");
	}
	else
	{
		debug(Debug_Notify,"flash设备打开成功!\r\n");
	}
}

void Erase_Flash_Chip(void)
{
	DeviceIoctl(MX251608D_Dev,MX251608D_CMDERASE,0);

	//擦除芯片14s
	while(DeviceIoctl(MX251608D_Dev,MX251608D_CMDISBUSY,0)) 	//忙等待
	{
		SysTimeDly(100);																	
	}
}

uint16 Get_Flash_NormalCarNum(void)
{
	uint16 tmp = 0;
	
	Param_Read(GET_U16_OFFSET(ParamNormalCarNum), &tmp, sizeof(uint16));
	if(tmp > MAX_NORMAL_VEHICLE_NUMBER) tmp = 0;
	
	return tmp;		
}

uint16 Get_Flash_OverWetCarNum(void)
{
	uint16 tmp = 0;
	
	Param_Read(GET_U16_OFFSET(ParamOverWetCarNum), &tmp, sizeof(uint16));
	if(tmp > MAX_OVERWEIGHT_VEHICLE_NUMBER) tmp = 0;
	
	return tmp;		
}

uint16 Get_Flash_NormalCarNumFront(void)
{
	uint16 tmp = 0;
	
	Param_Read(GET_U16_OFFSET(ParamCarNumFront), &tmp, sizeof(uint16));
	if(tmp > MAX_NORMAL_VEHICLE_NUMBER) tmp = 0;

	return tmp;
}

uint16 Get_Flash_NormalCarNumRear(void)
{
	uint16 tmp = 0;
	
	Param_Read(GET_U16_OFFSET(ParamCarNumRear), &tmp, sizeof(uint16));
	if(tmp > MAX_NORMAL_VEHICLE_NUMBER) tmp = 0;

	return tmp;
}

//存储正常过车数据
//正常历史车辆数由车辆数、写指针、队头车辆位置维护
uint8 Store_Normal_Vehicle(const void *data, uint8 len)
{
#if 1
	int j = 0,i = 0;						//函数临时控制变量
	uint32 addr = 0;						//擦除Flash时提供的地址
	BOOL bErase = FALSE;					//逻辑变量，是否需要擦除dataFlash，初始化为false
	uint16 rear = 0, carnum = 0;
	uint16 front = 0;

	if((data == NULL) || (len < 2)) return FALSE;

	//获取队尾指针
	rear = Get_Flash_NormalCarNumRear();
		
	/*******判断存储时是否需要擦除flash********/
	/*******如果需要擦除，得到擦除地址*********/
	/*******根据擦除地址，擦除4KBflash*********/
	i = rear/15;			//擦除的块数
	
	j = rear%15;			//是否应该擦除
	if(j == 0) bErase = TRUE;
	
	if(bErase)
	{
		addr = i*4096 + START_ADDR_OF_NORMAL_VEHICLE;			//按4KB擦除时提供的起始地址
		if(!Erase_CarInfo_Flash(addr)) 
			return FALSE;
	}
	
	//向Flash写入车辆数据，实现正常车辆数据存储功能
	//计算烧写地址
	addr = rear*256 + START_ADDR_OF_NORMAL_VEHICLE;
	if(Store_CarInfo_Flash(addr, data, len))	
	{
		//增加写指针
		rear++;
		if(rear >= MAX_NORMAL_VEHICLE_NUMBER) rear = 0;
		Param_Write(GET_U16_OFFSET(ParamCarNumRear), &rear, sizeof(uint16));

		//读车辆数, 增加车辆数
		carnum = Get_Flash_NormalCarNum();
		carnum++;
		if(carnum > MAX_NORMAL_VEHICLE_NUMBER)
		{
			//车辆数满后丢掉一个扇区的车数
			carnum = MAX_NORMAL_VEHICLE_NUMBER - 16 + 1;
			
			//回绕,读队头指针
			front = Get_Flash_NormalCarNumFront();
			front += 16;
			if(front >= MAX_NORMAL_VEHICLE_NUMBER) front = 0;
			Param_Write(GET_U16_OFFSET(ParamCarNumFront), &front, sizeof(uint16));
		}
		//写车辆数
		Param_Write(GET_U16_OFFSET(ParamNormalCarNum), &carnum, sizeof(uint16));
		
		debug(Debug_Notify,"向Flash写第%d个车辆数据,addr:0x%06x\r\n",carnum,addr);		

		return TRUE;																																//存储成功返回
	}

	return FALSE;
#else
	return TRUE;
#endif
}


//存储超载车辆数据
uint8 Store_Overweight_Vehicle(const void *data, uint8 len)
{
#if 1
	int j = 0,i = 0;						//函数临时控制变量
	uint32 addr = 0;						//擦除Flash时提供的地址
	BOOL bErase = FALSE;					//逻辑变量，是否需要擦除dataFlash，初始化为false

	if((data == NULL) || (len < 2)) return FALSE;

	Param_Read(GET_U16_OFFSET(ParamOverWetCarNum), &gOverWeight_Num, sizeof(uint16));

	//超限车辆数不存在回绕
	if(gOverWeight_Num >= MAX_OVERWEIGHT_VEHICLE_NUMBER)	return FALSE;
		
	/*******判断存储时是否需要擦除flash********/
	/*******如果需要擦除，得到擦除地址*********/
	/*******根据擦除地址，擦除4KBflash*********/
	i = gOverWeight_Num/15;			//擦除的块数
	
	j = gOverWeight_Num%15;			//是否应该擦除
	if(j == 0) bErase = TRUE;
	
	if(bErase)
	{
		addr = i*4096 + START_ADDR_OF_OVERWEIGHT_VEHICLE;	//按4KB擦除时提供的起始地址
		if(!Erase_CarInfo_Flash(addr)) 
			return FALSE;
	}
	
	//向Flash写入车辆数据，实现正常车辆数据存储功能
	//计算烧写地址
	addr = gOverWeight_Num*256 + START_ADDR_OF_OVERWEIGHT_VEHICLE;
	if(Store_CarInfo_Flash(addr, data, len))	
	{	
		//写入车辆数
		gOverWeight_Num++;
		if(gOverWeight_Num > MAX_OVERWEIGHT_VEHICLE_NUMBER) gOverWeight_Num = MAX_OVERWEIGHT_VEHICLE_NUMBER;
		Param_Write(GET_U16_OFFSET(ParamOverWetCarNum), &gOverWeight_Num, sizeof(uint16));

		debug(Debug_Notify,"向Flash写第%d个超载车辆数据,addr:0x%06x\r\n",gOverWeight_Num,addr);
		return TRUE;	
	}

	return FALSE;
#else
	return TRUE;
#endif
}

static uint8 TmpCarinfoBuf[256] = {0};			//存储取出来的车辆信息

//查询正常过车数据
void *Query_Normal_Vehicle(uint16 id, uint8 len)
{
	uint32 rlen = 0;
	uint32 readAddr = 0;				//读DataFlash时提供的地址

	if((id < 1) || (id > MAX_NORMAL_VEHICLE_NUMBER)) return NULL;
	if(len < 2) return NULL;

	while(DeviceIoctl(MX251608D_Dev,MX251608D_CMDISBUSY,0)) 	//忙等待
	{
		SysTimeDly(3);																	
	}
	
	readAddr = (id-1)*256 + START_ADDR_OF_NORMAL_VEHICLE;		//读DataFlash时提供的地址

	rlen = BlockDeviceRead(MX251608D_Dev, readAddr, TmpCarinfoBuf, len);
	if(rlen <= 0)	//读取
	{
		debug(Debug_Error,"Flash读取出错\r\n");
		return NULL;
	}
	
	if(rlen != len)																						//读取的数据个数和希望读取的数据个数不匹配
	{
		debug(Debug_Notify,"Flash数据不匹配出错\r\n");
		return NULL;
	}		
	else
	{
		debug(Debug_Notify,"从Flash读第%d个正常车辆数据,addr:0x%06x\r\n",id,readAddr);
	}

	if(!Check_Data_Valid(TmpCarinfoBuf, len))
	{
		debug(Debug_Error, "Query CarInfo invalid!\r\n");
		return NULL;
	}

	return TmpCarinfoBuf;
}

// 查询超载车辆数据
void *Query_Overweight_Vehicle(uint16 id, uint8 len)
{
	uint32 rlen = 0;
	uint32 readAddr = 0;				//读DataFlash时提供的地址
	
	if((id < 1) || (id > MAX_OVERWEIGHT_VEHICLE_NUMBER)) return NULL;
	if(len < 2) return NULL;
	
	Param_Read(GET_U16_OFFSET(ParamOverWetCarNum), &gOverWeight_Num, sizeof(uint16));
	if(id > gOverWeight_Num)	return NULL;

	while(DeviceIoctl(MX251608D_Dev,MX251608D_CMDISBUSY,0)) 	//忙等待
	{
		SysTimeDly(3);																	
	}
	
	readAddr = (id-1)*256 + START_ADDR_OF_OVERWEIGHT_VEHICLE;		//读DataFlash时提供的地址

	rlen = BlockDeviceRead(MX251608D_Dev,readAddr,TmpCarinfoBuf, len);
	if(rlen <= 0)	//读取
	{
		debug(Debug_Notify,"Flash读取出错\r\n");
		return NULL;
	}
	if(rlen != len)																						//读取的数据个数和希望读取的数据个数不匹配
	{
		debug(Debug_Notify,"Flash数据不匹配出错\r\n");
		return NULL;
	}		
	else
	{
		debug(Debug_Notify,"从Flash读第%d个超载车辆数据,addr:0x%06x\r\n",id,readAddr);
	}

	if(!Check_Data_Valid(TmpCarinfoBuf, len))
	{
		debug(Debug_Error, "Query CarInfo invalid!\r\n");
		return NULL;
	}

	return TmpCarinfoBuf;
}

