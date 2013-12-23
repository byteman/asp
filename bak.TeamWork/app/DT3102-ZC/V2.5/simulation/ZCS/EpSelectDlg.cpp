#include "stdafx.h"
#include "EpSelectDlg.h"
#include "StrFileName.h"
#include "DocInfo.h"
#include "afxdialogex.h"
#include "Station.h"
#include "ZcsEp.h"

// CEpSelectDlg dialog

IMPLEMENT_DYNAMIC(CEpSelectDlg, CDialogEx)

CEpSelectDlg::CEpSelectDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CEpSelectDlg::IDD, pParent)
	, m_strName(_T(""))
{
	m_IsChanged = FALSE;
}

BOOL CEpSelectDlg::IsChanged()
{
	return m_IsChanged;
}

void CEpSelectDlg::ClearListBox(CListBox* pListBox)
{
	int i;
	for( i = pListBox->GetCount() - 1; i >= 0; i--)
	{
		pListBox->DeleteString(i);
	}
}

CEpSelectDlg::~CEpSelectDlg()
{

}

void CEpSelectDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT1, m_strName);
	DDX_Control(pDX, IDC_COMBO1, m_cnMainScalerInput);
	DDX_Control(pDX, IDC_COMBO2, m_cbUpAlexAD);
	DDX_Control(pDX, IDC_COMBO4, m_cbDownAlexAD);
	DDX_Control(pDX, IDC_COMBO3, m_cbBoradAlexAD);
	DDX_Control(pDX, IDC_COMBO5, m_cdLC2);
	DDX_Control(pDX, IDC_COMBO6, m_cbGCFront);
	DDX_Control(pDX, IDC_COMBO7, m_cbGCEnd);
	DDX_Control(pDX, IDC_COMBO8, m_cbMainNornal);
	DDX_Control(pDX, IDC_COMBO9, m_cbUpAlexNormal);
	DDX_Control(pDX, IDC_COMBO10, m_cbBoradAlexNormal);
	DDX_Control(pDX, IDC_COMBO11, m_cbDownAlexNormal);
	DDX_Control(pDX, IDC_COMBO12, m_cbUpAlexReg);
	DDX_Control(pDX, IDC_COMBO13, m_cbBoardAlexReg);
	DDX_Control(pDX, IDC_COMBO14, m_cbDownAlexReg);
}

BEGIN_MESSAGE_MAP(CEpSelectDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CEpSelectDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_ADDNOTE, &CEpSelectDlg::OnBnClickedButtonAddnote)
	ON_BN_CLICKED(IDC_BUTTON_REMOVENOTE, &CEpSelectDlg::OnBnClickedButtonRemovenote)
	ON_BN_CLICKED(IDCANCEL, &CEpSelectDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_APPLY_NODE, &CEpSelectDlg::OnBnClickedApplyNode)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CEpSelectDlg::OnCbnSelchangeCombo1)
END_MESSAGE_MAP()



void CEpSelectDlg::InitListBox(CString strDataFile)
{
	strDataFile = strDataFile;

	//初始化已添加节点列表
	CString strNode;
	ClearListBox(&m_lbSelect);		//清除
	for(int i = 0; i < m_arrSelect.GetCount(); i++)
	{
		m_lbSelect.AddString(m_arrSelect[i]);
	}

	//测点个数
	int nEpCountAll = m_arrEpStrAll.GetCount();

	//初始化空余测点列表
	ClearListBox(&m_lbUnSelect);				//清除
	for(int i = 0; i < nEpCountAll; i++)
	{
		if(IsSelect(m_arrEpStrAll[i]) == FALSE)
		{
			m_lbUnSelect.AddString(m_arrEpStrAll[i]);
		}
	}
}

//应用
void CEpSelectDlg::OnBnClickedApplyNode()
{
	CString str;
	UpdateData();

	m_arrSelect.RemoveAll();

	m_cnMainScalerInput.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbDownAlexAD.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbBoradAlexAD.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbUpAlexAD.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cdLC2.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbGCFront.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbGCEnd.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbMainNornal.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbDownAlexNormal.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbBoradAlexNormal.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbUpAlexNormal.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbDownAlexReg.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbBoardAlexReg.GetWindowText(str);
	m_arrSelect.Add(str);

	m_cbUpAlexReg.GetWindowText(str);
	m_arrSelect.Add(str);


	//添加
	for(int i = 0; i < m_arrSelect.GetCount(); i++)
	{
		if(m_arrSelect[i] == _T(""))
		{
			m_IsChanged = FALSE;
			return;
		}
	}

	m_IsChanged = TRUE;
}

//在选择框里查找名称
BOOL CEpSelectDlg::FindInSelectBox(CString strName)
{
	CString strText;

	for(int i = 0; i < m_lbSelect.GetCount(); i++)
	{
		m_lbSelect.GetText(i, strText);
		if(strName == strText) return TRUE;
	}

	return FALSE;
}

void CEpSelectDlg::OnBnClickedOk()
{
	OnBnClickedApplyNode();

	if(m_IsChanged == FALSE)
	{
		MessageBox(_T("不能有空余节点"));
		return;
	}

	OnOK();
}

void CEpSelectDlg::OnBnClickedButtonAddnote()
{
	NodeTrans(&m_lbUnSelect, &m_lbSelect);
}

void CEpSelectDlg::OnBnClickedButtonRemovenote()
{
	NodeTrans(&m_lbSelect, &m_lbUnSelect);
}

void CEpSelectDlg::NodeTrans(CListBox* pSrc, CListBox* pDes, int nMaxTrans)
{
	int nSrcSelCount = pSrc->GetSelCount();
	if(nSrcSelCount == 0) return;

	int i, si;
	CString strNode;
	for(i = pSrc->GetCount() - 1, si = 0; i >= 0; i--)
	{
		if(pSrc->GetSel(i) > 0)
		{
			pSrc->GetText(i, strNode);
			pDes->AddString(strNode);
			pSrc->DeleteString(i);

			si++;

			if(si >= nSrcSelCount) return;
			if(si >= nMaxTrans) return;
		}
	}
}

void CEpSelectDlg::OnBnClickedCancel()
{
	OnCancel();
}

void CEpSelectDlg::InitComboList(CComboBox* pComboBox)
{
	//初始化主秤选择表
	pComboBox->Clear();

	//测点个数
	int nEpCountAll = m_arrEpStrAll.GetCount();

	//初始化空余测点列表
	for(int i = 0; i < nEpCountAll; i++)
	{
		pComboBox->AddString(m_arrEpStrAll[i]);
	}
}

void CEpSelectDlg::SelectComboStr(CComboBox* pComboBox, CString str)
{
	int nSel = -1;

	nSel = pComboBox->FindString(0, str);
	if(nSel >= 0)
	{
		pComboBox->SetCurSel(nSel);
	}
}

BOOL CEpSelectDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitComboList(&m_cnMainScalerInput);
	InitComboList(&m_cbUpAlexAD);
	InitComboList(&m_cbDownAlexAD);
	InitComboList(&m_cbBoradAlexAD);
	InitComboList(&m_cdLC2);
	InitComboList(&m_cbGCFront);
	InitComboList(&m_cbGCEnd);
	InitComboList(&m_cbMainNornal);
	InitComboList(&m_cbUpAlexNormal);
	InitComboList(&m_cbBoradAlexNormal);
	InitComboList(&m_cbDownAlexNormal);
	InitComboList(&m_cbUpAlexReg);
	InitComboList(&m_cbBoardAlexReg);
	InitComboList(&m_cbDownAlexReg);

	CString str;
	if(m_arrSelect.GetCount() > 0)
		SelectComboStr(&m_cnMainScalerInput, m_arrSelect[0]);

	if(m_arrSelect.GetCount() > 1)
		SelectComboStr(&m_cbDownAlexAD, m_arrSelect[1]);

	if(m_arrSelect.GetCount() > 2)
		SelectComboStr(&m_cbBoradAlexAD, m_arrSelect[2]);

	if(m_arrSelect.GetCount() > 3)
		SelectComboStr(&m_cbUpAlexAD, m_arrSelect[3]);

	if(m_arrSelect.GetCount() > 4)
		SelectComboStr(&m_cdLC2, m_arrSelect[4]);

	if(m_arrSelect.GetCount() > 5)
		SelectComboStr(&m_cbGCFront, m_arrSelect[5]);

	if(m_arrSelect.GetCount() > 6)
		SelectComboStr(&m_cbGCEnd, m_arrSelect[6]);

	if(m_arrSelect.GetCount() > 7)
		SelectComboStr(&m_cbMainNornal, m_arrSelect[7]);

	if(m_arrSelect.GetCount() > 8)
		SelectComboStr(&m_cbDownAlexNormal, m_arrSelect[8]);

	if(m_arrSelect.GetCount() > 9)
		SelectComboStr(&m_cbBoradAlexNormal, m_arrSelect[9]);

	if(m_arrSelect.GetCount() > 10)
		SelectComboStr(&m_cbUpAlexNormal, m_arrSelect[10]);

	if(m_arrSelect.GetCount() > 11)
		SelectComboStr(&m_cbDownAlexReg, m_arrSelect[11]);

	if(m_arrSelect.GetCount() > 12)
		SelectComboStr(&m_cbBoardAlexReg, m_arrSelect[12]);

	if(m_arrSelect.GetCount() > 13)
		SelectComboStr(&m_cbUpAlexReg, m_arrSelect[13]);

	return TRUE;
}

void CEpSelectDlg::FillSelectArr(CStringArray& arr)
{
	m_arrSelect.RemoveAll();

	for(int i = 0; i < arr.GetCount(); i++)
	{
		m_arrSelect.Add(arr[i]);
	}
}

void CEpSelectDlg::FillEpStrAll(CStringArray& arr)
{
	m_arrEpStrAll.RemoveAll();

	for(int i = 0; i < arr.GetCount(); i++)
	{
		m_arrEpStrAll.Add(arr[i]);
	}
}

void CEpSelectDlg::GetSelectArr(CStringArray& arr)
{
	arr.RemoveAll();

	for(int i = 0; i < m_arrSelect.GetCount(); i++)
	{
		arr.Add(m_arrSelect[i]);
	}
}

BOOL CEpSelectDlg::IsSelect(CString str)
{
	for(int i = 0; i < m_arrSelect.GetCount(); i++)
	{
		if(m_arrSelect[i] == str) return TRUE;
	}
	
	return FALSE;
}



//主秤输入
void CEpSelectDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
}
