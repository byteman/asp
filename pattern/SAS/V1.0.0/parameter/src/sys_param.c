#include "includes.h"
#include "sys_param.h"
#include "f24cl64.h"
#include "task_def.h"

static HANDLE param_handle;

u32 gSysStatus = 0;		//
SSYSSET gSysSet = {0,0,"0000",0}; // 0:kg  1:t //2012.12.12 lxj

void Param_Interface_Init(void)
{
	uint8 err;

 	if(FM24CL64_Init(FRAMMutexPrio) == 0)				//EEPROM初始化			优先级
 	{	
 		debug(Debug_None, "FRAM init failed!\r\n");
 		SysTaskSuspend(SYS_PRIO_SELF);
//		return;
	}
	
	param_handle = DeviceOpen(BLOCK_FRAMEEPROM,0,&err);				//设备打开
	if(err != HVL_NO_ERR)
	{
		debug(Debug_Error, "Error: FRAM设备打开失败\r\n");
		while(1);																										
	};
}

BOOL   Param_Write(int addr,  void* data, uint32 size)
{
	BOOL err = FALSE;
	uint32 len = 0;	
	len = BlockDeviceWrite(param_handle, addr, (u8 *)data, size);
	if( len <= 0) {
		debug(Debug_Warning, "Warning: FRAM write err!\r\n");
	} else {
		err = TRUE;
		if(len != size) {
			debug(Debug_Warning, "Warning: FRAM write failed write_size=%d, actual_write=%d!\r\n", size, len);
			err = FALSE;
		}
	}

	return err;
}


BOOL   Param_Read(int addr,  void* data, uint32 size)
{
	BOOL err = FALSE;
	uint32 len = 0;	
	
	len = BlockDeviceRead(param_handle, addr, (u8 *)data, size);
	if(len <= 0) {
		debug(Debug_Warning, "Warning: FRAM read err!\r\n");
	} else {
		err = TRUE;
		if(len != size) {
			debug(Debug_Warning, "Warning: FRAM read failed write_size=%d, actual_write=%d!\r\n", size, len);
			err = FALSE;
		}
	}

	return err;
}

/////////////////////////////////////////////////////////////////////////////////////
u8 SysSetParamSave(void)		//保存密码等设置信息
{
	return Param_Write(SYSPARAM_SYSSET, (u8 *)&gSysSet, sizeof(SSYSSET));
}

u8 SysInitParam(void)
{
	SSYSSET tmp = {0};

	Param_Read(SYSPARAM_SYSSET, (u8 *)&tmp, sizeof(SSYSSET));
	if((tmp.unit > 1) || (tmp.Beep > 1))
		return 0;

	memcpy(&gSysSet, &tmp, sizeof(SSYSSET));
	return 1;
}

u8 GetSysStatus(u32 bit)
{
    if(gSysStatus & bit) {
        return 1;
    } else {
        return 0;
    }
}

u8 SetSysStatus(u32 bit,u8 en)
{
    if(en) {
        gSysStatus |=  bit;
    } else {
        gSysStatus &= ~bit;
    }
    return 0;
}


