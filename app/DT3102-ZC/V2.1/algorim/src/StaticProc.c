#include "StaticProc.h"



/*
 * 功能:    静态处理
 */
float  StaticProc(void* pSecb, float * pInArr, int nInCount)
{
#if 0   
	char i;
	float wet=0;

	for(i = 0;i < nInCount; i++) 
	{
	    wet += pInArr[i];
	}

    return wet;
#endif
		return pInArr[0];
}




