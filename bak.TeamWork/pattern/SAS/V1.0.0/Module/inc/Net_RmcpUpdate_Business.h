#ifndef NET_RMCPUPDATE_BUSINESS_H
#define NET_RMCPUPDATE_BUSINESS_H

#ifdef __cplusplus
extern "C" {
#endif

void Net_Rmcp_Update(unsigned short cmd, unsigned char *buf, int len);
void NetUpdate_Overtime_Business(void);
void NetUpdate_Abnormal_Business(char type);
void NetUpdate_Send_CancelMsg(void);


#ifdef __cplusplus
}
#endif

#endif
