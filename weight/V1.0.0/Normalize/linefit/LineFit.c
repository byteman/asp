#include "LineFit.h"


/*
 * 功能:    线性拟合
 * 输入:
 *          pNcb : 参数
 *					in：输入值
 * 输出     float：拟合结果
 */
float LineFit(void *pNcb, float in)
{
	unsigned char i;
	unsigned char nSeg;
	unsigned char nFitCount;
	FitPoint* pFitPoint;
	
	if (pNcb == 0)
	{
		return in;
	}
	nFitCount = SLINEFIT(pNcb).nFitCount;
	if(nFitCount == 0) return in;
	
	pFitPoint = SLINEFIT(pNcb).pFitPoint;
		

	//寻找输入点所在的段
	nSeg = 0;
	for(i = 1; i < nFitCount; i++)
	{
		if(in >= pFitPoint[i].x) nSeg++;

		else break;
	}

	//拟合
	in = (in - pFitPoint[nSeg].x) * pFitPoint[nSeg].k + pFitPoint[nSeg].y;

	return in;	
}




