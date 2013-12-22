#include <math.h>
#include "StateCheck.h"

#define STATE_ZERO      0x01
#define STATE_STEADY    0x10
/*
 * 功能:    稳定判断
 * 输入:
 *          pDcb : 参数
 *					float：输入值
 * 输出     int：判断结果
 */
int StateCheck(void *pStcb, float in)
{
    int state = 0;
    
	if (pStcb == 0)
	{
		return state;
	}
    SSTATECHECK(pStcb).nTimeRange = 10;
        
    SSTATECHECK(pStcb).nTimeIndex++;

    if(SSTATECHECK(pStcb).nTimeIndex >= SSTATECHECK(pStcb).nTimeRange)
    {
        SSTATECHECK(pStcb).nTimeIndex = 0;
        
    	if(fabs(in - SSTATECHECK(pStcb).fPrev) < SSTATECHECK(pStcb).fSteadyRange)
    	{
            state |= STATE_STEADY;
    	}
    	if(fabs(in) < SSTATECHECK(pStcb).fZeroRange)
    	{
            state |= STATE_ZERO;
    	}
    	
    	SSTATECHECK(pStcb).fPrev = in;
    	SSTATECHECK(pStcb).nPreState = state;
    } else {
    	state = SSTATECHECK(pStcb).nPreState;
    }

    return state;
}


char GetSteadySta(int State)
{
    char ret = 0;
    
    if(State & STATE_STEADY) {
        ret = 1;
    }

    return ret;
}

char GetZeroSta(int State)
{
    char ret = 0;
    
    if(State & STATE_ZERO) {
        ret = 1;
    }

    return ret;
}



