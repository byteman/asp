#include "StaticProc.h"

//小秤重量
static float m_nSmallWet = 0.0f;
static float m_nSmallZero = 0.0f;
float GetSmallWet(void)
{
	return m_nSmallWet - m_nSmallZero;
}

void SetSmallWetZero(int bSet)
{
	if(bSet > 0)	m_nSmallZero = m_nSmallWet;
	else m_nSmallZero = 0;	
}


//大秤重量
static float m_nBigWet = 0.0f;
static float m_nBigZero = 0.0f;
float GetBigWet(void)
{
	return m_nBigWet - m_nBigZero;
}

void SetBigWetZero(int bSet)
{
	if(bSet > 0)	m_nBigZero = m_nBigWet;
	else m_nBigZero = 0;	
}

/*
 * 功能:    静态处理
 */
float  StaticProc(void* pSecb, float * pInArr, int nInCount)
{
		if(nInCount < 4)
		{
			m_nSmallWet = 0.0f;
			m_nSmallZero = 0.0f;
			m_nBigWet = 0.0f;
			m_nBigZero = 0.0f;
			return 0.0f;
		}
	
		m_nSmallWet = pInArr[2] + pInArr[3];
		m_nBigWet = pInArr[0];
	
		return pInArr[0] + pInArr[2] + pInArr[3];
}




