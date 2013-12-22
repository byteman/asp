#ifndef SOFTDOG_H
#define SOFTDOG_H

//软件看门狗状态
typedef enum 
{
    SOFTDOG_RUN = 0,
    SOFTDOG_STOP
}SoftDogState;

void SoftWatchDog_ISR(void);
int SoftDog_Init(void);

//软件看门狗开关
//true 关闭 false 打开
void Set_SoftDog_Changer(BOOL flag);

//软件喂狗
void SoftDog_Feed(uint32 id);

//硬件喂狗
void feed_dog(void);

//注册软件看门狗需要的参数
int Register_SoftDog(uint16 time, SoftDogState state, const char *name);


#endif
