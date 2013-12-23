#include "math.h"
#include "ZeroTrace.h"
#include "..\drv\drvproc.h" 

/*
 * 功能:    置零或清除置零
 * 输入:
 *          pZtcb : 注册时参数
 *					bSet : 1，置零；0，清除置零
 * 输出     float：零跟结果
 */
void SetZero(void *pZtcb, signed char bSet)
{
		if(bSet > 0)
		{
			SZEROTRACE(pZtcb).cHandZerosFlag = 1;   //手动置零
		}
		else
		{
			SZEROTRACE(pZtcb).cHandZerosFlag = -1;
		}
}

/*
 * 功能:    零位跟踪，三点跟踪，取中间点
 * 输入:
 *          in : 输入
 * 输出     float：零跟结果
 */
float ZeroTrace(void *pZtcb, float in)
{
	float fDelt;
	float fOut;
    
	if(pZtcb == NULL) return in;

	//开机计时
	if(SZEROTRACE(pZtcb).nPowerIndex < SZEROTRACE(pZtcb).nPowerTime)
	{		
		SZEROTRACE(pZtcb).nPowerIndex++;
		return 0;
	}
	
	//开机置零
	if(SZEROTRACE(pZtcb).nPowerIndex == SZEROTRACE(pZtcb).nPowerTime)
	{	
        if(fabs(WetDrvProc(in, SZEROTRACE(pZtcb).fDrv)) <= SZEROTRACE(pZtcb).fPowerRange)
        {
			SZEROTRACE(pZtcb).fPowerOnZero = in;
    		SZEROTRACE(pZtcb).fTraceZero = in;
        }
		
		SZEROTRACE(pZtcb).PointFirst = in;
		SZEROTRACE(pZtcb).PointMid = in;
		SZEROTRACE(pZtcb).PointLast = in;
		
		SZEROTRACE(pZtcb).nPowerIndex++;			//加一后不会再进入对于条件
        SZEROTRACE(pZtcb).cHandZerosFlag = 0;       //手动置零使用
		SZEROTRACE(pZtcb).nTraceIndex = 0;		    //零位跟踪使用
		
		return (in - SZEROTRACE(pZtcb).fTraceZero);
	}
	
	//手动置零
	if(SZEROTRACE(pZtcb).cHandZerosFlag == 1)
	{	
        SZEROTRACE(pZtcb).cHandZerosFlag = 0;
		
        if(fabs(WetDrvProc(in - SZEROTRACE(pZtcb).fTraceZero, SZEROTRACE(pZtcb).fDrv)) <= SZEROTRACE(pZtcb).fHandRange)
        {
				SZEROTRACE(pZtcb).fPowerOnZero = in;
    		SZEROTRACE(pZtcb).fTraceZero = in;
        }
	}
	else if(SZEROTRACE(pZtcb).cHandZerosFlag == -1)
	{
		//手动清除置零
		SZEROTRACE(pZtcb).cHandZerosFlag = 0;
		
		SZEROTRACE(pZtcb).fPowerOnZero = 0;
		SZEROTRACE(pZtcb).fTraceZero = 0;		
	}
	
	//不跟踪时不做处理
	if(SZEROTRACE(pZtcb).nTraceTime == 0)
	{
		SZEROTRACE(pZtcb).nTraceIndex = 0;
		return (in - SZEROTRACE(pZtcb).fTraceZero);
	}	
	
	//跟踪时间增加
	SZEROTRACE(pZtcb).nTraceIndex++;
	
	//还没达到跟踪时间
	if(SZEROTRACE(pZtcb).nTraceIndex < SZEROTRACE(pZtcb).nTraceTime)
	{
		return (in - SZEROTRACE(pZtcb).fTraceZero);
	}
	
	//已达到跟踪时间	
	SZEROTRACE(pZtcb).nTraceIndex = 0;
	
	//缓存
	SZEROTRACE(pZtcb).PointFirst = SZEROTRACE(pZtcb).PointMid;
	SZEROTRACE(pZtcb).PointMid = SZEROTRACE(pZtcb).PointLast;
	SZEROTRACE(pZtcb).PointLast = in;
	
	//判断跟踪范围
	if( (fabs(SZEROTRACE(pZtcb).PointFirst - SZEROTRACE(pZtcb).fTraceZero) < SZEROTRACE(pZtcb).fTraceRange) &&
		(fabs(SZEROTRACE(pZtcb).PointMid - SZEROTRACE(pZtcb).fTraceZero) < SZEROTRACE(pZtcb).fTraceRange)  && 
		(fabs(SZEROTRACE(pZtcb).PointLast - SZEROTRACE(pZtcb).fTraceZero) < SZEROTRACE(pZtcb).fTraceRange) )
	{
		//中间点的误差量
		fDelt = SZEROTRACE(pZtcb).PointMid - SZEROTRACE(pZtcb).fTraceZero;
		
		//在跟踪步长以内
		if(fabs(fDelt) <= SZEROTRACE(pZtcb).fTraceStep)
		{
			//直接置零
			SZEROTRACE(pZtcb).fTraceZero = SZEROTRACE(pZtcb).PointMid;
		}
		else
		{
			if(fDelt > 0)
			{
				SZEROTRACE(pZtcb).fTraceZero += SZEROTRACE(pZtcb).fTraceStep;	
			}
			else
			{
				SZEROTRACE(pZtcb).fTraceZero -= SZEROTRACE(pZtcb).fTraceStep;		
			}				
		}			
	}
	
	//绝对零点判断
	else if( (fabs(SZEROTRACE(pZtcb).PointFirst - SZEROTRACE(pZtcb).fPowerOnZero) < SZEROTRACE(pZtcb).fTraceRange) &&
		(fabs(SZEROTRACE(pZtcb).PointMid - SZEROTRACE(pZtcb).fPowerOnZero) < SZEROTRACE(pZtcb).fTraceRange)  && 
		(fabs(SZEROTRACE(pZtcb).PointLast - SZEROTRACE(pZtcb).fPowerOnZero) < SZEROTRACE(pZtcb).fTraceRange) )
	{
			//恢复到开机零点
			SZEROTRACE(pZtcb).fTraceZero = SZEROTRACE(pZtcb).fPowerOnZero;			
	}
	
	//输出
	fOut = in - SZEROTRACE(pZtcb).fTraceZero;

	return fOut;	
}




