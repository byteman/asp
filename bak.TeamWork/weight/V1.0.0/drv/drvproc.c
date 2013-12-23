#include "drvproc.h" 

float WetDrvProc(float Wet, float Drv)
{
	long nStep;

	if(Drv == 0) Drv = 1;
	
	//分度值处理
	if(Wet >= 0)
	{
		nStep = (long)((2 * Wet + Drv) / (2 * Drv));
	}
	else
	{
		nStep = (long)((2 * Wet - Drv) / (2 * Drv));
	}

	return (float)(Drv * nStep);
}

