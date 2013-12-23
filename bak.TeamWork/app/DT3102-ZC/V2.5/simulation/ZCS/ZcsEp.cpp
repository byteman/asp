#include "StdAfx.h"
#include "ZcsEp.h"
#include "resource.h"
#include "EpSelectDlg.h"
#include "ZcsDlg.h"
#include "Connection.h"
#include "Station.h"
//#include "AlsStation.h"
#include "sys_config.h"
#include "debug_info.h"
#include "MultimediaTimer.h"
#include "Weight.h"
#include "Mode7AlexParser.h"
#include "axletype_business.h"

extern CStation* g_pStatin;
extern gWetParam gWetPar;



#ifdef BOOL
#undef BOOL
#endif

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

typedef unsigned int HScaler; /*!> 称台对象句柄*/
extern HScaler hs_Main;

void ScalerDestory(HScaler hs);
float  GetScalerStaticZeroTraceValue(HScaler hs);

//算法业务
void Weight_Business(void *buf);

//算法模块初始化
int Weight_Init(void);

//获取秤台动态重量
int32 Get_Dynamic_Weight(void);

//设置秤台重量, 调试模拟使用
void Set_Static_Weight(int32 wet);		

//获取秤台静态重量
int32 Get_Static_Weight(void);	
int32 Get_Static_MaxWeight(void);	
void Clear_ScalerMaxWeight(void);	

//判断K值异常
uint8 Check_K_Value(float k);

//保存AD通道零点,标定零点
bool Weight_Calc_Zero(void);

//计算AD通道K系数
uint8 Weight_Cal_K(int inWet, int chanle);

//内蒙标定
bool SCSZC1_Cal_K(int inWet, int chanle, int place);


//设置秤台静态分度值
int Set_Scaler_StaticDrv(uint8 drv);

//设置秤台动态分度值
int Set_Scaler_DynamicDrv(uint8 drv);


//返回当前ad通道数
int32 Get_ADChanle_Num(void);

//初始化算法参数
void Init_Wet_Par(void);

void Set_ScalerStatic_K(float k);

//业务定时器复位
void BusinessTimerReset();

//业务定时器
void BusinessTimerTick();

//光幕1
void LC2MakeMsg(int nState);

//地感1
void GC1MakeMsg(int nState);

//地感2
void GC2MakeMsg(int nState);

//业务流程处理
void Business_Thread_Adp(void *arg);

void CarQueue_Init(void);

extern gWetParam gWetPar;
extern sMode7AlexDyncProc SCS_ZC7_DyncProc;

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
}
#endif

//打印可换行的调试信息
void OutPutMulitlineStr(CString StrOut, CString StrLine)
{
	if(g_pStatin != NULL)
	{
		CString strStep;
		int nStart = 0;
		int nLeave;

		g_pStatin->GetCommManager()->EnableDebugMsg(TRUE);

		if(StrOut.Find(StrLine, 0) < 0)
		{
			g_pStatin->GetCommManager()->OutputDebugStr(StrOut);
		}
		else
		{
			while(StrOut.GetLength() > 0)
			{
				nStart = StrOut.Find(StrLine, 0);
				if(nStart >= 0)
				{
					strStep = StrOut.Left(nStart);
					g_pStatin->GetCommManager()->OutputDebugStr(strStep);
				}
				else
				{
					if(StrOut.GetLength() > 0)
					{
						g_pStatin->GetCommManager()->OutputDebugStr(StrOut);
					}

					break;
				}

				//继续寻找剩余的
				nLeave = StrOut.GetLength() - nStart - StrLine.GetLength();
				if(nLeave > 0)
				{
					StrOut = StrOut.Right(nLeave);
				}
				else
				{
					break;
				}
			}
		}	
	}
}

CZcsEp::CZcsEp(void)
{
	m_bIsSignal = 1;
	m_strType = _T("float");
	m_nType = 2;
	m_nTypeSize = 4;

	m_nIcon = 3;

	m_bUseSim = FALSE;
}

CZcsEp::~CZcsEp(void)
{

}

CString CZcsEp::GetTypeString()
{
	return m_strType;
}

uint8 CZcsEp::GetTypeCode()
{
	return m_nType;
}

int	CZcsEp::GetTypeSize()
{
	return m_nTypeSize;
}

BOOL CZcsEp::IsSignal()
{
	return m_bIsSignal;
}

BOOL CZcsEp::OnDbClick()
{
	return OnProperty();
}

void CZcsEp::ResetZero(void)
{
	return;
}

void CZcsEp::SetZero(void)
{
	return;
}

BOOL CZcsEp::StoreConfig(CArchive &ar)
{
	ar<<GetSymbol();

	ar<<cur;				//当前观测值(用于观测数据)
	ar<<fs;					//采样率
	ar<<zero;				//调零时数据
	ar<<max;				//最大值(确定图形窗口范围时需要)
	ar<<min;				//最小值(确定图形窗口范围时需要)
	ar<<name;				//名称
	ar<<unit;				//数据的物理单位

	ar<<m_strType;
	ar<<m_nType;
	ar<<m_nTypeSize;
	ar<<m_bIsSignal;

	ar<<gWetPar.MaxWet;
	ar<<gWetPar.PowerZero;
	ar<<gWetPar.HandlZero;
	ar<<gWetPar.ZeroTruckSpeed;
	ar<<gWetPar.DynamicDivValue;
	ar<<gWetPar.StaticDivValue;
	ar<<gWetPar.ZeroTruckRang;
	ar<<gWetPar.StaticFilterLevel;
	ar<<gWetPar.DynamicFilterLevel;
	ar<<gWetPar.SuperSpeedLevel;
	ar<<gWetPar.ZeroTruckTick;
	ar<<gWetPar.PreAxleUpWet;
	ar<<gWetPar.PreAxleDownWet;
	ar<<gWetPar.BackAxleUpWet;
	ar<<gWetPar.BackAxleDownWet;
	ar<<gWetPar.TotalUpWet;
	ar<<gWetPar.TotalDownWet;
	ar<<gWetPar.Length;
	ar<<gWetPar.AxleLength;
	ar<<gWetPar.Speed_5k;
	ar<<gWetPar.Speed_10k;
	ar<<gWetPar.Speed_15k;
	ar<<gWetPar.Speed_20k;
	ar<<gWetPar.Speed_25k;
	ar<<gWetPar.Speed_30k;			//30km/h
	ar<<gWetPar.Max2AxleWet;		//车辆超重重量，按轴分
	ar<<gWetPar.Max3AxleWet;
	ar<<gWetPar.Max4AxleWet;
	ar<<gWetPar.Max5AxleWet;
	ar<<gWetPar.Max6AxleWet;
	ar<<gWetPar.MaxTrafficWet;			//红绿灯重量阈值
	ar<<gWetPar.DynamicK;	  			//动态系数
	ar<<gWetPar.PreAxle1_K;				//ad通道k系数
	ar<<gWetPar.PreAxle2_K;
	ar<<gWetPar.Scaler_K;
	ar<<gWetPar.BackAxle_K;
	ar<<gWetPar.Total_K;
	ar<<gWetPar.PreAxle1ZeroAD;			//ad通道ad零点
	ar<<gWetPar.PreAxle2ZeroAD;
	ar<<gWetPar.ScalerZeroAD;
	ar<<gWetPar.BackAxleZeroAD;

	int nEpInputCount = m_arrEpInput.GetCount();
	ar<<nEpInputCount;

	for(int i = 0; i < nEpInputCount; i++)
	{
		ar<<m_arrEpInput[i];
	}

	return TRUE;
}

BOOL CZcsEp::LoadConfig(CArchive &ar, UINT nFileVersion)
{
	ar>>cur;				//当前观测值(用于观测数据)
	ar>>fs;					//采样率
	ar>>zero;				//调零时数据
	ar>>max;				//最大值(确定图形窗口范围时需要)
	ar>>min;				//最小值(确定图形窗口范围时需要)
	ar>>name;				//名称
	ar>>unit;				//数据的物理单位

	ar>>m_strType;
	ar>>m_nType;
	ar>>m_nTypeSize;
	ar>>m_bIsSignal;

	if((nFileVersion & 0xFF000000) == 0xA5000000)
	{
		if((nFileVersion & 0x00FFFFFF) >= 0x00010102)
		{
			ar>>gWetPar.MaxWet;
			ar>>gWetPar.PowerZero;
			ar>>gWetPar.HandlZero;
			ar>>gWetPar.ZeroTruckSpeed;
			ar>>gWetPar.DynamicDivValue;
			ar>>gWetPar.StaticDivValue;
			ar>>gWetPar.ZeroTruckRang;
			ar>>gWetPar.StaticFilterLevel;
			ar>>gWetPar.DynamicFilterLevel;
			ar>>gWetPar.SuperSpeedLevel;
			ar>>gWetPar.ZeroTruckTick;
			ar>>gWetPar.PreAxleUpWet;
			ar>>gWetPar.PreAxleDownWet;
			ar>>gWetPar.BackAxleUpWet;
			ar>>gWetPar.BackAxleDownWet;
			ar>>gWetPar.TotalUpWet;
			ar>>gWetPar.TotalDownWet;
			ar>>gWetPar.Length;
			ar>>gWetPar.AxleLength;
			ar>>gWetPar.Speed_5k;
			ar>>gWetPar.Speed_10k;
			ar>>gWetPar.Speed_15k;
			ar>>gWetPar.Speed_20k;
			ar>>gWetPar.Speed_25k;
			ar>>gWetPar.Speed_30k;			//30km/h
			ar>>gWetPar.Max2AxleWet;		//车辆超重重量，按轴分
			ar>>gWetPar.Max3AxleWet;
			ar>>gWetPar.Max4AxleWet;
			ar>>gWetPar.Max5AxleWet;
			ar>>gWetPar.Max6AxleWet;
			ar>>gWetPar.MaxTrafficWet;			//红绿灯重量阈值
			ar>>gWetPar.DynamicK;	  			//动态系数
			ar>>gWetPar.PreAxle1_K;				//ad通道k系数
			ar>>gWetPar.PreAxle2_K;
			ar>>gWetPar.Scaler_K;
			ar>>gWetPar.BackAxle_K;
			ar>>gWetPar.Total_K;
			ar>>gWetPar.PreAxle1ZeroAD;			//ad通道ad零点
			ar>>gWetPar.PreAxle2ZeroAD;
			ar>>gWetPar.ScalerZeroAD;
			ar>>gWetPar.BackAxleZeroAD;

			//获取站
			CStation* pStation;
			pStation = GetStation();
			//if(pStation != NULL)
			//{
			//	CAlsStation* pAlsStation;
			//	pAlsStation = dynamic_cast<CAlsStation*>(pStation);
			//	if(pAlsStation != NULL)
			//	{
			//		pAlsStation->gWetPar = gWetPar;
			//	}
			//}
		}
	}

	CString str;
	int nEpInputCount;
	m_arrEpInput.RemoveAll();

	ar>>nEpInputCount;
	for(int i = 0; i < nEpInputCount; i++)
	{
		ar>>str;
		m_arrEpInput.Add(str);
	}
	
	return TRUE;
}

BOOL CZcsEp::StoreData(CArchive &ar)
{
	UINT nSize = size();

	ar<<nSize;

	for(UINT i = 0; i < nSize; i++)
	{
		ar<<at(i);
	}

	return TRUE;
}

BOOL CZcsEp::LoadData(CArchive &ar, UINT nFileVersion)
{
	UINT nSize;
	double f;

	ar>>nSize;
	for(UINT i = 0; i < nSize; i++)
	{
		ar>>f;
		push_back(f);
	}

	return TRUE;
}


void CZcsEp::OnNetMessage(UINT nID)
{
	nID -= ID_TREE_MSG_START;

	if(nID == 0)
	{
		CZcsDlg dlg;

		dlg.MaxWet  = gWetPar.MaxWet;
		dlg.PowerZero  = gWetPar.PowerZero;
		dlg.HandlZero  = gWetPar.HandlZero;
		dlg.ZeroTruckSpeed  = gWetPar.ZeroTruckSpeed;
		dlg.DynamicDivValue  = gWetPar.DynamicDivValue;
		dlg.StaticDivValue  = gWetPar.StaticDivValue;
		dlg.ZeroTruckRang  = gWetPar.ZeroTruckRang;
		dlg.SuperSpeedLevel  = gWetPar.SuperSpeedLevel;
		dlg.ZeroTruckTick  = gWetPar.ZeroTruckTick;
		dlg.Axle1UpWet  = gWetPar.PreAxleUpWet;
		dlg.Axle1DownWet  = gWetPar.PreAxleDownWet;
		dlg.Axle2UpWet  = gWetPar.BackAxleUpWet;
		dlg.Axle2DownWet  = gWetPar.BackAxleDownWet;
		dlg.TotalUpWet  = gWetPar.TotalUpWet;
		dlg.TotalDownWet  = gWetPar.TotalDownWet;
		dlg.Length  = gWetPar.Length;
		dlg.AxleLength  = gWetPar.AxleLength;
		dlg.Speed_0k  = 995;//gWetPar.Speed_0k;
		dlg.Speed_5k  = gWetPar.Speed_5k;
		dlg.Speed_10k  = gWetPar.Speed_10k;
		dlg.Speed_15k  = gWetPar.Speed_15k;
		dlg.Speed_20k  = gWetPar.Speed_20k;
		dlg.Speed_25k  = gWetPar.Speed_25k;
		dlg.Speed_30k  = gWetPar.Speed_30k;			//30km/h
		dlg.Max2AxleWet  = gWetPar.Max2AxleWet;		//车辆超重重量，按轴分
		dlg.Max3AxleWet  = gWetPar.Max3AxleWet;
		dlg.Max4AxleWet  = gWetPar.Max4AxleWet;
		dlg.Max5AxleWet  = gWetPar.Max5AxleWet;
		dlg.Max6AxleWet  = gWetPar.Max6AxleWet;
		dlg.MaxTrafficWet  = gWetPar.MaxTrafficWet;			//红绿灯重量阈值
		dlg.DynamicK  = gWetPar.DynamicK;	  			//动态系数
		dlg.PreAxle1_K  = gWetPar.PreAxle1_K;				//ad通道k系数
		dlg.PreAxle2_K  = gWetPar.PreAxle2_K;
		dlg.Scaler_K  = gWetPar.Scaler_K;
		dlg.BackAxle_K  = gWetPar.BackAxle_K;
		dlg.PreAxle1ZeroAD  = gWetPar.PreAxle1ZeroAD;			//ad通道ad零点
		dlg.PreAxle2ZeroAD  = gWetPar.PreAxle2ZeroAD;
		dlg.ScalerZeroAD  = gWetPar.ScalerZeroAD;
		dlg.BackAxleZeroAD  = gWetPar.BackAxleZeroAD;

		if(dlg.DoModal() == IDOK)
		{
			gWetPar.MaxWet = dlg.MaxWet;
			gWetPar.PowerZero = dlg.PowerZero;
			gWetPar.HandlZero = dlg.HandlZero;
			gWetPar.ZeroTruckSpeed = dlg.ZeroTruckSpeed;
			gWetPar.DynamicDivValue = dlg.DynamicDivValue;
			gWetPar.StaticDivValue = dlg.StaticDivValue;
			gWetPar.ZeroTruckRang = dlg.ZeroTruckRang;
			gWetPar.SuperSpeedLevel = dlg.SuperSpeedLevel;
			gWetPar.ZeroTruckTick = dlg.ZeroTruckTick;
			gWetPar.PreAxleUpWet = dlg.Axle1UpWet;
			gWetPar.PreAxleDownWet = dlg.Axle1DownWet;
			gWetPar.BackAxleUpWet = dlg.Axle2UpWet;
			gWetPar.BackAxleDownWet = dlg.Axle2DownWet;
			gWetPar.TotalUpWet = dlg.TotalUpWet;
			gWetPar.TotalDownWet = dlg.TotalDownWet;
			gWetPar.Length = dlg.Length;
			gWetPar.AxleLength = dlg.AxleLength;
			gWetPar.Speed_0k = 995;//dlg.Speed_0k;
			gWetPar.Speed_5k = dlg.Speed_5k;
			gWetPar.Speed_10k = dlg.Speed_10k;
			gWetPar.Speed_15k = dlg.Speed_15k;
			gWetPar.Speed_20k = dlg.Speed_20k;
			gWetPar.Speed_25k = dlg.Speed_25k;
			gWetPar.Speed_30k = dlg.Speed_30k;			//30km/h
			gWetPar.Max2AxleWet = dlg.Max2AxleWet;		//车辆超重重量，按轴分
			gWetPar.Max3AxleWet = dlg.Max3AxleWet;
			gWetPar.Max4AxleWet = dlg.Max4AxleWet;
			gWetPar.Max5AxleWet = dlg.Max5AxleWet;
			gWetPar.Max6AxleWet = dlg.Max6AxleWet;
			gWetPar.MaxTrafficWet = dlg.MaxTrafficWet;			//红绿灯重量阈值
			gWetPar.DynamicK = dlg.DynamicK;	  			//动态系数
			gWetPar.PreAxle1_K = dlg.PreAxle1_K;				//ad通道k系数
			gWetPar.PreAxle2_K = dlg.PreAxle2_K;
			gWetPar.Scaler_K = dlg.Scaler_K;
			gWetPar.BackAxle_K = dlg.BackAxle_K;
			gWetPar.PreAxle1ZeroAD = dlg.PreAxle1ZeroAD;			//ad通道ad零点
			gWetPar.PreAxle2ZeroAD = dlg.PreAxle2ZeroAD;
			gWetPar.ScalerZeroAD = dlg.ScalerZeroAD;
			gWetPar.BackAxleZeroAD = dlg.BackAxleZeroAD;

			CStation* pStation = GetStation();
			g_pStatin = pStation;
			//if(pStation != NULL)
			//{
			//	CAlsStation* pAlsStation;
			//	pAlsStation = dynamic_cast<CAlsStation*>(pStation);
			//	if(pAlsStation != NULL)
			//	{
			//		pAlsStation->gWetPar = gWetPar;
			//	}
			//}
		}		
	}

	//启用仿真
	else if(nID == 1)
	{
		m_bUseSim = !m_bUseSim;
	}

	//重新处理
	else if(nID == 2)
	{
		InitSample();
		clear();
		ClearOutPut();
		CarQueue_Init();
		BusinessTimerReset();
		Calc();	
		GetStation()->UpDateAllDoc();
	}

	//清除数据
	else if(nID == 3)
	{
		InitSample();
		clear();
		ClearOutPut();
		GetStation()->UpDateAllDoc();
	}

	//删除
	else if(nID == 4)
	{
		this->GetConnection()->RemoveEp(this);
	}
}

void CZcsEp::InitNetMenu(CMenu* pMenu)
{
	int nCount = 0;

	pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("参数设置"));
	nCount++;

	if(m_bUseSim == FALSE)
	{
		pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("启用仿真"));
	}
	else
	{
		pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("禁用仿真"));
	}
	nCount++;

	pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("重新处理"));
	nCount++;

	pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("清除数据"));
	nCount++;

	pMenu->AppendMenu(MF_BYCOMMAND, ID_TREE_MSG_START + nCount, _T("删除"));
	nCount++;

}

const CString CZcsEp::GetSymbol()
{
	//return _T("Zcs-V2_5");
	return _T("Zcs");
}

BOOL CZcsEp::OnProperty()
{
	CStringArray arrAll;
	CEpSelectDlg dlg;

	dlg.m_strName = GetName();

	this->GetStation()->ListEp(arrAll);

	dlg.FillEpStrAll(arrAll);
	dlg.FillSelectArr(m_arrEpInput);

	if(dlg.DoModal() == IDOK)
	{
		clear();
		ClearOutPut();

		SetName(dlg.m_strName);

		dlg.GetSelectArr(m_arrEpInput);

		InitSample();

		return TRUE;
	}
	
	return FALSE;
}

void CZcsEp::InitSample()
{
	m_Eps.clear();


	CEp* pEp;
	CStation* pStation;

	pStation = GetStation();
	g_pStatin = pStation;

	for(int i = 0; i < m_arrEpInput.GetCount(); i++)
	{
		pEp = pStation->GetEp(m_arrEpInput[i]);
		m_Eps.push_back(pEp);
	}

	ScalerDestory(hs_Main);

	//if(pStation != NULL)
	//{
	//	CAlsStation* pAlsStation;
	//	pAlsStation = dynamic_cast<CAlsStation*>(pStation);
	//	if(pAlsStation != NULL)
	//	{
	//		gWetPar = pAlsStation->gWetPar;
	//	}
	//}

	if(gWetPar.Scaler_K < 0.00001f) gWetPar.Scaler_K = 1.0f;

	//初始化轴速度修正
#if 1		//南充东
	gWetPar.Follow_0k = 1150;
	gWetPar.Follow_2P5k = 1120;
	gWetPar.Follow_5k = 1020;
	gWetPar.Follow_7P5k = 850;
	gWetPar.Follow_10k = 800;
	gWetPar.Follow_12P5k = 750;
	gWetPar.Follow_15k = 800;
	gWetPar.Follow_20k = 900;
	gWetPar.Follow_25k = 950;
	gWetPar.Follow_30k = 1000;
#else		//南充站
	gWetPar.Follow_0k = 1030;
	gWetPar.Follow_2P5k = 1035;
	gWetPar.Follow_5k = 1038;
	gWetPar.Follow_7P5k = 1020;
	gWetPar.Follow_10k = 868;
	gWetPar.Follow_12P5k = 860;
	gWetPar.Follow_15k = 960;
	gWetPar.Follow_20k = 1000;
	gWetPar.Follow_25k = 1020;
	gWetPar.Follow_30k = 1050;
#endif
	//gWetPar.Follow_0k = 1000;
	//gWetPar.Follow_2P5k = 1000;
	//gWetPar.Follow_5k = 1000;
	//gWetPar.Follow_7P5k = 1000;
	//gWetPar.Follow_10k = 1000;
	//gWetPar.Follow_12P5k = 1000;
	//gWetPar.Follow_15k = 1000;
	//gWetPar.Follow_20k = 1000;
	//gWetPar.Follow_25k = 1000;
	//gWetPar.Follow_30k = 1000;

	Weight_Init();
}

void CZcsEp::StopSample()
{

}

void CZcsEp::ClearOutPut()
{
	if(m_Eps.size() > MainScalerNormal) m_Eps[MainScalerNormal]->clear();
	if(m_Eps.size() > UpAlexNormal) m_Eps[UpAlexNormal]->clear();
	if(m_Eps.size() > BoardAlexNormal) m_Eps[BoardAlexNormal]->clear();
	if(m_Eps.size() > DownAlexNormal) m_Eps[DownAlexNormal]->clear();
	if(m_Eps.size() > UpAlexReg) m_Eps[UpAlexReg]->clear();
	//if(m_Eps.size() > BoardAlexReg) m_Eps[BoardAlexReg]->clear();
	//if(m_Eps.size() > DownAlexReg) m_Eps[DownAlexReg]->clear();
}

void CZcsEp::Calc()
{
	CStation* pStation;
	pStation = GetStation();
	float Scaler_Weight;

	long adbuf[4] = {0};

	//检查数据
	if (m_Eps.size() < 14)
	{
		return;
	}

	//锁定数据
	pStation->LockData();

	//if(pStation != NULL)
	//{
	//	CAlsStation* pAlsStation;
	//	pAlsStation = dynamic_cast<CAlsStation*>(pStation);
	//	if(pAlsStation != NULL)
	//	{
	//		gWetPar = pAlsStation->gWetPar;
	//		gWetPar.Speed_0k = 998;
	//	}
	//}

	//初始化确定输入数据长度
	unsigned int nDataCount = 0x7FFFFFFF;
	for(unsigned int i = 0; i < 7; i++ )
	{
		if(nDataCount > m_Eps[i]->GetDataCount())
		{
			nDataCount = m_Eps[i]->GetDataCount();
		}
	}

	if(nDataCount > m_Eps[BoardAlexReg]->GetDataCount())
	{
		nDataCount = m_Eps[BoardAlexReg]->GetDataCount();
	}

	if(nDataCount > m_Eps[DownAlexReg]->GetDataCount())
	{
		nDataCount = m_Eps[DownAlexReg]->GetDataCount();
	}


	fs = m_Eps[0]->fs;
	m_Eps[MainScalerNormal]->fs = m_Eps[0]->fs;
	m_Eps[UpAlexNormal]->fs = m_Eps[0]->fs;
	m_Eps[BoardAlexNormal]->fs = m_Eps[0]->fs;
	m_Eps[DownAlexNormal]->fs = m_Eps[0]->fs;
	m_Eps[UpAlexReg]->fs = m_Eps[0]->fs;
	m_Eps[BoardAlexReg]->fs = m_Eps[0]->fs;
	m_Eps[DownAlexReg]->fs = m_Eps[0]->fs;

	//数据处理
	for(unsigned int i = size(); (i < nDataCount); i++)
	{
		//信号处理====================================================
		adbuf[0] = (long)m_Eps[MainScalerAD]->at(i);
		adbuf[1] = (long)m_Eps[DownAlexAD]->at(i);
		adbuf[2] = (long)m_Eps[BoardAlexAD]->at(i);
		adbuf[3] = (long)m_Eps[UpAlexAD]->at(i);
		
		//称重业务
		Weight_Business(adbuf);


		//轮胎识别器处理
		sAxisType1_State_TypeDef axle1_status;	// = fRead_AxisType1State();
		axle1_status.Bit.EventFlag = (m_Eps[BoardAlexReg]->at(i) > 0) ? 1 : 0;
		axle1_status.Bit.SingleOrBoth = (m_Eps[DownAlexReg]->at(i) > 0) ? 1 : 0;
		axle1_status.Bit.ErrFlag = 0;
		AxleType_Business(&axle1_status);

		Scaler_Weight = (float)Get_Static_Weight();
		//Scaler_Weight = GetScalerStaticZeroTraceValue(hs_Main);

		//信号输出
		push_back(Scaler_Weight);
		//m_Eps[MainScalerNormal]->push_back(GetAdcChanleStandValue(hs_Main, 0));
		//m_Eps[DownAlexNormal]->push_back(GetAdcChanleStandValue(hs_Main, 1));
		//m_Eps[BoardAlexNormal]->push_back(GetAdcChanleStandValue(hs_Main, 2));
		//m_Eps[UpAlexNormal]->push_back(GetAdcChanleStandValue(hs_Main, 3));

		m_Eps[MainScalerNormal]->push_back(GetAdcChanleZerolizeValue(hs_Main, 0));
		m_Eps[DownAlexNormal]->push_back(GetAdcChanleZerolizeValue(hs_Main, 1));
		m_Eps[BoardAlexNormal]->push_back(GetAdcChanleZerolizeValue(hs_Main, 2));
		m_Eps[UpAlexNormal]->push_back(GetAdcChanleZerolizeValue(hs_Main, 3));

		//轴识别结果
		m_Eps[UpAlexReg]->push_back(SCS_ZC7_DyncProc.nUpFlag * 800);
		//m_Eps[DownAlexReg]->push_back(SCS_ZC7_DyncProc.nLeaveUpFlag * 800);

		if(m_bUseSim == TRUE)
		{
			//虚拟消息======================================================
			BusinessTimerTick();					//定时器
			LC2MakeMsg((int)m_Eps[LC2]->at(i));		//光幕
			GC1MakeMsg((int)m_Eps[GCFront]->at(i));		//前地感
			GC2MakeMsg((int)m_Eps[GCEnd]->at(i));		//后地感

			//流程处理======================================================
			Business_Thread_Adp(NULL);
		}
	}
	
	//解锁数据
	pStation->UnlockData();	
}

CEp* CZcsEp::Spawn()
{
	return new CZcsEp();
}




