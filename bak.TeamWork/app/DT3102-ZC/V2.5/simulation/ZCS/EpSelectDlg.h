#pragma once
#include "afxwin.h"
#include "afxdialogex.h"
#include "Resource.h"

// CEpSelectDlg dialog
class CStation;
class CEpSelectDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CEpSelectDlg)

public:
	CEpSelectDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CEpSelectDlg();

// Dialog Data
	enum { IDD = IDD_NODE_SELCET_DLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();

protected:
	CListBox m_lbUnSelect;
	CListBox m_lbSelect;
	CComboBox m_cbDataFile;
	CString m_strCurDataFile;
	BOOL	m_IsChanged;

	CStringArray m_arrSelect;
	CStringArray m_arrUnSelect;
	CStringArray m_arrEpStrAll;

public:
	void FillEpStrAll(CStringArray& arr);
	void FillSelectArr(CStringArray& arr);
	void GetSelectArr(CStringArray& arr);
	BOOL IsSelect(CString str);

public:
	afx_msg void OnBnClickedButtonAddnote();
	afx_msg void OnBnClickedButtonRemovenote();
	afx_msg void OnBnClickedCancel();
	BOOL IsChanged();
	

public:
	virtual BOOL OnInitDialog();
	void ClearListBox(CListBox* pListBox);
	BOOL FindInSelectBox(CString strName);

private:
	void InitListBox(CString strDataFile = _T(""));
	void NodeTrans(CListBox* pSrc, CListBox* pDes, int nMaxTrans = 1000);

public:
	void InitComboList(CComboBox* pComboBox);
	void SelectComboStr(CComboBox* pComboBox, CString str);

public:
	afx_msg void OnBnClickedApplyNode();

	CString m_strName;
	CComboBox m_cnMainScalerInput;
	afx_msg void OnCbnSelchangeCombo1();
	CComboBox m_cbUpAlexAD;
	CComboBox m_cbDownAlexAD;
	CComboBox m_cbBoradAlexAD;
	CComboBox m_cdLC2;
	CComboBox m_cbGCFront;
	CComboBox m_cbGCEnd;
	CComboBox m_cbMainNornal;
	CComboBox m_cbUpAlexNormal;
	CComboBox m_cbBoradAlexNormal;
	CComboBox m_cbDownAlexNormal;
	CComboBox m_cbUpAlexReg;
	CComboBox m_cbBoardAlexReg;
	CComboBox m_cbDownAlexReg;
};
