#include "DyncZerolize.h"



/*
 * 功能:    动态零位处理
 */
float  DyncZeroLize(void *pZCB, float fIn, int bStaticZeroState)
{
    if(bStaticZeroState) 
    {
        SDYNCZERO(pZCB).fZeroValue = fIn;
    }

    return (fIn - SDYNCZERO(pZCB).fZeroValue);
}




