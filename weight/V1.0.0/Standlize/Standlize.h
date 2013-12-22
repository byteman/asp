#ifndef __ADCNORMALIZE_H_
#define __ADCNORMALIZE_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct {
    float fParam;
}sStandlize;

//取参数宏
#define SSTAND(x)	(*((sStandlize*)x))

/*
 * 功能:    标准化
 */
float  Standlize(void *pSCB,float fIn);

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
