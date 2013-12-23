#include "DyncAlexParserThree.h"
#include <math.h>  
#include <stdlib.h>


//////////////////////////////////////////////

 /*
 * 功能:    动态轴处理初始化
 */
char  AlexDyncProcThreeInit(void* pDecb)
{
	if (pDecb == 0) return (char)-1;

	SALEXTHREEDYNC(pDecb).nAlexValid = 0;
	SALEXTHREEDYNC(pDecb).nPulseWidth = 0;
	SALEXTHREEDYNC(pDecb).nUpFlag = 0;
	SALEXTHREEDYNC(pDecb).nUpCnt = 0;
	SALEXTHREEDYNC(pDecb).nUPkeepCnt = 0;
	SALEXTHREEDYNC(pDecb).nDownkeepCnt = 0;
	SALEXTHREEDYNC(pDecb).nDownCnt = 0;

	if(SALEXTHREEDYNC(pDecb).mSpeedLen == 0)
		SALEXTHREEDYNC(pDecb).mSpeedLen = 80;
	SALEXTHREEDYNC(pDecb).fWet = 0;      
	SALEXTHREEDYNC(pDecb).fWetReliab = 0;     
	SALEXTHREEDYNC(pDecb).iPowerOnIndex = 0;  

	SALEXTHREEDYNC(pDecb).nAlexNo = 0; 
	SALEXTHREEDYNC(pDecb).nAlexINorRemove = 0; 
	SALEXTHREEDYNC(pDecb).nAlexWet = 0; 
	SALEXTHREEDYNC(pDecb).nAlexPulseWith = 0; 
	SALEXTHREEDYNC(pDecb).nAlexMaxWet = 0; 
	

    return 0;
}


/*
 * 功能:    动态轴处理
 */
float AlexDyncThreeProc(void* pDecb, float * pInArr, int nInCount)
{
	sAlexThreeDyncProc *pDync = (sAlexThreeDyncProc *)pDecb;
	
	pDync->mScaleWet = pInArr[0];

	return pDync->mScaleWet;	
}

