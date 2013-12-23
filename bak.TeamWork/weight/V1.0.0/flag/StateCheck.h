#ifndef __STATE_CHECK_H_
#define __STATE_CHECK_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef struct _sStateCheck
{
    int         nTimeIndex;
    int         nTimeRange;
    int         nPreState;
	float 	    fPrev;
	float		fSteadyRange;					//稳定判断范围
	float		fZeroRange;					    //零位判断范围
}sStateCheck;
   
//取参数宏
#define SSTATECHECK(x)	(*((sStateCheck*)x))


/*
 * 功能:    线性拟合
 * 输入:
 *          in : 输入
 * 输出     float：滤波结果
 */
int StateCheck(void *pStcb, float in);

char GetSteadySta(int State);
char GetZeroSta(int State);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
