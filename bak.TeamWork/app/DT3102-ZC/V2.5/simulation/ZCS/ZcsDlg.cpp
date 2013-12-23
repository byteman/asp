// ZcsDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ZcsDlg.h"
#include "afxdialogex.h"
#include <iostream>

using namespace std;
// CZcsDlg 对话框

IMPLEMENT_DYNAMIC(CZcsDlg, CDialogEx)

CZcsDlg::CZcsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CZcsDlg::IDD, pParent)
	, MaxWet(0)
	, HandlZero(0)
	, ZeroTruckSpeed(0)
	, DynamicDivValue(0)
	, StaticDivValue(0)
	, ZeroTruckRang(0)
	, ZeroTruckTick(0)
	, PowerZero(0)
	, ScalerZeroAD(0)
	, Scaler_K(0)
	, TotalUpWet(0)
	, TotalDownWet(0)
	, Axle1UpWet(0)
	, Axle1DownWet(0)
	, Axle2UpWet(0)
	, Axle2DownWet(0)
	, Axle3UpWet(0)
	, Axle3DownWet(0)
	, PreAxle1ZeroAD(0)
	, PreAxle1_K(0)
	, Speed_5k(0)
	, Speed_10k(0)
	, Speed_15k(0)
	, Speed_20k(0)
	, Speed_25k(0)
	, Speed_30k(0)
	, BackAxleZeroAD(0)
	, BackAxle_K(0)
	, PreAxle2ZeroAD(0)
	, PreAxle2_K(0)
	, DynamicK(0)
	, Max2AxleWet(0)
	, Max3AxleWet(0)
	, Max4AxleWet(0)
	, Max5AxleWet(0)
	, Max6AxleWet(0)
	, MaxTrafficWet(0)
	, Length(0)
	, AxleLength(0)
	, SuperSpeedLevel(0)
{

}

CZcsDlg::~CZcsDlg()
{
}

void CZcsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, MaxWet);
	DDX_Text(pDX, IDC_EDIT2, PowerZero);
	DDX_Text(pDX, IDC_EDIT3, HandlZero);
	DDX_Text(pDX, IDC_EDIT4, ZeroTruckSpeed);
	DDX_Text(pDX, IDC_EDIT5, DynamicDivValue);
	DDX_Text(pDX, IDC_EDIT6, StaticDivValue);
	DDX_Text(pDX, IDC_EDIT7, ZeroTruckRang);
	DDX_Text(pDX, IDC_EDIT8, ZeroTruckTick);


	DDX_Text(pDX, IDC_EDIT9, ScalerZeroAD);
	DDX_Text(pDX, IDC_EDIT13, Scaler_K);
	DDX_Text(pDX, IDC_EDIT17, TotalUpWet);
	DDX_Text(pDX, IDC_EDIT21, TotalDownWet);
	DDX_Text(pDX, IDC_EDIT22, Axle1UpWet);
	DDX_Text(pDX, IDC_EDIT18, Axle1DownWet);
	DDX_Text(pDX, IDC_EDIT19, Axle2UpWet);
	DDX_Text(pDX, IDC_EDIT24, Axle2DownWet);
	DDX_Text(pDX, IDC_EDIT23, Axle3UpWet);
	DDX_Text(pDX, IDC_EDIT20, Axle3DownWet);
	DDX_Text(pDX, IDC_EDIT14, PreAxle1ZeroAD);
	DDX_Text(pDX, IDC_EDIT10, PreAxle1_K);
	DDX_Text(pDX, IDC_EDIT28, Speed_5k);
	DDX_Text(pDX, IDC_EDIT25, Speed_10k);
	DDX_Text(pDX, IDC_EDIT26, Speed_15k);
	DDX_Text(pDX, IDC_EDIT30, Speed_20k);
	DDX_Text(pDX, IDC_EDIT29, Speed_25k);
	DDX_Text(pDX, IDC_EDIT27, Speed_30k);
	DDX_Text(pDX, IDC_EDIT11, BackAxleZeroAD);
	DDX_Text(pDX, IDC_EDIT16, BackAxle_K);
	DDX_Text(pDX, IDC_EDIT15, PreAxle2ZeroAD);
	DDX_Text(pDX, IDC_EDIT12, PreAxle2_K);



	DDX_Text(pDX, IDC_EDIT31, DynamicK);
	DDX_Text(pDX, IDC_EDIT35, Max2AxleWet);
	DDX_Text(pDX, IDC_EDIT32, Max3AxleWet);
	DDX_Text(pDX, IDC_EDIT33, Max4AxleWet);
	DDX_Text(pDX, IDC_EDIT37, Max5AxleWet);
	DDX_Text(pDX, IDC_EDIT36, Max6AxleWet);
	DDX_Text(pDX, IDC_EDIT34, MaxTrafficWet);
	DDX_Text(pDX, IDC_EDIT40, Length);
	DDX_Text(pDX, IDC_EDIT38, AxleLength);
	DDX_Text(pDX, IDC_EDIT39, SuperSpeedLevel);
}


BEGIN_MESSAGE_MAP(CZcsDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CZcsDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CZcsDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK3, &CZcsDlg::OnBnClickedOk3)
	ON_BN_CLICKED(IDOK2, &CZcsDlg::OnBnClickedOk2)
END_MESSAGE_MAP()


// CZcsDlg 消息处理程序


void CZcsDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CZcsDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码

	CDialogEx::OnCancel();
}

extern "C" 
{
#include "includes.h"
#include "weight_param.h"
	extern gWetParam gWetPar;
};



void CZcsDlg::OnBnClickedOk3()
{

	FILE* fp = fopen("param.cfg","r");
	if (fp)
	{
		fread((void*)&gWetPar,sizeof(gWetPar),1,fp);
		fclose(fp);
		MaxWet  = gWetPar.MaxWet;
		PowerZero  = gWetPar.PowerZero;
		HandlZero  = gWetPar.HandlZero;
		ZeroTruckSpeed  = gWetPar.ZeroTruckSpeed;
		DynamicDivValue  = gWetPar.DynamicDivValue;
		StaticDivValue  = gWetPar.StaticDivValue;
		ZeroTruckRang  = gWetPar.ZeroTruckRang;
		SuperSpeedLevel  = gWetPar.SuperSpeedLevel;
		ZeroTruckTick  = gWetPar.ZeroTruckTick;
		Axle1UpWet  = gWetPar.PreAxleUpWet;
		Axle1DownWet  = gWetPar.PreAxleDownWet;
		Axle2UpWet  = gWetPar.BackAxleUpWet;
		Axle2DownWet  = gWetPar.BackAxleDownWet;
		TotalUpWet  = gWetPar.TotalUpWet;
		TotalDownWet  = gWetPar.TotalDownWet;
		Length  = gWetPar.Length;
		AxleLength  = gWetPar.AxleLength;
		Speed_0k  = 995;//gWetPar.Speed_0k;
		Speed_5k  = gWetPar.Speed_5k;
		Speed_10k  = gWetPar.Speed_10k;
		Speed_15k  = gWetPar.Speed_15k;
		Speed_20k  = gWetPar.Speed_20k;
		Speed_25k  = gWetPar.Speed_25k;
		Speed_30k  = gWetPar.Speed_30k;			//30km/h
		Max2AxleWet  = gWetPar.Max2AxleWet;		//车辆超重重量，按轴分
		Max3AxleWet  = gWetPar.Max3AxleWet;
		Max4AxleWet  = gWetPar.Max4AxleWet;
		Max5AxleWet  = gWetPar.Max5AxleWet;
		Max6AxleWet  = gWetPar.Max6AxleWet;
		MaxTrafficWet  = gWetPar.MaxTrafficWet;			//红绿灯重量阈值
		DynamicK  = gWetPar.DynamicK;	  			//动态系数
		PreAxle1_K  = gWetPar.PreAxle1_K;				//ad通道k系数
		PreAxle2_K  = gWetPar.PreAxle2_K;
		Scaler_K  = gWetPar.Scaler_K;
		BackAxle_K  = gWetPar.BackAxle_K;
		PreAxle1ZeroAD  = gWetPar.PreAxle1ZeroAD;			//ad通道ad零点
		PreAxle2ZeroAD  = gWetPar.PreAxle2ZeroAD;
		ScalerZeroAD  = gWetPar.ScalerZeroAD;
		BackAxleZeroAD  = gWetPar.BackAxleZeroAD;
		UpdateData(FALSE);
	}
	//SetDlgItemText(IDC_EDIT1,CvtString(max));

	// TODO: 在此添加控件通知处理程序代码
}


void CZcsDlg::OnBnClickedOk2()
{
	FILE* fp = fopen("param.cfg","w+");
	if (fp)
	{
		fwrite((void*)&gWetPar,sizeof(gWetPar),1,fp);
		fclose(fp);
	}
#if 0
	fstream file;

	file.open(_T("param.cfg"), ios_base::in | ios_base::trunc);

	if(file.fail())
	{
		AfxMessageBox(_T("can't open param.cfg!"));
		return;
	}

	file.write((const char *)&gWetPar, sizeof(gWetPar));
	file.close();
#endif
	// TODO: 在此添加控件通知处理程序代码
}
