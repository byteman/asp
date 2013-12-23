#ifndef SCSZC_DEBUG_INFO_H
#define SCSZC_DEBUG_INFO_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"
#include "app_msg.h"


void MsgInfo_Print(TaskMsg *pMsg);
void CarComfirm_Print(void);
//打印车辆队列车辆信息
void CarQueue_Print_Info(void);
const char *Get_Car_StatusName(int32 id);
const char *Get_Car_LevelName(uint8 id);
const char *Get_Car_TryeType(uint8 nCount);

//注册串口调试命令
void SCSZC_Debug_Cmd_Init(void);

#ifdef __cplusplus
}
#endif

#endif

