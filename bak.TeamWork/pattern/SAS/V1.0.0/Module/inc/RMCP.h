#ifndef RMCP_INCLUDE
#define RMCP_INCLUDE

#ifdef __cplusplus
extern "C" {
#endif



#if defined(__GNUC__) || defined(WIN32)
#define PACK __packed
#else
#define PACK
#endif



#define CIPHER_DIGEST_SIZE 16

#pragma pack(push) //保存对齐状态
#pragma  pack(1)  

typedef struct{
#ifdef RMCP_SUPPORT_ENCRYPT
    unsigned char   _cipherDigest[CIPHER_DIGEST_SIZE];
#endif
	char 			_protoName[4];
	unsigned char 	_protoVer;
	unsigned int    _totalLength;
	unsigned short  _msgType;
 }RMCPHeader;

#pragma  pack(pop)

#define RMCP_HEAD_LEN (sizeof(RMCPHeader))
#define RMCP_VER 0x20   //V2.0

#define PROTO_HEADER_LEN 42
#define PROTO_HEADER1 'R'
#define PROTO_HEADER2 'M'
#define PROTO_HEADER3 'C'
#define PROTO_HEADER4 'P'

typedef enum{
    CMD_REQ_AUTH=1,
    CMD_RESP_AUTH_OK,
    CMD_RESP_AUTH_FAILD,
	CMD_HEART_REQ=0x11,
	CMD_HEART_RESP,
	CMD_BUSINESS_REQ=0x21,
    CMD_BUSINESS_RESP,

    //rmcp升级
	CMD_UPDATE_SERVER_REQUEST = 0x51,
	CMD_UPDATE_3102_REFUSE,
	CMD_UPDATE_3102_ACCEPT,
	CMD_UPDATE_DATA_SEG,
	CMD_UPDATE_3102_RSEG,
	CMD_UPDATE_CANCEL,
	CMD_UPDATE_CANCEL_ACK,
	CMD_UPDATE_3102_NONSUPPORT,
	CMD_UPDATE_DATA_VERIFY,
	CMD_UPDATE_3102_RESULT,
	CMD_UPDATE_DATA_SEG_ACK,
	CMD_UPDATE_DATA_CHECK_ERR,
	CMD_UPDATE_DATA_NUM_ERR,

	CMD_WAVE_DATA = 0xf5,	//波形数据
	CMD_DEBUG_DATA, 	//调试信息
	CMD_PARAM_DATA, 	//3102参数
	CMD_APP_UPDATE, 	//调试软件升级命令
	CMD_SYS_CTRL		//系统控制命令
}CmdRMCP;

enum {
	//系统控制	
	CMD_CTRL_RST = 0x10,	
	//打印系统重启次数
	CMD_CTRL_RSTCNT,	
	//设置调试信息打印等级
	CMD_CTRL_SET_DLEVLE, 
	//打印系统运行时间
	CMD_CTRL_SYS_RUNTIME,
	//秤台置零
	CMD_CTRL_SCALER_SETZERO,
	//模拟车辆
	CMD_CTRL_SIMULATE_CAR,	
	//打印程序版本
	CMD_CTRL_APP_VER,
	//打印外设设备状态
	CMD_CTRL_DEVICE_STATUS,
	//模拟倒车
	CMD_CTRL_SIMULATE_BACKCAR,
	//跟车设置
	CMD_CTRL_FOLLOWCAR_MOD,
	//整秤k系数设置
	CMD_CTRL_SET_SCALERK,
	//上秤端2个传感器k系数设置
	CMD_CTRL_SET_AD3K,
	//对话框退出
	CMD_CTRL_EXIT,
};

//网络升级收尾工作
void Net_Update_Complete(void);

//RMCP报文头打包
int Build_Rcmp_HeadPack(CmdRMCP msgType, unsigned char* buf, int context_size);

void parseRMCPMessage(RMCPHeader* pHeader, unsigned char* context ,int len);

void convertRCMPHeader(RMCPHeader* head);

int sendRmcpMessage(CmdRMCP msgType, unsigned char* context, int context_size);

unsigned int rmcp_htonl(unsigned int n);
unsigned short rmcp_htons(unsigned short n);

#ifdef __cplusplus
}
#endif



#endif
