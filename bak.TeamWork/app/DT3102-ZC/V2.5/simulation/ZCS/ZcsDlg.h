#pragma once

#include "resource.h"
// CZcsDlg 对话框

class CZcsDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CZcsDlg)

public:
	CZcsDlg(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CZcsDlg();

// 对话框数据
	enum { IDD = IDD_ZCS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	UINT MaxWet;
	BYTE HandlZero;
	BYTE ZeroTruckSpeed;
	BYTE DynamicDivValue;
	BYTE StaticDivValue;
	BYTE ZeroTruckRang;
	afx_msg void OnBnClickedOk();
	UINT ZeroTruckTick;
	BYTE PowerZero;
	float ScalerZeroAD;
	float Scaler_K;
	UINT TotalUpWet;
	UINT TotalDownWet;
	UINT Axle1UpWet;
	UINT Axle1DownWet;
	UINT Axle2UpWet;
	UINT Axle2DownWet;
	UINT Axle3UpWet;
	UINT Axle3DownWet;
	float PreAxle1ZeroAD;
	float PreAxle1_K;
	UINT Speed_0k;
	UINT Speed_5k;
	UINT Speed_10k;
	UINT Speed_15k;
	UINT Speed_20k;
	UINT Speed_25k;
	UINT Speed_30k;
	float BackAxleZeroAD;
	float BackAxle_K;
	float PreAxle2ZeroAD;
	float PreAxle2_K;
	float DynamicK;
	UINT Max2AxleWet;
	UINT Max3AxleWet;
	UINT Max4AxleWet;
	UINT Max5AxleWet;
	UINT Max6AxleWet;
	UINT MaxTrafficWet;
	UINT Length;
	UINT AxleLength;
	BYTE SuperSpeedLevel;
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk3();
	afx_msg void OnBnClickedOk2();
};
