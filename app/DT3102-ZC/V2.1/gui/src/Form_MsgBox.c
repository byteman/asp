#include "Form_MsgBox.h"


//========================================可输入数字的浮点数编辑窗体========================================
static const char* m_pTitle = NULL;
DEF_TEXT_LABEL(mMsgText, &g_MsgBoxDlg, 5, 40, 100, 14, CTRL_VISABLE, "");

static LPControl m_MsgBoxDlgControl[] = 
{
	&mMsgText,
};

void MsgBoxDlgDraw(LPWindow pWindow);
void MsgBoxDlgProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

CWindow g_MsgBoxDlg = 
{
	m_MsgBoxDlgControl,
	sizeof(m_MsgBoxDlgControl)/sizeof(LPControl),
	0,
	20,
	20,
	200,
	88,
	WS_VISABLE,
	NULL,
	6, 
	NULL, 
	NULL,
	MsgBoxDlgDraw,
	MsgBoxDlgProc
};

//打开编辑框设置窗口
void MsgBoxDlg(CWindow* pDlg, char* pTitle, char* pMsg)
{
	uint8 err;
	GuiMsgInfo guiMsgInfo;		//GUI消息结构实体

	//参数检查
	if(pMsg == NULL) return;

	m_pTitle = pTitle;	
	CTRL_CONTENT(mMsgText).s = pMsg;	 

	//设置父窗口
	pDlg->pParentWindow = g_pCurWindow;	

    g_pCurWindow = pDlg;
	//发送加载消息
	PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, 0);
	
	//消息循环
	while(TRUE)
	{
		GuiMsgQueuePend(&guiMsgInfo, &err);		//等待GUI队列消息
		if(err == SYS_NO_ERR) 
		{
			if(guiMsgInfo.ID == WM_RETURN)
			{
				g_pCurWindow = pDlg->pParentWindow;
				PostWindowMsg(g_pCurWindow, WM_SHOW, 0, 0);
				break;
			}
			else
			{
				pDlg->ProcFunc(pDlg, &guiMsgInfo);
			}
		}  
	}
}

void MsgBoxDlgDraw(LPWindow pWindow)
{
	uint16 i;
	int16 nStrSize;
	LPControl	lpControl;	  

	//禁止绘图
	SetRedraw(FALSE);

	//禁止刷屏
	EnableScreenFlush(FALSE);

	//设置视图
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

	//清除显存
	EraseRect(0, 0, pWindow->nViewSizeX, pWindow->nViewSizeY);

	//绘制窗口名称
	if(m_pTitle != NULL)
	{
		nStrSize = strlen((char*)m_pTitle);	
		if(nStrSize > 0)
		{		
			DrawGbText((char*)m_pTitle, 2, 2);
		}	
	}
	
	//禁止绘图
	//SetRedraw(FALSE);

	//禁止刷屏
	//EnableScreenFlush(FALSE);

	//绘制控件
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);

		if((lpControl->state & CTRL_VISABLE) != 0)
		{ 
			lpControl->DrawFunc(lpControl);
		}
	}

	//画窗体边框
	DrawRect(0, 0, pWindow->nViewSizeX - 1, pWindow->nViewSizeY - 1);
	DrawHoriLine(1, pWindow->nViewSizeY - 1, pWindow->nViewSizeX);
	DrawVertLine(pWindow->nViewSizeX - 1, 1, pWindow->nViewSizeY);

	//画分隔线
	DrawHoriLine(1, 18, pWindow->nViewSizeX); 

	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}



void MsgBoxDlgProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;
	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			pWindow->DrawFunc(pWindow);
			break;

		case WM_SHOW:
			pWindow->DrawFunc(pWindow);
			break;

		case WM_CLOSE:
			if(pWindow->pParentWindow != NULL)
			{
				//返回父窗口
				g_pCurWindow = pWindow->pParentWindow;
				PostWindowMsg(g_pCurWindow, WM_RETURN, 0, 0);
			}
			break;

		case WM_UPDATECTRL:
			pControl = (CControl*)(pGuiMsgInfo->wParam);
			if(pControl != NULL)
			{
				pControl->DrawFunc(pControl);
			} 			
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_BACK:			//返回
					PostWindowMsg(pWindow, WM_CLOSE, 0, 0);
					break;

				case KEY_OK:				//确定
					PostWindowMsg(pWindow, WM_CLOSE, 0, 0);
					break;

				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}

