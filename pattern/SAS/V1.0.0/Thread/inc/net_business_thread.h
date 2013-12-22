#ifndef NET_BUSINESS_THREAD
#define NET_BUSINESS_THREAD

//网络传输数据类型
enum {
	Net_Data_Wet = 1,		//波形数据
	Net_Data_Debug,		//调试命令
	Net_Data_Parm,			//系统参数数据包
	Net_Update_Cmd,		//升级请求命令
	Net_Update_Data,		//升级数据包
	Net_Update_Crc,		//crc
	
	Net_App_Restart = 0x75,		//重启系统命令
};

void Build_NetData_Pack(char *buf, uint16 len, uint8 type);
BOOL Get_NetUpdate_Flag(void);

//升级文件校验附加秘钥
static u8 NET_Bin_KEY[] = "EMOS_V001_001_001_001";

//擦除固件升级flash区域
u8 Erase_AppFlash(void);

/****************************************************************************
* 名	称：_asm void SystemReset(void)
* 功	能：重新启动
* 入口参数：无
* 出口参数：无
* 说	明：
****************************************************************************/
void SystemReset(void);



#endif

