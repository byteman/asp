/* 
 * File:    debug_info.h
 * Brief:   print debug info
 *
 * History:
 * 1. 2012-11-16 创建文件, 从云南项目移植 river
 * 2. 2012-11-28 添加分级打印函数debug    river
 * 3. 2013-3-7   增加串口1 shell
 */

#ifndef DEBUG_INFO_H
#define DEBUG_INFO_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"
#include "app_msg.h"

//打印等级
enum {
	Debug_None = 0,
	Debug_Error,
	Debug_Business,
	Debug_Warning,
	Debug_Notify,
	Debug_Constant			// 连续的打印，从不停止
};	

//串口交互shell
typedef int (*DebugFun)(char *arg);
int Register_Debug_Cmd(char *name, DebugFun fun, char *help, BOOL bParam);

//调试串口输出, 注意该函数不可多个线程调用
int Usart1_WriteData(char *pbuf, u32 len);
int Usart1_ReadData(char *pbuf, u32 len);
void Usart1_Clear_Buffer(void);
void Usart1_Set_OvertimeTick(uint32 tick);



//网络输出调试信息
uint8 Debug_Mem_Free(void *pblk);
int debug_get_info(char *buf);

//调试信息打印等级设置
void Set_Debug_Level(uint8 level);
int8 Get_Debug_Level(void);

//网络调试信息输出
void debug(int8 level, const char *fnt, ...);

//调试串口初始化
void Print_Debug_Init(void);
void Debug_Init(void);

#ifdef __cplusplus
}
#endif

#endif
