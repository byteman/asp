#ifndef __LINE_FIT_H_
#define __LINE_FIT_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct _FitPoint
{
	float x;
	float y;
	float k;
} FitPoint;

typedef struct _sLineFit
{
	FitPoint   *pFitPoint;		//标定点
	int		nFitCount;					//标定点数
}sLineFit;

//取参数宏
#define SLINEFIT(x)	(*((sLineFit*)x))



/*
 * 功能:    线性拟合
 * 输入:
 *          in : 输入
 * 输出     float：滤波结果
 */
float LineFit(void *pNcb, float in);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
