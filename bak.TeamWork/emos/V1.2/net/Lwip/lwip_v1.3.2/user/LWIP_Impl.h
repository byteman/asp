#ifndef _LWIP_Impl_H_
#define _LWIP_Impl_H_
#ifdef __cplusplus		   		//定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "lwip/sockets.h"
#include "includes.h"
	
    /*IP地址*/
    typedef struct{
        char addr1;
        char addr2;
        char addr3;
        char addr4;
    } sIP_ADDR;

    /*本机MAC地址*/
    typedef struct{
        char addr1;
        char addr2;
        char addr3;
        char addr4;
        char addr5;
        char addr6;
    } sMAC_ADDR;
    
    /*本机IP地址*/
    typedef struct{
        sIP_ADDR ipaddr;                                    /*本机IP地址*/
        sIP_ADDR SubnetMask;                                /*本机掩码地址*/
        sIP_ADDR GateWay;                                   /*本机网关地址*/
    } Glocal_IP_ADDR;

    
    /*服务器IP地址，和端口号*/
    typedef struct{
        sIP_ADDR ipaddr;                                    /*服务器IP地址*/
        int port;                                           /*服务器端口*/
    } sServerADDR;



////////////////////////////////////////////////////////////
    
/*配置互斥量和LWIP线程优先级*/
typedef struct{
    char nMutexPrio;                                    /*互斥量优先级,该优先级在网络优先级中应该最高*/
    char nLwipBPrio;                                    /*LWIP业务处理线程优先级,该优先级建议比用户优先级低一个级别*/
    char nLwipRPrio;                                    /*LWIP接收线程优先级，该优先级建议比LWIP业务线程优先级低一个级别*/
}sTASK_PARAM;



///@brief
///函数接口定义
/****************************************************************************
* 功	能：网络设备初始化
* 入口参数：Glocal_IP_ADDR:本机IP地址和端口
          ：sMAC_ADDR本机MAC地址
          ：sTASK_PARAM LWIP利用到的优先级 互斥量，两个任务的优先级
* 出口参数：成功时返回1，失败时返回0。 
* 其它说明：
****************************************************************************/
u8 LwIP_Init(Glocal_IP_ADDR local_IP,sMAC_ADDR macAddr,sTASK_PARAM taskparam);


/****************************************************************************
* 功	能：网络IP地址的修改
* 入口参数：Glocal_IP_ADDR:本机IP地址和端口
* 出口参数：成功时返回0，失败时返回-1。 
* 其它说明：
****************************************************************************/
int LwIP_Set(Glocal_IP_ADDR local_IP);


/****************************************************************************
* 功	能：网络硬件初始化
* 入口参数：domain:说明我们网络程序所在的主机采用的通讯协族(AF_UNIX和AF_INET等)。
          ：type采用的通讯协议(SOCK_STREAM,SOCK_DGRAM等) 
          ： SOCK_STREAM表明我们用的是TCP 协议,这样会提供按顺序的,
          ：可靠,双向,面向连接的比特流；SOCK_DGRAM 表明我们用的是UDP协议,
          ：这样只会提供定长的,不可靠,无连接的通信。
          ：protocol:由于我们指定了type,所以这个地方我们一般只要用0来代替就可以了。
* 出口参数：成功时返回文件描述符，失败时返回-1。 
* 其它说明：Int socket(int domain, int type, int protocol)
****************************************************************************/
#define socket(a,b,c)         lwip_socket(a,b,c)



/****************************************************************************
* 功能说明：连接服务器 
* 入口参数：sockfd:socket返回的文件描述符。
          ：serv_addr储存了服务器端的连接信息，其中sin_add是服务端的地址。
          ：addrlen serv_addr的长度。
* 出口参数：connect函数是客户端用来同服务端连接的.成功时返回0，失败时返回-1。
* 其它说明：int connect(int sockfd, struct sockaddr * serv_addr,int addrlen) 
****************************************************************************/
#define connect(a,b,c)        lwip_connect(a,b,c)



/****************************************************************************
* 功能说明：数据接收 
* 入口参数：sockfd:socket返回的文件描述符。
          ：buf 是存放接收数据的缓冲区。
          ：len是缓冲的长度。
          ：Flags也被置为0，设置为1表示超时10MS，100表示超时1S。
* 出口参数：返回实际上接收的字节数，当出现错误时，返回-1。
* 其它说明： int recv(int sockfd,void *buf,int len,unsigned int flags);
****************************************************************************/
#define recv(a,b,c,d)         lwip_recv(a,b,c,d)



/****************************************************************************
* 功能说明： 
* 入口参数：sockfd:socket返回的文件描述符。
          ：buf是个指向要发送数据的指针。
          ：len是以字节为单位的数据的长度。
          ：flags一般情况下置为0。
* 出口参数：函数返回实际上发送出的字节数，可能会少于您希望发送的数据，
*           在程序中应该将send()的返回值和欲发送的字节数进行比较。
* 其它说明：int send(int sockfd, const void *buf, int len, int flags) 
****************************************************************************/
#define send(a,b,c,d)         lwip_send(a,b,c,d)



/****************************************************************************
* 功能说明： 
* 入口参数：sockfd返回的文件描述符。
* 出口参数：操作成功时返回0，在出现错误时返回-1。
* 其它说明： 
****************************************************************************/
#define close(s)              lwip_close(s)


/****************************************************************************
* 功能说明： 
* 入口参数：sockfd:是由socket调用返回的文件描述符。
          ：my_addr:是一个指向sockaddr的指针。
          ：addrlen:是sockaddr结构的长度。
* 出口参数：bind将本地的端口同socket返回的文件描述符捆绑在一起，成功是返回0，失败时返回-1。
* 其它说明： int bind(int sockfd, struct sockaddr *my_addr, int addrlen)
****************************************************************************/
#define bind(a,b,c)           lwip_bind(a,b,c)


/****************************************************************************
* 功能说明： 
* 入口参数：sockfd:是由bind调用返回的文件描述符。
          ：backlog:设置请求排队的最大长度。
* 出口参数：listen函数将bind的文件描述符变为监听套接字，成功是返回0，失败时返回-1。
* 其它说明： int listen(int sockfd,int backlog) 
****************************************************************************/
#define listen(a,b)           lwip_listen(a,b)


/****************************************************************************
* 功能说明： 
* 入口参数：sockfd是被监听的socket描述符。
          ：addr通常是个指向sockaddr变量的指针，该变量用来存放提出连接请求服务的主机的信息。
          ：addrlen通常为一个指向值为sizeof(struct sockaddr)的整型指针变量。

* 出口参数：返回客户端新的socket描述符监听套接字，失败时返回-1。
* 其它说明： int accept(int sockfd, void *addr, int *addrlen)
****************************************************************************/
#define accept(a,b,c)         lwip_accept(a,b,c)


/****************************************************************************
* 功能说明： UDP 接收数据接口
* 入口参数：sockfd:socket返回的文件描述符。
          ：buf是个指向要发送数据的指针。
          ：len是以字节为单位的数据的长度。
          ：flags一般情况下置为0。
          ：from该变量保存源机的IP地址及端口号。
          ：fromlen常置为sizeof (struct sockaddr)。
* 出口参数：返回接收到的字节数或当出现错误时返回-1。
* 其它说明：Int recvfrom(int sockfd, void *buf, int len, int flags,   struct sockaddr *from, socklen_t *fromlen)
****************************************************************************/
#define recvfrom(a,b,c,d,e,f) lwip_recvfrom(a,b,c,d,e,f)


/****************************************************************************
* 功能说明： UDP发送数据接口
* 入口参数：sockfd:socket返回的文件描述符。
          ：buf是个指向要发送数据的指针。
          ：len是以字节为单位的数据的长度。
          ：flags一般情况下置为0。
          ：to表示目地机的IP地址和端口号信息。
          ：tolen常常被赋值为sizeof (struct sockaddr)。
* 出口参数：返回实际发送的数据字节长度或在出现发送错误时返回-1。
* 其它说明：int sendto(int sockfd, const void *buf,int len,unsigned int flags,const struct sockaddr *to, int tolen)
****************************************************************************/
#define sendto(a,b,c,d,e,f)   lwip_sendto(a,b,c,d,e,f)


/****************************************************************************
* 功能说明： 
* 入口参数：maxfdp1的值是需要检查的号码最高的文档描述符加1。
          ：readfds被select()监控的读描述符集合。
          ：writefds被select()监控的写文档描述符集合
          ：exceptfds被select()监控的异常处理的文档描述符集合
          ：Timeout参数是个指向struct timeval类型的指针，能够使select()
          ：在等待timeout长时间后没有文档描述符准备好即返回。
* 出口参数：超时返回0，失败时返回-1，其他返回大于0。
* 其它说明：int select(int maxfdp1,fd_set *readfds,fd_set *writefds，fd_set *exceptfds,struct timeval *timeout); 
****************************************************************************/
#define select(a,b,c,d,e)     lwip_select(a,b,c,d,e)

/****************************************************************************
* 功能说明：网线连接状态判断 
* 入口参数：无。
* 出口参数：网线插入返回0，其他返回非0值。
* 其它说明：该函数只能够在网络初始化成功之后调用 
****************************************************************************/
int  LwIP_GetStatus(void);

#ifdef __cplusplus		   		//定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
#include "LwipJob.h"
#include "includes.h"
#include "LWIP_Impl.h"
#include "string.h"
#include "usart1_init.h"
#include "usart_cfg.h"
#include "printf_init.h"

typedef enum 
{
	NetInit = 0,
	NetConecting,
	NetConected,
	NetClose
}ENETSTATAS;

#define  NETBUFLEN      0x200
static ENETSTATAS NetSta = NetInit;
static u8   gcNetRecvBuf[NETBUFLEN]="";

//最大支持客户端数量
#define CLIENTNUM 10

//客户机模型
void lwipClient(void)
{
	struct sockaddr_in sServer;
	int iSSocket = -1;
	int ret;
	char cbuf[20];
	sServerADDR ServerAddr={192,168,50,110,10000};      //服务器IP地址和端口号

	printf("Lwip 客户端测试开始\r\n");
	while(1)
	{   
		switch(NetSta) 
		{
			case NetInit:
			{
				iSSocket = socket(AF_INET,SOCK_STREAM,IPPROTO_IP);
				if(iSSocket >= 0) 
				{
					NetSta = NetConecting;
					printf("socket 建立成功\r\n");
					SysTimeDly(50);
				} 
				else 
				{
					printf("socket 建立失败\r\n");
					SysTimeDly(400);
				}
				break;
			}
						
			case NetConecting:
			{
				sServer.sin_len = sizeof(struct sockaddr_in);
				sServer.sin_family = AF_INET;
				sServer.sin_port =  htons(ServerAddr.port);
				sprintf(cbuf,"%d.%d.%d.%d", ServerAddr.ipaddr.addr1, ServerAddr.ipaddr.addr2, ServerAddr.ipaddr.addr3, ServerAddr.ipaddr.addr4);
				printf("开始连接服务器务器ip地址是：%s\r\n",cbuf);
				inet_aton(cbuf, &(sServer.sin_addr));
				ret = connect(iSSocket,(struct sockaddr*)(&sServer),sizeof(sServer));		//连接服务器
				if (ret < 0) 
				{
					printf("连接服务器务器ip地址失败\r\n");
					NetSta = NetInit;
					SysTimeDly(10);
					close(iSSocket);
					SysTimeDly(400);
				} 
				else 
				{
					printf("连接服务器务器ip地址成功\r\n");
					SysTimeDly(10);
					NetSta = NetConected;
				}
			}
			case NetConected:
			{
                #if 0
				memset(gcNetRecvBuf, 0, NETBUFLEN);
				ret = recv(iSSocket,gcNetRecvBuf,NETBUFLEN,100);
				if(ret < 0) 
				{
					if(NetSta == NetConected) 
					{
						NetSta = NetInit;
						SysTimeDly(100);
						close(iSSocket);
					}
					SysTimeDly(200);
				}
				else if(0 == ret)
				{
					printf("%d 接收超时\r\n",SysTimeGet());
				}
				else 
				{
					send(iSSocket,gcNetRecvBuf,ret,0);		//发送数据
				}
                #else
                memset(gcNetRecvBuf, 'A', NETBUFLEN);
                if(send(iSSocket,gcNetRecvBuf,NETBUFLEN,0) <0 )
                {
                    NetSta = NetClose;
                }
                #endif
                SysTimeDly(1);
				break;
			}
			case NetClose:
			{
				NetSta = NetInit;
                close(iSSocket);
				SysTimeDly(100);
				break;
			}
			default:
			{

				NetSta = NetInit;
				SysTimeDly(100);
                close(iSSocket);
				break;
			}
		}
	}
}

//服务器模型
void lwipServer(void)
{
  //可连接客户端的文件描述符数组
  static int connect_host[CLIENTNUM];
  static int connect_number =  0;
  int sock, newconn, size,revLen,i;
	//int maxfd = -1;                                         //最大侦听文件描述符
	//fd_set scanfd;	                                        //侦听描述符集合
	//struct   timeval   timeout; 	                        //超时
  struct sockaddr_in address, remotehost;

    
	//timeout.tv_sec     =   0;                               //阻塞1秒后超时返回    
	//timeout.tv_usec   =   100;    
	memset(connect_host, -1, (sizeof(int)*CLIENTNUM));


	printf("%d create a TCP socket\r\n",SysTimeGet());
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{
			printf("can not create socket");
			return;
	}

	// bind to port 9999 at any interface
	address.sin_family = AF_INET;
	address.sin_port = htons(9999);
	address.sin_addr.s_addr = INADDR_ANY;
	printf("%d bind to port 9999 at any interface\r\n",SysTimeGet());
	if (bind(sock, (struct sockaddr *)&address, sizeof (address)) < 0)
	{
			printf("can not bind socket");
			return;
	}

	//listen for incoming connections (TCP listen backlog = 5)
	printf("%d listen for incoming connections (TCP listen backlog = 5)\r\n",SysTimeGet());
	listen(sock, 5);

	size = sizeof(remotehost);

	while (1) 
	{
		printf("%d waitting accept a new\r\n",SysTimeGet());
		newconn = accept(sock, (struct sockaddr *)&remotehost, (socklen_t *)&size);

		printf("a client connect, from:%s port = %d\n",inet_ntoa(remotehost.sin_addr),remotehost.sin_port);
		while(1)                                            //继续轮询等待客户端连接
		{
			for(i=0;i<CLIENTNUM;i++)                        //查找合适位置，将客户端的文件描述符放入				
			{
				if(connect_host[i] == -1)                   //找到
				{
					connect_host[i]= newconn;               //放入
					connect_number ++;                      //客户端计数器加1
					break;						
				}	
			}
					
			#if 0
			maxfd = -1;                                     //最大文件描述符值初始化为-1		
			FD_ZERO(&scanfd);                               //清零文件描述符集合
			for(i=0;i<CLIENTNUM;i++)                        //将文件描述符放入集合
			{
				if(connect_host[i] != -1)                   //合法的文件描述符
				{
					FD_SET(connect_host[i], &scanfd);       //放入集合
					if(maxfd < 	connect_host[i])            //更新最大文件描述符值
					{
						maxfd = connect_host[i];
					}
				}
			}
				
			err = select(maxfd + 1, &scanfd, NULL, NULL, &timeout) ;//select等待		
			switch(err)
			{
				case 0:                                     //超时
									printf(" 接收数据超时\r\n");
					break;
				case -1:                                    //错误发生
					break;
				default:                                    //有可读套接字文件描述符
				if(connect_number<=0)
				{
					break;
				}
				for(i = 0;i<CLIENTNUM;i++)
				{
					
					if(connect_host[i] != -1)               //查找激活的文件描述符
					if(FD_ISSET(connect_host[i],&scanfd))   
					{  
						memset(gcNetRecvBuf,0,NETBUFLEN);
						revLen = recv(connect_host[i], gcNetRecvBuf, NETBUFLEN,0); 
						if(revLen >0)
						{
							printf("%s",gcNetRecvBuf);
							send(connect_host[i], gcNetRecvBuf, revLen,0);
						}
						else
						{
							close(connect_host[0]);
							break;
						}
					} 
				}
				break; 	
			}	
			#else
			revLen = recv(connect_host[0], gcNetRecvBuf, NETBUFLEN,0); 
			if(revLen >0)
			{
				//printf("%s",gcNetRecvBuf);
				send(connect_host[0], gcNetRecvBuf, revLen,0);
			}
			else
			{
				close(connect_host[0]);
				break;
			}
			#endif
		}
	}
}

//测试线程
void LwipJobProcess(void)		//任务管理线程
{
  Glocal_IP_ADDR local_IP =		                            //本机IP
	{
		192,168,50,111,
		255,255,255,0,
		192,168,50,1
	};
    sMAC_ADDR MAC_ADDR={0x2E,0x21,0x22,0x23,0x24,0x5C};     //MAC地址
    sTASK_PARAM sTASK_PARAM={15,17,18};                     //互斥量和优先级，注意互斥量优先级要比创建线程的优先级高


	USART_STRUCT usart1cfg = 
	{
		57600,																	//波特率
		USART_WordLength_8b,										//数据位
		USART_StopBits_1,												//停止位
		USART_Parity_No													//效验位
	};
	if(USART1_Config(&usart1cfg) == 0)				//串口初始化
		while(1);
	Set_PrintfPort(1);						//设置打印串口
	
	printf("%d 初始化LWIP\r\n",SysTimeGet());
	if(LwIP_Init(local_IP,MAC_ADDR,sTASK_PARAM) == 0)
		while(1);
	
	SysTimeDly(10);
	
	printf("修改IP地址\r\n");
	LwIP_Set(local_IP);
	
	#if 0
	while(1)
	{
		printf("%d ",LwIP_GetStatus());//读取网线连接状态0表示网线已经连接上 -1表示断开，该函数只有在网络初始化完成后调用
		SysTimeDly(10);
	}
	#endif

	#if 1
	lwipClient();
	#else
	lwipServer();
	#endif
	
	while(1)
	{
		SysTimeDly(1);
	}
}
*/
