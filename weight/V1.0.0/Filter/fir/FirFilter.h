#ifndef __FIR_FILTER_H_
#define __FIR_FILTER_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct _sFirFilter
{
	const float*  pZNum;		//零点
	float* 	pDelay;		//缓冲区
	int		nLen;		//长度
	int		bInit;		//是否已经初始化
}sFirFilter;

//取参数宏
#define SFIR(x)	(*((sFirFilter*)x))



/*
 * 功能:    FIR滤波初始化
 * 输入:
 *          ad : AD值
 * 输出     0: 正常   -1: 异常
 */
char  FirFilterInit(void *pFCB);

/*
 * 功能:    FIR滤波
 * 输入:
 *          ad : AD值
 * 输出     float：滤波结果
 */
float FirFilterDynamic(void *pFCB, float in);
float FirFilterStatic(void *pFCB, float in);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
