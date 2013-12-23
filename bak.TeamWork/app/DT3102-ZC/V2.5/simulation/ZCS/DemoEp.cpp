// DemoEp.cpp : 定义 DLL 的初始化例程。
//

#include "stdafx.h"
#include "ZcsEp.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


extern "C"
{
	__declspec(dllexport) int GetClassCount()
	{
		return 1;
	}

	__declspec(dllexport) CEp* SpawnClass(int index)
	{
		if(index == 0)
		{
			return new CZcsEp();
		}
		else if(index == 1)
		{
			//return new CStandlizeEp();
		}
		else if(index == 2)
		{
			//return new CZeroTraceEp();
		}
		return NULL;
	}

}
