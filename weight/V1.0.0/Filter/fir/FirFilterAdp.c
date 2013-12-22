#include "FirFilter.h"

/****************************************************************************************************
 *  滤波系数
 ***************************************************************************************************/
float const FastFirNum[125] = {
        0.0,0.0,0.0,6.396e-15,6.892e-14,4.47e-13,2.191e-12,8.866e-12,3.101e-11,9.644e-11,
        0.383e-10,0.991e-10,1.682e-09,3.754e-09,7.838e-09,1.534e-08,2.812e-08,4.81e-08,7.599e-08,1.086e-07,
        1.334e-07,1.19e-07,0.0,-3.459e-07,-1.13e-06,-2.696e-06,-5.565e-06,-1.049e-05,-1.851e-05,-3.094e-05,
        -4.937e-05,-7.556e-05,-1.112e-04,-1.575e-04,-2.147e-04,-2.812e-04,-3.528e-04,-4.211e-04,-4.729e-04,-4.881e-04,
        -4.395e-04,-2.914e-04,0.0,4.863e-04,1.225e-03,2.276e-03,3.7e-03,5.55e-03,7.869e-03,1.068e-02,
        1.399e-02,1.776e-02,2.195e-02,2.645e-02,3.116e-02,3.591e-02,4.055e-02,4.488e-02,4.873e-02,5.193e-02,
        5.433e-02,5.582e-02,5.633e-02,5.582e-02,5.433e-02,5.193e-02,4.873e-02,4.488e-02,4.055e-02,3.591e-02,
        3.116e-02,2.645e-02,2.195e-02,1.776e-02,1.399e-02,1.068e-02,7.869e-03,5.55e-03,3.7e-03,2.276e-03,
        1.225e-03,4.863e-04,0.0,-2.914e-04,-4.395e-04,-4.881e-04,-4.729e-04,-4.211e-04,-3.528e-04,-2.812e-04,
        -2.147e-04,-1.575e-04,-1.112e-04,-7.556e-05,-4.937e-05,-3.094e-05,-1.851e-05,-1.049e-05,-5.565e-06,-2.696e-06,
        -1.13e-06,-3.459e-07,0.0,1.19e-07,1.334e-07,1.086e-07,7.599e-08,4.81e-08,2.812e-08,1.534e-08,
        7.838e-09,3.754e-09,1.682e-09,0.991e-10,0.383e-10,9.644e-11,3.101e-11,8.866e-12,2.191e-12,4.47e-13,
        6.892e-14,6.396e-15,0.0,0.0,0.0
};

float const StaticFirNum[13] = {
        -2.272e-06,0.0,3.536e-03,3.004e-02,.1084,.22,.2761,.22,.1084,3.004e-02,
        3.536e-03,0.0,-2.272e-06
};


/*!
 * \brief    AD码初级滤波初始化
 * 
 * \param[in][out]	pFCB 滤波参数
 * \return 返回初始化结果
 * \retval 0  正常   
 * \retval -1 异常
 */
char  FirFilterInit(void *pFCB)
{
	if(pFCB == 0) return (char)-1;
	SFIR(pFCB).bInit = 0;

	return 0;
}

/*!
 *  \brief    FIR滤波
 *  \param	  pFCB 滤波参数
 *  \param    in   滤波输入AD值
 *  \return   滤波结果
 */
float FirFilterDynamic(void *pFCB, float in)
{
	int i;
	int nLen;
	const float* pZNum;
	float* pDelay;
	float Sum;

	return in;

	if(pFCB == 0) return in;
	nLen = SFIR(pFCB).nLen;
	if(nLen <= 0) return (float)in;

	pZNum  = SFIR(pFCB).pZNum;
	pDelay = SFIR(pFCB).pDelay;

	//初始化
	if(SFIR(pFCB).bInit == 0)
	{
		for(i = 0; i < nLen; i++)
		{
			pDelay[i] = (float)in;
		}

		SFIR(pFCB).bInit = 1;

		return 	(float)in;
	}

	//滤波
	Sum = 0;
	for(i = 0; i < nLen - 1; i++)
	{
		pDelay[i] = pDelay[i + 1];
		Sum += pDelay[i] * pZNum[i];			
	}

	//最后一个点
	pDelay[nLen - 1] = (float)in;
	Sum += pDelay[nLen - 1] * pZNum[nLen - 1];			
	
	return 	Sum * 1.0000266f;
}


float FirFilterStatic(void *pFCB, float in)
{
	int i;
	int nLen;
	const float* pZNum;
	float* pDelay;
	float Sum;

	if(pFCB == 0) return in;
	nLen = SFIR(pFCB).nLen;
	if(nLen <= 0) return (float)in;

	pZNum  = SFIR(pFCB).pZNum;
	pDelay = SFIR(pFCB).pDelay;

	//初始化
	if(SFIR(pFCB).bInit == 0)
	{
		for(i = 0; i < nLen; i++)
		{
			pDelay[i] = (float)in;
		}

		SFIR(pFCB).bInit = 1;

		return 	(float)in;
	}

	//滤波
	Sum = 0;
	for(i = 0; i < nLen - 1; i++)
	{
		pDelay[i] = pDelay[i + 1];
		Sum += pDelay[i] * pZNum[i];			
	}

	//最后一个点
	pDelay[nLen - 1] = (float)in;
	Sum += pDelay[nLen - 1] * pZNum[nLen - 1];			
	
	return 	Sum * 1.0000266f;
}
