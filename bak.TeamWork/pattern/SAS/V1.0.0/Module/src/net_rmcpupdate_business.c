#include "includes.h"
#include "rmcp.h"
#include "net_business_thread.h"
#include "net_rmcpupdate_business.h"
#include "system_init.h"
#include "crc.h"
#include "eth_init.h"
#include "debug_info.h"

enum {
	FSM_UPDATE_INIT = 0,
	FSM_UPDATE_PROCESS,
	FSM_UPDATE_ABNORMAL,
	FSM_UPDATE_ENDING,
    FSM_UPDATE_SUCCESS,
	FSM_UPDATE_COMPLETE,
	FSM_UPDATE_OVERTIME,
	FSM_UPDATE_CANCEL
};

typedef struct {
	unsigned char Module_ID;
	unsigned int FileLen;
	unsigned short FileIDLen;
	unsigned short CheckFileIDLen;
}RMCPUpdate;


struct _NetUpdateFun 
{
	unsigned short state;
	unsigned int DataID;
	BOOL bComplete;
	BOOL bCheck;
	unsigned short FileCrc;
	RMCPUpdate UpdateHead;
};

static struct _NetUpdateFun NetUpdateFun = {{0}};
static unsigned char UpdateHeadBuf[128] = {0};
static int UpdateHeadBufLen = 0;
static unsigned char sendbuf[128] = {0};

#define UPDATE_OVERTIME_TICK1 3
#define UPDATE_OVERTIME_TICK2 (2*UPDATE_OVERTIME_TICK1+1)

static volatile char UpdateOvertimeTick = 0;
static volatile BOOL bUpdateOvertimeFlag = FALSE;
static volatile BOOL bStartCheckUpdate = FALSE;

static void Update_Refuse(char *str)
{
	int slen = 0;
	
	//发送拒绝升级rmcp报文
	sendbuf[0] = strlen(str) + 1;
	strcpy((char *)&sendbuf[1], str);
	slen = sendbuf[0] + 1;
	sendRmcpMessage(CMD_UPDATE_3102_REFUSE, sendbuf, slen);
}

void NetUpdate_Abnormal_Business(char type)
{
	int slen = 0;
	unsigned int tmp = 0;
	
	//发送纠正升级文件传输rmcp报文
	memcpy(sendbuf, UpdateHeadBuf, UpdateHeadBufLen);
	sendbuf[UpdateHeadBufLen] = type;
	slen = UpdateHeadBufLen + 1;
	if(type == 0)
	{
		//期望接收数据分段号
		tmp = rmcp_htonl(NetUpdateFun.DataID + 1);
		memcpy(&sendbuf[slen], &tmp, 4);
		slen += 4;
	}
	sendRmcpMessage(CMD_UPDATE_3102_RSEG, sendbuf, slen);
}

void NetUpdate_Send_CancelMsg(void)
{
	unsigned char tmp = 0;

    sendRmcpMessage(CMD_UPDATE_CANCEL, &tmp, 1);
}

static void Update_Result(char result, char *str)
{
	int slen = 0;
	unsigned int tmp = 0;
	
	//发送升级结果报文
	memcpy(sendbuf, UpdateHeadBuf, UpdateHeadBufLen);
	sendbuf[UpdateHeadBufLen] = result;
	slen = UpdateHeadBufLen + 1;
	if(result == 1)
	{
		//失败原因
		tmp = strlen(str) + 1;
		sendbuf[slen] = tmp;
		slen += 1;
		strcpy((char *)&sendbuf[slen], str);
		slen += tmp;
	}
	sendRmcpMessage(CMD_UPDATE_3102_RESULT, sendbuf, slen);
}

void NetUpdate_Overtime_Business(void)
{
	static BOOL flag = FALSE;

	if(!bStartCheckUpdate) return;
	if(bUpdateOvertimeFlag) return;

	if(UpdateOvertimeTick == 0)
	{
		flag = FALSE;
	}

	UpdateOvertimeTick++;
	if(flag)
	{
		if(UpdateOvertimeTick > UPDATE_OVERTIME_TICK2)
		{
			UpdateOvertimeTick = 0;
			//超时，取消升级
			bUpdateOvertimeFlag = TRUE;
			bStartCheckUpdate = FALSE;
			debug(Debug_None, "Update timeover!\r\n");
		}
	}
	else
	{
		if(UpdateOvertimeTick > UPDATE_OVERTIME_TICK1)
		{
			if(NetUpdateFun.bComplete)
			{
				if(NetUpdateFun.bCheck)
				{
					//升级文件接收完成，需要接收升级文件校验包
					NetUpdate_Abnormal_Business(1);
				}
			}
			else
			{
				//提醒终端发送下一包数据
				NetUpdate_Abnormal_Business(0);
			}
			debug(Debug_None, "Update timeover(%ds) first!\r\n", UPDATE_OVERTIME_TICK1);
			flag = TRUE;
			UpdateOvertimeTick = 1;
		}
	}
}

//#define CHECK_ACK_PROTOCOL

#ifdef CHECK_ACK_PROTOCOL
static unsigned char Check_Sum(unsigned char *buf, int len)
{
	unsigned char rst = 0;
	int i = 0;

	if((buf == NULL) || (len == 0)) return 0;

	for(i = 0; i < len; i++)
	{
		rst += buf[i];
	}

	return rst;
}
#endif

void Net_Rmcp_Update(unsigned short cmd, unsigned char *buf, int len)
{
	static char *prompt0 = "擦除flash失败,拒绝升级\r\n";
	static char *prompt1 = "升级头文件超过3102头文件缓冲区长度\r\n";
	static char *prompt2 = "写更新标志失败\r\n";
	static char *prompt3 = "写flash失败\r\n";
	static char *prompt4 = "CRC校验失败\r\n";
	static BOOL fail_flag = FALSE;
	static int addr = 0, rfilelen = 0;
	static unsigned short FileCrc = 0;
	unsigned char *pBuf = buf;
	unsigned short tmp = 0;
	unsigned int TmpDataID = 0, tlen;
#ifdef CHECK_ACK_PROTOCOL   
	unsigned char crc_check = 0, f_check = 0;
#endif
	if(buf == NULL) return;

	NetUpdateFun.state = cmd;

	//超时处理
	if(bUpdateOvertimeFlag)
	{
		if(cmd != CMD_UPDATE_SERVER_REQUEST)
		{
			//NetUpdateFun清零
			memset(&NetUpdateFun, 0, sizeof(struct _NetUpdateFun));
			bUpdateOvertimeFlag = FALSE;
			bStartCheckUpdate = FALSE;
			NetUpdate_Send_CancelMsg();
			return;
		}
	}

	//超时计时器清零
	UpdateOvertimeTick = 0;

	while(1)
	{
	    switch(NetUpdateFun.state)
	    {
	        case CMD_UPDATE_SERVER_REQUEST:
	            //NetUpdateFun清零
	            memset(&NetUpdateFun, 0, sizeof(struct _NetUpdateFun));
	            CRC16_ResetDR();
	            addr = 0;
	            fail_flag = FALSE;
	            rfilelen = 0;
	            FileCrc = 0;
	            
	            //保存升级头
	            if(len < (128+4))
	            {
	                UpdateHeadBuf[0] = *buf;
	                memcpy(&UpdateHeadBuf[1], (buf + 1 + 4), len - 5);
	                UpdateHeadBufLen = len - 4;
	            }
	            else
	            {
	                debug(Debug_Error, prompt1);
	                SysTimeDly(2);
	                Update_Refuse(prompt1);
	                break;
	            }
	            
	            NetUpdateFun.UpdateHead.Module_ID = *pBuf++;
	            
	            memcpy(&tlen, pBuf, sizeof(int));
	            NetUpdateFun.UpdateHead.FileLen = rmcp_htonl(tlen);
	            rfilelen = NetUpdateFun.UpdateHead.FileLen;
	            pBuf += sizeof(int);
	            
	            memcpy(&tmp, pBuf, sizeof(short));
	            NetUpdateFun.UpdateHead.FileIDLen = rmcp_htons(tmp);
	            pBuf += sizeof(short);
	            pBuf += NetUpdateFun.UpdateHead.FileIDLen;
	            
	            memcpy(&tmp, pBuf, sizeof(short));
	            NetUpdateFun.UpdateHead.CheckFileIDLen = rmcp_htons(tmp);
	            pBuf += sizeof(short);
	            if(NetUpdateFun.UpdateHead.CheckFileIDLen == 0)
	            {
	                NetUpdateFun.bCheck = FALSE;	//不需要校验升级文件
	            }
	            else
	            {
	                NetUpdateFun.bCheck = TRUE;
	            }

	            debug(Debug_None, "擦除flash,准备网络升级!\r\n");
	            debug(Debug_None, "Recv FileLen=%d\r\n", NetUpdateFun.UpdateHead.FileLen); 
	            //擦除固件flash区域
	            if(Erase_AppFlash() != 0)
	            {
	            	SysTimeDly(100);
	                debug(Debug_None, prompt0);
	                SysTimeDly(2);			
	                Update_Refuse(prompt0);
	                break;
	            }
	            SysTimeDly(100);
				debug(Debug_None, "擦除flash成功,开始升级,请等待!\r\n");

				//开启超时监测
				bStartCheckUpdate = TRUE;
	            bUpdateOvertimeFlag = FALSE;
	            
	            //回应接受升级
	            sendRmcpMessage(CMD_UPDATE_3102_ACCEPT, UpdateHeadBuf, UpdateHeadBufLen);
	            break;
	            
	        case CMD_UPDATE_DATA_SEG:
#ifdef CHECK_ACK_PROTOCOL        	
	        	//数据包校验
	       		crc_check = pBuf[len-1];
	       		f_check = Check_Sum(pBuf, len-1);
	       		if(crc_check != f_check)
	       		{
	       			debug(Debug_None, "check crc failed %04x-%04x-%d\r\n", crc_check, f_check, len);
	       			sendRmcpMessage(CMD_UPDATE_DATA_CHECK_ERR, NULL, 0);
	       			break;
	       		}
	       		len -= 1;
#endif
	       		//数据已传输完成
	            if(NetUpdateFun.bComplete)
	            {
	                if(NetUpdateFun.bCheck)
	                {
	                    //升级文件接收完成，需要接收升级文件校验包
	                    NetUpdate_Abnormal_Business(1);
	                }
	                NetUpdateData_Mem_Free(buf);
	                break;
	            }         
	                
	            //跳过module_id
	            *pBuf++;
	            len -= 1;
	            //保存分段号
	            memcpy(&TmpDataID, pBuf, 4);
	            TmpDataID = rmcp_htonl(TmpDataID);
	            len -= 4;
	      		
	            //丢包处理
	            if(TmpDataID > 0)
	            {
		            if(TmpDataID != (NetUpdateFun.DataID + 1))
		            {
		            #ifndef CHECK_ACK_PROTOCOL  
		                NetUpdate_Abnormal_Business(0);
		                debug(Debug_None, "Segs abnormal!rid:%d, dataid:%d\r\n", TmpDataID, NetUpdateFun.DataID);
					    NetUpdateData_Mem_Free(buf);
                    #else
						debug(Debug_None, "check num failed-%d\r\n", TmpDataID);
	       				sendRmcpMessage(CMD_UPDATE_DATA_NUM_ERR, NULL, 0);
	       			#endif
		                break;
		            }
		        }
	            NetUpdateFun.DataID = TmpDataID;
	            pBuf += 4;
	            if(rfilelen >= len)
	            {
	            	rfilelen -= len;
	            }
	            else
	            {
	            	len = rfilelen;
	            	rfilelen = 0;
	            }

	            //写入flash
	            if(Flash_APPBak_Store(addr, pBuf, len) == 0) fail_flag = TRUE;
	            addr += len;	

	            //计算升级文件crc校验值,和bootloader匹配
	  	        FileCrc = CRC16(0, pBuf, len);
	  	        
				//debug(Debug_None, "%d Segs-%04x-%02x\r\n", TmpDataID, FileCrc, crc_check);

	            if(rfilelen == 0)
	            {
	                NetUpdateFun.bComplete = TRUE;
	                if(!NetUpdateFun.bCheck)
	                {
	                    //不需要校验
	                    NetUpdateFun.state = FSM_UPDATE_COMPLETE;
	                }
	            }    
#ifdef CHECK_ACK_PROTOCOL  
				sendRmcpMessage(CMD_UPDATE_DATA_SEG_ACK,(unsigned char *)&NetUpdateFun.DataID,sizeof(int));
#endif
                //释放内存
	            NetUpdateData_Mem_Free(buf);
	            break;
	            
	        case CMD_UPDATE_CANCEL:
	            //NetUpdateFun清零
	            bStartCheckUpdate = FALSE;
	            tmp = 0;
	            memset(&NetUpdateFun, 0, sizeof(struct _NetUpdateFun));
	            sendRmcpMessage(CMD_UPDATE_CANCEL_ACK, (unsigned char *)&tmp, 1);
	            debug(Debug_None, "用户取消升级!\r\n");
	            break;
	            
	        case CMD_UPDATE_DATA_VERIFY:
	        	//跳过module_id
	            pBuf++;
	            memcpy(&tmp, pBuf, 2);
	        	FileCrc = CRC16(0,NET_Bin_KEY,sizeof(NET_Bin_KEY));	
	        	
	            tmp = rmcp_htons(tmp);
	            if(tmp != FileCrc)
	            {
	                debug(Debug_None, prompt4);
	                debug(Debug_None, "RecCrc:%x,FileCrc:%x\r\n", tmp, FileCrc);
	                SysTimeDly(2);
	                Update_Result(1, prompt4);
	                break;
	            }
	            debug(Debug_None, "RecCrc:%x, FileCrc:%x\r\n", tmp, FileCrc);
	            NetUpdateFun.state = FSM_UPDATE_COMPLETE;
	            break;

	        case FSM_UPDATE_COMPLETE:
	        	bStartCheckUpdate = FALSE;
	            //更新升级标志
	            NetUpdateFun.state = FSM_UPDATE_CANCEL;
	            if(!fail_flag)
	            {
	                if(Flash_APPInfo_Store(FileCrc, NetUpdateFun.UpdateHead.FileLen, NETBOOT) == 0)
	                {
	                    debug(Debug_None, prompt2);
	                    SysTimeDly(2);
	                    break;
	                }
	                if(Flash_UPFlag() == 0)
	                {
	                    debug(Debug_None, prompt2);
	                    SysTimeDly(2);
	                    break;
	                }
	                debug(Debug_None, "网络升级完成,如果需要运行新程序,请重启!\r\n");
	                SysTimeDly(2);
	                Update_Result(0, NULL);
	                NetUpdateFun.state = FSM_UPDATE_SUCCESS;
	                Net_Update_Complete();
	            }
	            else
	            {
	                debug(Debug_None, prompt3);
	                SysTimeDly(2);
	                Update_Result(1, prompt3);
	            }
	            break;

	        default: break;
	    }

	    if(NetUpdateFun.state != FSM_UPDATE_COMPLETE) break;
	}//end while(1);
}


