#ifndef __DYNCZEROLIZE_H_
#define __DYNCZEROLIZE_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct {
    float fZeroValue;   // 动态零位
}sDyncZero;

//取参数宏
#define SDYNCZERO(x)	(*((sDyncZero*)x))

/*
 * 功能:    动态零位处理
 */
float  DyncZeroLize(void *pZCB, float fIn, int bStaticZeroState);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
