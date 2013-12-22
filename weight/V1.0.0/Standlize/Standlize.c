#include "Standlize.h"



/*
 * 功能:    标准化
 */
float  Standlize(void *pSCB,float fIn)
{
	if (pSCB == 0)
	{
		return 0;
	}
    return fIn*(SSTAND(pSCB).fParam);
}




