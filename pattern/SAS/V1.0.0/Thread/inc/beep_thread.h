#ifndef _BEEPJOB_INIT_H	
#define _BEEPJOB_INIT_H

#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"
#include "beep_init.h"
	
/****************************************************************************
* 名	称：u8 BEEPJOB_Init(u8 PRIO_t,SYS_EVENT* JOBEVENT)
* 功	能：BeepJOB初始化
* 入口参数：u8 PRIO_t								任务优先级
						SYS_EVENT* JOBEVENT			鸣叫消息队列
* 出口参数：u8 					是否成功	1成功	0失败
* 说	明：无
****************************************************************************/
u8 BEEPJOB_Init(u8 PRIO_t,SYS_EVENT* JOBEVENT);
	


#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif


