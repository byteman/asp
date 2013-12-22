/*
 * File: softdog.c
 * Brief: Soft WatchDog File
 
 * History:
 * 1.2013-1-25 create file;
 */

#include "includes.h"
#include "dog_init.h"
#include "softdog.h"

#define HardSoftCnt  	50		//硬件看门狗50ms喂一次
#define MaxSoftDogNum 	8

static HANDLE dog_handle;

struct soft_watchdog_timer {
    uint16 time_out;    //看门狗计数初值, 计时单位10ms
    uint16 time;		//看门狗计数器
    SoftDogState  state;		//状态
    const char *name;
};

static uint8 SoftDogNum = 0;
static volatile BOOL SoftDogChanger = FALSE;

static struct soft_watchdog_timer SoftDogList[MaxSoftDogNum] = {0};

void Set_SoftDog_Changer(BOOL flag)
{
	SoftDogChanger = flag;
}

void SoftWatchDog_ISR(void) 
{
	static uint16 softcnt = 0;
    static BOOL StopDogFlag = FALSE;
	static uint16 cnt = 0;
	static BOOL flag = FALSE;
	uint8 i = 0;

	//软喂狗时间超时,软件异常,需要复位
    if(StopDogFlag) return;

	softcnt += 10;

	//开机延迟3s运行软件看门狗, 等待所有线程完全启动
	if(!flag)
	{
		cnt++;
		if(cnt >= 300)
		{
			flag = TRUE;
		}
	}

	//线程监测
	if(flag && !SoftDogChanger)
	{
	    for(i = 0; i < SoftDogNum; i++) 
	    {
	        if(SoftDogList[i].state == SOFTDOG_RUN) 
	        {
				SoftDogList[i].time--;
				//软喂狗超时
				if(SoftDogList[i].time == 0)
				{
					StopDogFlag = TRUE;
					//打印出错信息
					printf("Err:%s SoftDog Out!\r\n", SoftDogList[i].name);
					return;
				}
			}
		}
	}

	//正常情况下定时喂狗
	if(softcnt >= HardSoftCnt) 
	{
		softcnt = 0;
		feed_dog();	
	}
}

//软件喂狗
void SoftDog_Feed(uint32 id)
{
#if SYS_CRITICAL_METHOD == 3  	 
	SYS_CPU_SR  cpu_sr = 0;
#endif

	if(id >= SoftDogNum) return;
	
	SYS_ENTER_CRITICAL();
	SoftDogList[id].time = SoftDogList[id].time_out;
	SYS_EXIT_CRITICAL();
}

int SoftDog_Init(void)
{
	uint8 err;
	
	if(Dog_Init() == 0) {
		debug(Debug_Error, "WatchDog init err\r\n");
		return 0;
	}
	
	dog_handle = DeviceOpen(CHAR_DOG, 0, &err);
	feed_dog();
	
	return 1;
}
	
//硬件喂狗
void feed_dog(void)
{
	if(dog_handle != NULL)
		DeviceIoctl(dog_handle,DOGCMD_FEED,0);
}

/*
 * Function: 注册软件看门狗需要的参数
 * Param:
 * time:  最大监测时间, 单位10ms
 * state: 初始化状态
 * name:  线程名字
 */
int Register_SoftDog(uint16 time, SoftDogState state, const char *name)
{
	int id = 0;
	
	if(time == 0) return -1;
	if(name == NULL) return -1;
	if(SoftDogNum >= MaxSoftDogNum) return -2;

	id = SoftDogNum;
	
	SoftDogList[id].state = state;
	SoftDogList[id].time = time;
	SoftDogList[id].time_out = time;
	SoftDogList[id].name = name;

	SoftDogNum++;

	return id;
}
			
