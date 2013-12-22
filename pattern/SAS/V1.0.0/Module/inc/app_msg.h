#ifndef APP_MSG_H
#define APP_MSG_H
	 
#ifdef __cplusplus
	 extern "C" {
#endif

#include "includes.h"
#include "task_msg.h"

//任务消息号基址，根据需要修改
enum {
	Msg_Data_Base		= 100,
	Msg_Business_Base = 200,
	Msg_NetSend_Base	= 300,
};

//业务处理线程接收消息
//消息号的排列要与scszc_debug_info.c里的 strMsgArr 字符数组相匹配
enum{
	Msg_None  =  Msg_Business_Base,

/*整车式消息号*/
	Msg_Reset,			//系统复位
	Msg_Refresh,		//系统刷新
	Msg_GC1_Enter,		//前地感触发
	Msg_GC1_Leave,		//前地感下沿
	Msg_GC2_Enter,		//后地感
	Msg_GC2_Leave,		
	Msg_LC1_Enter,		//前光幕上沿
	Msg_LC1_Leave,		//前光幕下沿
	Msg_LC2_Enter,	  	//分车光幕
	Msg_LC2_Leave,
	Msg_PreAlex_Come,		//前轴识别器进轴
	Msg_PreAlex_Add,		//前轴识别器进轴
	Msg_PreAlex_Remove,	//前轴识别器倒轴
	Msg_BackAlex,			//后轴识别器进轴
	Msg_Cmd_Repeat,		//重新采集
	Msg_Cmd_PlusSave,		//分段采集命令
	Msg_Cmd_PayNotify,
	Msg_Cmd_Force,			//强制采集命令
	Msg_Tick,				//10ms时钟

	Msg_WBAxle				//弯板轴信息
};

/*双秤台消息号*/
enum {
	VP_GM_TRIG = Msg_Business_Base + 50,      // 光幕触发  		=250
	VP_GM_RELE,         // 光幕释放			
	VP_DG_TRIG,         // 地感触发		
	VP_DG_RELE,         // 地感释放		
	VP_LZ_SINGLE,       // 单轮			
	VP_LZ_DOUBLE,       // 双轮				
	VP_VEHUP_A,         // A 上称		
	VP_VEHDOWN_A,       // A 下称			
	VP_VEHUP_B,         // B 上称		
	VP_VEHDOWN_B,       // B 下称			
	VP_MSG_TICK,		// 2ms时钟消息		
};

//业务数据管理线程接收消息
enum {
	Msg_Device_Heart = Msg_Data_Base,	 //10s定时上报外设状态, 秤上有车不上传
	Msg_Device_Fault,		//外设即时故障信息
	Msg_Overtime,			//超时信息
	Msg_BackCarInfo,		//倒车信息
	Msg_CarInfo_Ready		//车辆信息
};

//网络发送线程接收消息
enum {
	Msg_NetSend = Msg_NetSend_Base,
	Msg_NetDebug
};
	
typedef struct 
{
	uint32 priv;		//用户自定义指针
	uint16 msg_id;		//消息号
	uint16 msg_len;		//数据长度
	uint32 msg_ctx;		//数据指针
}TaskMsg;

//应用层消息队列全局变量
extern MsgQ NetMsgQ;
extern MsgQ BusinessMsgQ; 
extern MsgQ DataManagerMsgQ;

/*应用层消息队列初始化*/
void App_Msg_Init(void);

/* 发送传感器ad值与设备状态  */
BOOL Net_Send_Wave(void *buf);


/* 接收到消息释放内存 */
uint8 Wave_Mem_Free(void *pblk);

#ifdef __cplusplus
}
#endif

#endif


