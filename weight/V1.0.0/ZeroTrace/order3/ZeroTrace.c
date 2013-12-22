#include "math.h"
#include "ZeroTrace.h"
#include "..\drv\drvproc.h" 

//置零
void SetZero(void *pZtcb)
{
    SZEROTRACE(pZtcb).cHandZerosFlag = 1;   //手动置零
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
	if(SZEROTRACE(pZtcb).cHandZerosFlag)
	{	
        SZEROTRACE(pZtcb).cHandZerosFlag = 0;
        if(fabs(WetDrvProc(in - SZEROTRACE(pZtcb).fTraceZero, SZEROTRACE(pZtcb).fDrv)) <= SZEROTRACE(pZtcb).fHandRange)
        {
    		SZEROTRACE(pZtcb).fTraceZero = in;
        }
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
	
	//输出
	fOut = in - SZEROTRACE(pZtcb).fTraceZero;

	return fOut;	
}




