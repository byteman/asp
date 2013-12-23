#include "RMCP.h"
#include <string.h>
#include <stdio.h>
#include "softdog.h"
//#include "RMCPAdp.h"
//#include "RMCP_ZCS.h"
#include "debug_info.h"
#include "net_business_thread.h"
#include "net_rmcpupdate_business.h"
#include "app_msg.h"
#include "task_def.h"
#include "crc.h"

#ifdef RMCP_SUPPORT_ENCRYPT
#define NET_PROTOCOL_MIN_LEN (CIPHER_DIGEST_SIZE+11)
#define NET_HEAD_INDEX       CIPHER_DIGEST_SIZE
#else
#define NET_PROTOCOL_MIN_LEN 11
#define NET_HEAD_INDEX       0
#endif

#define MAX_PACKET_BUF_SIZE 1024
#define MAX_CTX_BUF_SIZE    1024
////////////////////////////////////////////////////


static volatile BOOL bDebugUpdateFlag = FALSE;

/**
 * Convert an u32_t from host- to network byte order.
 *
 * @param n u32_t in host byte order
 * @return n in network byte order
 */
unsigned int
rmcp_htonl(unsigned int n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

/**
 * Convert an u32_t from network- to host byte order.
 *
 * @param n u32_t in network byte order
 * @return n in host byte order
 */
unsigned int
rmcp_ntohl(unsigned int n)
{
  return rmcp_htonl(n);
}

/**
 * Convert an u16_t from host- to network byte order.
 *
 * @param n u16_t in host byte order
 * @return n in network byte order
 */
unsigned short
rmcp_htons(unsigned short n)
{
  return ((n & 0xff) << 8) | ((n & 0xff00) >> 8);
}

/**
 * Convert an u16_t from network- to host byte order.
 *
 * @param n u16_t in network byte order
 * @return n in host byte order
 */
unsigned short
rmcp_ntohs(unsigned short n)
{
  return rmcp_htons(n);
}

int sendRmcpMessage(CmdRMCP msgType, unsigned char* context, int context_size)
{ 
    char *pbuf = NULL;
    uint8 err;
    TaskMsg msg = {0};
	
    if(context_size > UPDATE_MSG_LEN)
    {
        debug(Debug_Error, "sendRmcpMessage size too large\r\n");
        return 0;
    }
    
    pbuf = NetUpdateMsg_Malloc();
    if(pbuf == NULL) return 0;

	//rmcp报文头打包
    Build_Rcmp_HeadPack(msgType, (unsigned char *)pbuf, context_size);
    if(context != NULL)
    {
		memcpy(pbuf + RMCP_HEAD_LEN, context, context_size);
	}

    //波形数据包
    msg.msg_id = Msg_NetUpdate;
    msg.msg_ctx = (u32)pbuf;
    msg.msg_len = context_size + RMCP_HEAD_LEN;
    err = Task_QPost(&NetMsgQ, &msg);
    if(err != SYS_ERR_NONE) 
    {
        NetUpdateMsg_Mem_Free(pbuf);
        debug(Debug_Warning, "NetSendPost failed!\r\n");
        return 0;
    }
    
    return 1;
}

void convertRCMPHeader(RMCPHeader* head)
{
    head->_totalLength = rmcp_ntohl(head->_totalLength);
    head->_msgType = rmcp_ntohs(head->_msgType);
}

int Build_Rcmp_HeadPack(CmdRMCP msgType, unsigned char* buf, int context_size)
{
	RMCPHeader header;

	header._msgType = msgType;
    header._totalLength = sizeof(RMCPHeader) + context_size;

    header._protoName[0] = 'R';
    header._protoName[1] = 'M';
    header._protoName[2] = 'C';
    header._protoName[3] = 'P';
    header._protoVer = RMCP_VER;
    convertRCMPHeader(&header);
	
	memcpy(buf,&header,sizeof(RMCPHeader));

	return sizeof(RMCPHeader);
}

void Net_Update_Complete(void)
{
	Set_NetUpdate_Flag(FALSE);
	Set_SoftDog_Changer(FALSE);
	bDebugUpdateFlag = FALSE;
	//调试打印等级设为业务等级
	Print_Debug_Init();
}

//网络控制命令结果打印回调匹配函数
static int Net_RmcpCtrlCmd_Send(const void *pbuf, u32 len)
{
	debug(Debug_None, pbuf);

	return len;
}

//static unsigned char UpdateDataBuf[300] = {0};

void parseRMCPMessage(RMCPHeader* pHeader, unsigned char* context ,int len)
{
	TaskMsg UpdateMsg = {0};
	char *pbuf = NULL;
	
    if(pHeader == NULL) return;
    if(context == NULL) return;

    switch(pHeader->_msgType)
	{
        case CMD_UPDATE_SERVER_REQUEST:
        case CMD_UPDATE_CANCEL:
        case CMD_UPDATE_DATA_VERIFY:
        	UpdateMsg.msg_id = pHeader->_msgType;
        	UpdateMsg.msg_ctx = (uint32)context;
        	UpdateMsg.msg_len = len;
      		Task_QPost(&NetUpdateMsgQ, &UpdateMsg);
      		
        	if(pHeader->_msgType == CMD_UPDATE_CANCEL)
        	{
        		Net_Update_Complete();
        	}
        	break;

       	case CMD_UPDATE_DATA_SEG:
       		//数据段长度异常，拒绝升级
       		if((len > NetUpdateDataSize) || (len < 5))
       		{
       			NetUpdate_Send_CancelMsg();
       			Net_Update_Complete();
       			break;
       		}
       		
       		pbuf = NetUpdateData_Malloc();
       		//分配内存失败,申请重发
       		if(pbuf == NULL)
       		{
       			NetUpdate_Abnormal_Business(0);
       			break;
       		}
			memcpy(pbuf, context, len);

			UpdateMsg.msg_id = pHeader->_msgType;
        	UpdateMsg.msg_ctx = (uint32)pbuf;
        	UpdateMsg.msg_len = len;
        	if(Task_QPost(&NetUpdateMsgQ, &UpdateMsg) != SYS_ERR_NONE)
        	{
        		NetUpdateData_Mem_Free(pbuf);
        		NetUpdate_Abnormal_Business(0);
        	}
       		break;
            
        case CMD_APP_UPDATE:
        	//停止波形发送,关闭看门狗监测
			Set_NetUpdate_Flag(TRUE);
			Set_SoftDog_Changer(TRUE);
			//调试信息打印等级设为0
			Set_Debug_Level(Debug_None, FALSE);
			
			bDebugUpdateFlag = TRUE;
        	break;
        	
        case CMD_SYS_CTRL:
        	if(!bDebugUpdateFlag)
        	{
				Net_RmcpCtrlCmd_Parse((char*)context, Net_RmcpCtrlCmd_Send);
			}
        	break;
        	
        default:
            break;
	}
}

