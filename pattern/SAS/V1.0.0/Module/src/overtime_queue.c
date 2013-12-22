#include "overtime_queue.h"
#include "app_rtc.h"

#define MAX_QUEUE  11         //队列的最大数据元素数目

typedef struct queue
{                 
    OvertimeData item[MAX_QUEUE];   
    int front, rear; //队头指针、队尾指针
	int size;
}QUEUE;

static QUEUE Q = {{0}};

//待发送数据流水号, 范围1~59999
static uint16 SerialID = 1;

/**************************************************
 * 队列初始化
 * 函数名称：	OverQueue_Init
 * 函数功能：	初始化一个待发送车辆数据队列，包括倒车信息数据
 * 参数：		无
 * 返回值：		无
 *************************************************/
void OverQueue_Init(void)
{
	  CSysTime *time = Get_System_Time();

	  Q.front = 0;
	  Q.rear = 0;
	  Q.size = 0;
 

	  srand(((unsigned int)time->hour << 16) | ((unsigned int)time->min << 8) | ((unsigned int)time->sec));
	  SerialID = rand();

}

/**************************************************
 * 数据入队
 * 函数名称：	OverQueue_Enqueue
 * 函数功能：	把一个车辆数据添加到队列尾部
 * 参数：		OvertimeData *data：指向车辆结构体的指针
 * 返回值：		无
 *************************************************/
int OverQueue_Enqueue(OvertimeData *data)
{
#if SYS_CRITICAL_METHOD == 3  	 
    SYS_CPU_SR  cpu_sr = 0;
#endif

	SYS_ENTER_CRITICAL();
	if ((Q.rear+1)%MAX_QUEUE == Q.front) 
	{
		SYS_EXIT_CRITICAL();
		debug(Debug_Business, "Warning:OverQueue full!\r\n");
		return -1;
	}
	Q.rear=(Q.rear+1)%MAX_QUEUE;
	Q.size++;
	
	data->SerialID = SerialID;
	memcpy(&Q.item[Q.rear], data, sizeof(OvertimeData));	

	SerialID++;
	if(SerialID > 59999) SerialID = 1;

	SYS_EXIT_CRITICAL();

	return 0;
}

/**************************************************
 * 数据入队
 * 函数名称：	OverQueue_GetSeriID
 * 函数功能：	
 * 参数：		无
 * 返回值：		当前序列号
 *************************************************/
unsigned short OverQueue_GetSeriID()
{
	return SerialID;
}

/**************************************************
 * 移除队头元素
 * 函数名称：	OverQueue_Remove_Head
 * 函数功能：	删除队列的队头元素
 * 参数：		无
 * 返回值：	无
 **************************************************/
void OverQueue_Remove_Head(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
    SYS_CPU_SR  cpu_sr = 0;
#endif

	SYS_ENTER_CRITICAL();
	if (Q.size == 0) 
	{
		SYS_EXIT_CRITICAL();
		return;
	}

    Q.front = (Q.front+1)%MAX_QUEUE;
    memset(&Q.item[Q.front], 0, sizeof(OvertimeData));
	if(Q.size > 0)
	{
		Q.size--;
	}
    SYS_EXIT_CRITICAL();
}

/**************************************************
 * 移除队尾元素
 * 函数名称：	OverQueue_Remove_Tail
 * 函数功能：	删除队列的队尾元素
 * 参数：		无
 * 返回值：		无
 **************************************************/
void OverQueue_Remove_Tail(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
        SYS_CPU_SR  cpu_sr = 0;
#endif

	SYS_ENTER_CRITICAL();
	if (Q.size == 0)
	{
		SYS_EXIT_CRITICAL();
		return;
	}

	memset(&Q.item[Q.rear], 0, sizeof(OvertimeData));

	if(Q.rear > 0)
	{
		Q.rear = (Q.rear-1);
	}
	else
	{
    	Q.rear = MAX_QUEUE - 1;
	}

	if(Q.size > 0)
	{
		Q.size--;
	}
    SYS_EXIT_CRITICAL();
}

/**************************************************
 * 获取队头元素
 * 函数名称：	OverQueue_Get_Head
 * 函数功能：	获取队列的队头元素
 * 参数：		无
 * 返回值：		指向车辆结构体的指针
 **************************************************/
OvertimeData *OverQueue_Get_Head(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
		SYS_CPU_SR	cpu_sr = 0;
#endif
	
	OvertimeData *p = NULL;

	SYS_ENTER_CRITICAL();
	if (Q.size == 0) 
	{
	  SYS_EXIT_CRITICAL();
	  return NULL;
	}

	p = &(Q.item[(Q.front+1)%MAX_QUEUE]);
	SYS_EXIT_CRITICAL();

	return(p);
}

/**************************************************
 * 获取队尾元素
 * 函数名称：	OverQueue_Get_Tail
 * 函数功能：	获取队列的队尾元素
 * 参数：		无
 * 返回值：		指向车辆结构体的指针
 **************************************************/
OvertimeData *OverQueue_Get_Tail(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
		SYS_CPU_SR	cpu_sr = 0;
#endif

	OvertimeData *p = NULL;

	SYS_ENTER_CRITICAL();
	if (Q.size == 0) 
	{
	  SYS_EXIT_CRITICAL();
	  return NULL;
	}

	p = &(Q.item[Q.rear]);
	SYS_EXIT_CRITICAL();
	
	return(p);
}


/**************************************************
 * 增加队头元素发送次数
 * 函数名称：	OverQueue_Add_HeadSendNum
 * 函数功能：	增加队头元素发送次数
 * 参数：		无
 * 返回值：		无
 **************************************************/
void OverQueue_Add_HeadSendNum(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
	SYS_CPU_SR  cpu_sr = 0;
#endif

	SYS_ENTER_CRITICAL();
	if (Q.size == 0)
	{
		SYS_EXIT_CRITICAL();
		return ;
	}

	(Q.item[(Q.front+1)%MAX_QUEUE]).SendNum++;
	SYS_EXIT_CRITICAL();
}


/**************************************************
 * 获取队头元素发送次数
 * 函数名称：	OverQueue_Get_HeadSendNum
 * 函数功能：	获取队头元素发送次数
 * 参数：		无
 * 返回值：		int 队头元素的发送次数
 **************************************************/
int OverQueue_Get_HeadSendNum(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
		SYS_CPU_SR	cpu_sr = 0;
#endif

	int n = -1;

	SYS_ENTER_CRITICAL();
	if (Q.size == 0) 
	{
	  SYS_EXIT_CRITICAL();
	  return -1;
	}

	n = (Q.item[(Q.front+1)%MAX_QUEUE]).SendNum;
	SYS_EXIT_CRITICAL();

	return(n);
}


/**************************************************
 * 队列是否为空
 * 函数名称：	OverQueue_Empty
 * 函数功能：	判断队列是否为空
 * 参数：		无
 * 返回值：		bool true：为空；false：不为空
 **************************************************/
 #if 0
BOOL OverQueue_Empty(void)
{
	if (Q.front==Q.rear) 
	  return TRUE;
	else 
	  return FALSE;
}
#endif

//获取队列长度
int OverQueue_Get_Size(void)
{
#if SYS_CRITICAL_METHOD == 3  	 
		SYS_CPU_SR	cpu_sr = 0;
#endif
	int n;

	SYS_ENTER_CRITICAL();
	n = Q.size;
	SYS_EXIT_CRITICAL();

	return n;    //((Q.rear-Q.front+MAX_QUEUE) % MAX_QUEUE);
}

