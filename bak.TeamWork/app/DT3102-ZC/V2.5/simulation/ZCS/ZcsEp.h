#pragma once
#include "ep.h"


typedef enum _ZcsNode
{
	//input-----------------
	MainScalerAD = 0,
	DownAlexAD,
	BoardAlexAD,
	UpAlexAD,
	LC2,
	GCFront,
	GCEnd,

	//output----------------
	MainScalerNormal,
	DownAlexNormal,
	BoardAlexNormal,
	UpAlexNormal,
	DownAlexReg,
	BoardAlexReg,
	UpAlexReg
}ZcsNode;


class CZcsEp : public CEp
{
public:
	CZcsEp(void);
	virtual ~CZcsEp(void);

protected:
	CString m_strType;
	uint8 m_nType;
	int m_nTypeSize;
	BOOL m_bIsSignal;
	BOOL m_bUseSim;

	CStringArray m_arrEpInput;
	vector<CEp*> m_Eps;						//端点组




public:
	virtual CString GetTypeString();	//获取原始数据类型，字符串描述 
	virtual uint8 GetTypeCode();		//获取原始数据类型，类型码描述
	virtual int	GetTypeSize();			//获取原始数据类型的强度
	virtual BOOL IsSignal();			//是否为信号，非信号量只有单值，不存在向量数组

public:
	//重载至CTreeObject
	virtual BOOL OnDbClick();

	virtual void OnNetMessage(UINT nID);
	virtual void InitNetMenu(CMenu* pMenu);

public:
	virtual void InitSample();
	virtual void StopSample();
	virtual void Calc();
	virtual void SetZero();
	virtual void ResetZero();

public:
	void ClearOutPut();

public:
	virtual const CString GetSymbol();
	virtual BOOL OnProperty();	
	virtual CEp* Spawn();

public:
	virtual BOOL StoreConfig(CArchive &ar);
	virtual BOOL LoadConfig(CArchive &ar, UINT nFileVersion);
	virtual BOOL StoreData(CArchive &ar);
	virtual BOOL LoadData(CArchive &ar, UINT nFileVersion);
};

