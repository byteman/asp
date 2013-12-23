#ifndef __ZERO_TRACE_H_
#define __ZERO_TRACE_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif


#ifndef NULL
#define NULL ((void *)0)
#endif

typedef struct _sZeroTrace
{
	int  		nTraceTime;					//零位跟踪时间
	int	 		nTraceIndex;				//零跟索引
	float		fPowerOnZero;				//零点
	float		fTraceZero;					//零点
	float		fTraceRange;  			    //零位跟踪范围
	float		fTraceStep;					//零位跟踪速度
	float		fHandRange;				    //手动置零范围
	signed char		cHandZerosFlag;			    //手动置零标志
	float		fPowerRange;				//开机置零范围
	int			nPowerTime;					//开机置零时间
	int			nPowerIndex;				//开机置零时间
	float PointFirst;			//缓存第一个点
	float PointMid;				//缓存中间点
	float PointLast;				//缓存最后一个点
	float fDrv;						//分度值
}sZeroTrace;

//取参数宏
#define SZEROTRACE(x)	(*((sZeroTrace*)x))


/*
 * 功能:    置零或清除置零
 * 输入:
 *          pZtcb : 注册时参数
 *					bSet : 1，置零；0，清除置零
 * 输出     float：零跟结果
 */
void SetZero(void *pZtcb, signed char bSet);


/*
 * 功能:    零位跟踪
 * 输入:
 *          in : 输入
 * 输出     float：零跟结果
 */
float ZeroTrace(void *pZtcb, float in);


#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

#endif
