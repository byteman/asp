#include "includes.h"
#include "task_msg.h"
#include "app_msg.h"
#include "device_info.h"
#include "net_business_thread.h"
#include "rmcp.h"
#include "ccm_mem.h"

#define WaveLen 		8		//4个块波形发送丢包率10%,6个基本不丢包
#define WaveSize		(320 + RMCP_HEAD_LEN + 1)
#define WavePackLen	20

//升级网络发送内存池
#define NetUpdateMsgLen    8
#define NetUpdateMsgSize (RMCP_HEAD_LEN + UPDATE_MSG_LEN)

#define AppNetUpdateMsgLen 8
#define AppNetMsgLen		16
#define AppBusinessMsgLen	32
#define AppDataMsgLen		24

#define MsgQSize sizeof(TaskMsg)

//应用层消息队列全局变量
MsgQ NetMsgQ = {0};
MsgQ BusinessMsgQ = {0};
MsgQ DataManagerMsgQ = {0};
MsgQ NetUpdateMsgQ = {0};

//发送波形数据内存缓冲池
// static  int WaveBuf[WaveLen][WaveSize] = {0};
static char *pWaveBuf = NULL;

//升级数据内存池
// static  int UpdateMsgBuf[NetUpdateMsgLen][NetUpdateMsgSize] = {0};
// static  int UpdateDataBuf[NetUpdateDataLen][NetUpdateDataSize] = {0};
static char *pUpdateMsgBuf = NULL;
static char *pUpdateDataBuf = NULL;

//消息队列内存缓冲池
// static  char NetMsgQBuf[AppNetMsgLen][MsgQSize] = {0};
// static  char BusinessMsgQBuf[AppBusinessMsgLen][MsgQSize] = {0};
// static  char DataMsgQBuf[AppDataMsgLen][MsgQSize] = {0};
// static  char NetUpdateMsgQBuf[AppNetUpdateMsgLen][MsgQSize] = {{0}};
static  char *pNetMsgQBuf = NULL;
static  char *pBusinessMsgQBuf = NULL;
static  char *pDataMsgQBuf = NULL;
static  char *pNetUpdateMsgQBuf = NULL;

//消息队列指针数组
static void *TaskMsgNetPoint[AppNetMsgLen];	
static void *TaskMsgBusinessPoint[AppBusinessMsgLen];	 
static void *TaskMsgDataPoint[AppDataMsgLen];
static void *TaskMsgNetUpdatePoint[AppNetUpdateMsgLen];

static SYS_MEM *pWaveMem = NULL;
static SYS_MEM *pUpdateMsgMem = NULL;
static SYS_MEM *pUpdateDataMem = NULL;

void App_Msg_Init(void)
{
	uint8 err;

	pWaveBuf 		= (char *)Ccm_Malloc(WaveLen * WaveSize * sizeof(char));
	pUpdateMsgBuf	= (char *)Ccm_Malloc(NetUpdateMsgLen * NetUpdateMsgSize * sizeof(char));
	pUpdateDataBuf	= (char *)Ccm_Malloc(NetUpdateDataLen * NetUpdateDataSize * sizeof(char));

	pNetMsgQBuf		= (char *)Ccm_Malloc(AppNetMsgLen * MsgQSize * sizeof(char));
	pBusinessMsgQBuf	= (char *)Ccm_Malloc(AppBusinessMsgLen * MsgQSize * sizeof(char));
	pDataMsgQBuf		= (char *)Ccm_Malloc(AppDataMsgLen * MsgQSize * sizeof(char));
	pNetUpdateMsgQBuf	= (char *)Ccm_Malloc(AppNetUpdateMsgLen * MsgQSize * sizeof(char));

	pWaveMem = SysMemCreate(pWaveBuf,WaveLen,WaveSize,&err);
    pUpdateMsgMem = SysMemCreate(pUpdateMsgBuf, NetUpdateMsgLen, NetUpdateMsgSize, &err);
    pUpdateDataMem = SysMemCreate(pUpdateDataBuf, NetUpdateDataLen, NetUpdateDataSize, &err);
    
 	Task_Msg_Create(&NetMsgQ,         &TaskMsgNetPoint[0], 		pNetMsgQBuf, 		AppNetMsgLen, 		MsgQSize);
 	Task_Msg_Create(&BusinessMsgQ,    &TaskMsgBusinessPoint[0],	pBusinessMsgQBuf,	AppBusinessMsgLen,	MsgQSize);
 	Task_Msg_Create(&DataManagerMsgQ, &TaskMsgDataPoint[0], 	pDataMsgQBuf,		AppDataMsgLen,		MsgQSize);	
 	Task_Msg_Create(&NetUpdateMsgQ,	&TaskMsgNetUpdatePoint[0],	pNetUpdateMsgQBuf,	AppNetUpdateMsgLen,	MsgQSize);
}

//网络发送波形数据与调试信息
//开机发送系统参数, 外部函数
extern void Send_SysParm_Info(char *sbuf);
extern uint16 g_bPayFlag;
static char debug_buf[120+RMCP_HEAD_LEN+1] = {0};

//网络发送内存分配失败测试
//#define WAVE_ALLOCATE_TEST
#ifdef WAVE_ALLOCATE_TEST
static char wcnt = 0;
#endif

BOOL Net_Send_Wave(void *buf)
{
	static uint8 cnt = 0;
	static char *pbuf;
	static BOOL reset_flag = TRUE;
	static char *pSysParmBuf = NULL;
	uint8 err;
	TaskMsg msg = {0};
	uint32 device_value = Get_Device_Status();
	uint32 len = 0;
	
	device_value &= 0x7FFFFFFF;
	if(g_bPayFlag == 1)
	{
		device_value |= 0x80000000;
		g_bPayFlag = 0;
	}

	if((buf == NULL) || (debug_buf == NULL))return FALSE;
	if(pWaveMem == NULL) return FALSE;

	//网络升级时不发送波形
	if(!Get_NetUpdate_Flag())
	{
		if(cnt == 0) 
		{		
			pbuf = (char *)SysMemGet(pWaveMem, &err);
			if((err != SYS_ERR_NONE) || (pbuf == NULL)) 
			{
#ifdef WAVE_ALLOCATE_TEST
				debug(Debug_Warning, "\r\nWave mem allocated failed! %d\r\n", err);
				wcnt = 0;
#endif
				return FALSE;
			}

			memset(pbuf, 0, WaveSize);
			
			//Build_NetData_Pack(pbuf, 321, Net_Data_Wet);
			Build_Rcmp_HeadPack(CMD_WAVE_DATA, (unsigned char *)pbuf, 320);
		}
		
		memcpy(pbuf + RMCP_HEAD_LEN + cnt*20, buf, 16);
		memcpy(pbuf + RMCP_HEAD_LEN + cnt*20 + 16, (u8 *)&device_value, 4);
	}

	//第一包参数数据
	if(reset_flag) 
	{
		//该指针开机只使用一次
		pSysParmBuf = (char *)SysMemGet(pWaveMem, &err);
		if(pSysParmBuf != NULL) 
		{
			Send_SysParm_Info(pSysParmBuf);
		}
		reset_flag = FALSE;			
	} 

	cnt++;
	if(cnt >= 16) {
		cnt = 0;
		
		memset(debug_buf, 0, 120+RMCP_HEAD_LEN+1);
		len = debug_get_info(debug_buf);

		//如果有调试信息, 发送调试信息
		if(len > 0) 
		{
			msg.msg_id = Msg_NetDebug;
			msg.msg_ctx = (u32)debug_buf;
			msg.msg_len = len;
			err = Task_QPost(&NetMsgQ, &msg);
		}

		//网络升级时不发送波形
		if(!Get_NetUpdate_Flag())
		{
			//波形数据包
			msg.msg_id = Msg_NetSend;
			msg.msg_ctx = (u32)pbuf;
			msg.msg_len = 320 + RMCP_HEAD_LEN;
	 		err = Task_QPost(&NetMsgQ, &msg);
	 		if(err != SYS_ERR_NONE) 
			{
			    Wave_Mem_Free(pbuf);
	 			debug(Debug_Warning, "NetSendPost failed!\r\n");
	 			return FALSE;
	 		}
	 	}
	}
	return TRUE;
}

uint8 Wave_Mem_Free(void *pblk)
{
#ifdef WAVE_ALLOCATE_TEST
	wcnt++;
	debug(Debug_Warning, "free!");
	if(wcnt >= 20)
	{
		wcnt = 0;
		debug(Debug_Warning, "\r\n");
	}
#endif
	return SysMemPut(pWaveMem, pblk);
}

//=====================================================
//升级消息缓冲池, 上发中心
char *NetUpdateMsg_Malloc(void)
{
    char *pbuf = NULL;
    uint8 err;
    
    pbuf = (char *)SysMemGet(pUpdateMsgMem, &err);
    if((err != SYS_ERR_NONE) || (pbuf == NULL)) 
    {
        debug(Debug_Warning, "\r\nNetUpdate msg-mem allocated failed! %d\r\n", err);
        return NULL;
    }

	memset(pbuf, 0, NetUpdateMsgSize);
    
    return pbuf;
}

uint8 NetUpdateMsg_Mem_Free(void *pblk)
{
	return SysMemPut(pUpdateMsgMem, pblk);
}

//=================================
//升级文件分段缓冲池，定长256
char *NetUpdateData_Malloc(void)
{
    char *pbuf = NULL;
    uint8 err;
    
    pbuf = (char *)SysMemGet(pUpdateDataMem, &err);
    if((err != SYS_ERR_NONE) || (pbuf == NULL)) 
    {
        debug(Debug_Warning, "\r\nNetUpdate data-mem allocated failed! %d\r\n", err);
        return NULL;
    }

	memset(pbuf, 0, NetUpdateDataSize);
    
    return pbuf;
}

uint8 NetUpdateData_Mem_Free(void *pblk)
{
	return SysMemPut(pUpdateDataMem, pblk);
}

