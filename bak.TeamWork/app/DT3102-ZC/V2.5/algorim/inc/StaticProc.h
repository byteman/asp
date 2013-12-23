#ifndef __STATICPROC_H_
#define __STATICPROC_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct 
{
    char nInCount;
}sStaticProc;

//取参数宏
#define SSTATIC(x)	(*((sStaticProc*)x))

/*
 * 功能:    静态处理
 */
float  StaticProc(void* pSecb, float * pInArr, int nInCount);

float GetSmallWet(void);
void SetSmallWetZero(int bSet);

float GetBigWet(void);
void SetBigWetZero(int bSet);

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
