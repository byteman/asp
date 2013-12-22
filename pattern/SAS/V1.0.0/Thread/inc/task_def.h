#ifndef TASK_DEF_H
#define TASK_DEF_H

//任务优先级
enum {
	StartPrio			= 3,		//启动线程

 	ADJobPrio 			= 4,			//平台ad采集线程
	
	KeyPrio 			= 14,	//按键线程

	FRAMMutexPrio		= 8,			//平台层互斥量

	BeepPrio			= 9,			//平台层蜂鸣器线程

	AlgorimPrio	    = 12,    //算法线程		
	BusinessPrio		= 13,	 //业务线程

	USBPrio				= 15,  	//usb 

	Com6MutexPrio		= 17, 
	
	SerialRecvPrio		= 18,	//串口2接收线程
	DebugPrio			= 19,	//串口1调试线程

	FlashMutexPrio		= 25,	//平台层互斥量	
	DataManagerPrio 	= 26,   //数据管理线程
	WatchDogPrio		= 27,
	
	USBStatus			= 28,			//平台层usb线程 usb状态机		

	LwipMutexPrio		= 30,			//平台层网络互斥量

	LwipPrio1			= 31,			//平台层网络线程
	LwipPrio2			= 32,			//平台层网络线程

	NetPrio		        = 33,	//网络线程
	NetSendPrio		= 34,
	GUIPrio				= 35,  	//GUI线程
};

//void feed_dog(void);

#endif
