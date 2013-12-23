#ifndef __DIG_SAMPLE_H_
#define __DIG_SAMPLE_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct _sDigSample
{
	int		nCycle;					//抽样周期
	int		nIndex;					//抽样索引
	float	fKeep;					//保持值
	float	fSum;						//中间求和值
}sDigSample;

//取参数宏
#define SDIGSAMPLE(x)	(*((sDigSample*)x))



/*
 * 功能:    抽样
 * 输入:
 *          in : 输入
 * 输出     int：抽样是否完成
 *					pOut：抽样输出
 */
int DigSample(void *pDscb, float in, float* pOut);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
