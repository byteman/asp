#include "DyncSensorGetAxisAlgo.h"
#include <string.h>
#include <math.h>
#include "stdio.h"

char  SoftAlexDyncProcInit(void* pDecb)
{	
	return 0;
}

float SoftAlexDyncProc(void* pDecb, float * pInArr, int nInCount)
{	
	float mScaleWet = pInArr[0] +pInArr[2];

	return mScaleWet;
}


