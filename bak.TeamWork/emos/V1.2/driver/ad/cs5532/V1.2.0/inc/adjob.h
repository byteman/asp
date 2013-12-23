#ifndef _ADJOB_INIT_H_	
#define _ADJOB_INIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include <stdbool.h>	
// #define AD_CMDStart			0			//开启采样
// #define AD_CMDEnd			1			//停止采样
#define AD_CMDCheck			2				//AD芯片故障查询		用返回32位的低4位表示，A高B次高C次低D低		1故障0正常

#define	ERROR_AD1	0x01
#define	ERROR_AD2	0x02
#define	ERROR_AD3	0x04
#define	ERROR_AD4	0x08

/****************************************************************************
* 名	称：u8 ADJOB_init(u8 PRIO_t)
* 功	能：AD采样任务初始化
* 入口参数：u8 PRIO_t							优先级
* 出口参数：u8		是否成功   1成功  0失败
* 说	明：无
****************************************************************************/
bool ADJOB_init(unsigned char PRIO_t);	 	//采样任务初始化

/****************************************************************************
* 名	称：HANDLE DeviceOpen(DEV_ID ID,u32 lParam,HVL_ERR_CODE* err)
* 功	能：设备打开获得设备句柄
* 入口参数：DEV_ID ID						设备ID号
						u32 lParam					参数				无用
						HVL_ERR_CODE* err		错误状态
* 出口参数：HANDLE 							设备句柄
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：HVL_ERR_CODE DeviceClose(HANDLE IDH)
* 功	能：设备关闭
* 入口参数：HANDLE IDH					设备句柄
* 出口参数：HVL_ERR_CODE* err		错误状态
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceRead(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备读操作
* 入口参数：HANDLE IDH		设备句柄
						u8* Buffer		存放数据地址					无用		
						u32 len				希望读取的数据个数		无用
						u32* Reallen	实际读取的数据个数		获得时长计时值
						u32 lParam		参数									无用
* 出口参数：s32						读取操作是否成功			无用
* 说	明：无
****************************************************************************/	

/****************************************************************************
* 名	称：s32 CharDeviceWrite(HANDLE IDH,u8* Buffer,u32 len,u32* Reallen,u32 lParam)
* 功	能：设备写操作
* 入口参数：HANDLE IDH		设备句柄							无用
						u8* Buffer		写数据源地址					无用
						u32 len				希望写入的数据个数		无用
						u32* Reallen	实际写入的数据个数		无用
						u32 lParam		参数									无用
* 出口参数：s32						写入操作是否成功			无用
* 说	明：
****************************************************************************/	

/****************************************************************************
* 名	称：s32 DeviceIoctl(HANDLE IDH,u32 cmd,u32 lParam)
* 功	能：设备控制
* 入口参数：HANDLE IDH		设备句柄
						u32 cmd				设备命令
						u32 lParam		命令参数见说明
* 出口参数：s32 					命令返回值见说明
* 说	明：
命令											参数							返回值
AD_CMDStart								无用							无用
AD_CMDEnd									无用							无用
AD_CMDCheck								无用							无用
****************************************************************************/	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif

/*
void Job1ManagerProcess(void)		//任务管理线程
{
	u32 ErrCnt = 0;								//错误次数
 	u8 err = 0;
// 	u32 xx = 0;									//调试开关采样
// 	u8 yy = 0;
// 	s32 ADstateFlag = 0;				//转换芯片是否出错
	
	long AD[4] = {0};						//得到的AD值
	
	HANDLE ADJOB_HANDLE;				//AD任务设备句柄
	
	USART_STRUCT usart1cfg = 
	{
		57600,																	//波特率
		USART_WordLength_8b,										//数据位
		USART_StopBits_1,												//停止位
		USART_Parity_No													//效验位
	};

	USART1_Config(&usart1cfg);								//串口初始化
	Set_PrintfPort(1);												//设置打印串口
	
	printf("开始\r\n");
	
	if(ADJOB_init(5) != 1)	//设备初始化		//任务优先级	AD获取消息队列	//AD获取优先级必须高于AD处理优先级
		while(1);
	
	ADJOB_HANDLE	= DeviceOpen(CHAR_AD,0,&err);			//打开AD采样任务
 	if(err != HVL_NO_ERR)
 		while(1);

	while(1)
	{
		if(CharDeviceRead(ADJOB_HANDLE,(u8*)AD,0,0,0) == 0)
			continue;
		
		printf("A = %d\r\nB = %d\r\nC = %d\r\nD = %d ERR = %d\r\n\r\n",AD[0],AD[1],AD[2],AD[3],ErrCnt);
		if(AD[0] == 0 || AD[1] == 0 || AD[2] == 0 || AD[3] == 0)
			ErrCnt++;
		
 		AD[0] = 0;		//CS5532A
 		AD[1] = 0;		//CS5532B
 		AD[2] = 0;		//CS5532C
 		AD[3] = 0;		//CS5532D
 		SysTimeDly(1);
			
		
//  	printf("A = %d\r\n",AD[0]);
//  	printf("B = %d\r\n",AD[1]);
//  	printf("C = %d\r\n",AD[2]);
// 		printf("D = %d\r\n\r\n",AD[3]);
		
// 		AD[0] = 0;
// 		AD[1] = 0;
// 		AD[2] = 0;
// 		AD[3] = 0;
		
// 		if(yy == 1)
// 			ADstateFlag = DeviceIoctl(ADJOB_HANDLE,AD_CMDCheck,0);		//检测转换芯片是否出错 用返回32位的低4位表示，A高B次高C次低D低		1故障0正常
// 		ADstateFlag = ADstateFlag;
// 		ADstateFlag = 0;
// 		yy = 0;
// 		
// 		xx++;																							
// 		if(xx == 2)			//因ADJOB的43行进行了SysTimeDly(50);	的延时，故信号量要累加，多等几次让其释放完
// 			DeviceIoctl(ADJOB_HANDLE,AD_CMDEnd,0);					//关闭采样
// 		
// 		if(xx == 1000)
// 			DeviceIoctl(ADJOB_HANDLE,AD_CMDStart,0);				//开启采样
// 		
// 		if(xx == 2000)
// 			DeviceIoctl(ADJOB_HANDLE,AD_CMDEnd,0);					//关闭采样
// 		
// 		if(xx == 3000)
// 			DeviceIoctl(ADJOB_HANDLE,AD_CMDStart,0);				//开启采样
// 		
// 		if(xx == 4000)
// 			DeviceClose(ADJOB_HANDLE);				//关闭设备		
	}
}
*/
