#ifndef BARRIERGATE_CTL_H
#define BARRIERGATE_CTL_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"

#define ESTATUE_NONE		254
#define ESTATUE_UPON		1
#define ESTATUE_DOWNOFF		0		 
		 
enum {
	ESTATUE_CTL_CLOSE= 0,
	ESTATUE_CTL_OPEN,
	ESTATUE_CTL_RESET
};		 
		 
enum {
	CLOSE_IDLE= 0,
	CLOSE_PREPARE,
	CLOSE_WAIT,
	CLOSE_PROTECT
};		 

typedef struct{

	u16 nCtrlTmr;		//道闸开始控制的启动延时时间
	u16 nCtrlIndex;		//道闸启动延时计数器
	
	u8  nEndCtrlFlag;		
	u8  nStartCtrlFlag;	//道闸控制开关
	u8  nCtrlStatue;
	
}CONTROL_TMR;

void BarrierGateAction_After_LC2DownOff(void);
void BarrierGate_Reset_Init(void);
void BarrierGate_Service(void);

void TrafficSignal_Business(void);
void barriergate_open(void);




#ifdef __cplusplus
}
#endif

#endif

