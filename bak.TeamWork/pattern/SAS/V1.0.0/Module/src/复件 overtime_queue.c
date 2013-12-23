#include "overtime_queue.h"
#include "app_rtc.h"
#include "debug_info.h"

#define MAX_QUEUE  10         //队列的最大数据元素数目

typedef struct queue
{                 
    OvertimeData item[MAX_QUEUE];   
    int front; //队头指针 队尾指针=(front+size)%MAX_QUEUE
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
	  CSysTime *time = NULL;
	  
	  Update_System_Time();
	  time = Get_System_Time();

	  Q.front = 0;
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
	if (Q.size >= MAX_QUEUE) 
	{
		SYS_EXIT_CRITICAL();
		debug(Debug_Business, "Warning:OverQueue full!\r\n");
		return -1;
	}
	
	data->SerialID = SerialID;
	SerialID++;
	if(SerialID > 59999) SerialID = 1;
	
	Q.size++;
	memcpy(&Q.item[(Q.front+Q.size)%MAX_QUEUE], data, sizeof(OvertimeData));	

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
#if SYS_CRITICAL_METHOD == 3  	 
    SYS_CPU_SR  cpu_sr = 0;
#endif
	uint16 tmp;

	SYS_ENTER_CRITICAL();
	tmp = SerialID;
	SYS_EXIT_CRITICAL();

	return tmp;
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

	memset(&Q.item[(Q.front+Q.size)%MAX_QUEUE], 0, sizeof(OvertimeData));
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

	p = &(Q.item[(Q.front+Q.size)%MAX_QUEUE]);
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
 * 队列是否为空
 * 函数名称：	OverQueue_Empty
 * 函数功能：	判断队列是否为空
 * 参数：		无
 * 返回值：		bool true：为空；false：不为空
 **************************************************/
__inline BOOL OverQueue_Empty(void)
{
	return (Q.size == 0) ? TRUE : FALSE;
}

//队列满判断
__inline BOOL OverQueue_Full(void)
{
	return (Q.size >= MAX_QUEUE) ? TRUE : FALSE;
}


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

OvertimeData *OverQueue_Get_Index(uint8 index)
{
#if SYS_CRITICAL_METHOD == 3  	 
	SYS_CPU_SR	cpu_sr = 0;
#endif
	OvertimeData *pCar = NULL;

	SYS_ENTER_CRITICAL();

	if(index < 1) 
	{
		SYS_EXIT_CRITICAL();
		return NULL;
	}

	if((int)index <= OverQueue_Get_Size())
	{
		pCar = &(Q.item[(Q.front+index)%MAX_QUEUE]);
	}

	SYS_EXIT_CRITICAL();

	return pCar;
}

