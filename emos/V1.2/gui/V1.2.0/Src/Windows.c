#include "guiwindows.h"
#include "string.h"
#include "stdio.h"

#include "task_msg.h"


#define MsgQSize sizeof(TaskMsg)

CWindow*	g_pCurWindow;

GuiMsgInfo			m_GuiMsgInfoArr[GUIMSGSIZE];	   	//GUI消息信息存储区

uint8				GuiMsgInfoStart;					//消息起始索引
uint8				GuiMsgInfoEnd;						//消息停止索引

void*				m_GuiMsgPointArr[GUIMSGSIZE];	   	//GUI消息信息指针数组
SYS_EVENT 			*m_pEvtGuiMsg;					  	//GUI消息队列事件

SYS_MEM *GuiMem;
uint8 GuiMsgQBuf[8][50] = {0};	


void DefaultWindowDraw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;

	//清除显存
	EraseBuffer();

	//禁止绘图
	SetRedraw(FALSE);

	//禁止刷屏
	EnableScreenFlush(FALSE);

	//设置视图
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

	//绘制控件
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
			lpControl->DrawFunc(lpControl);
		}
	}

	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}

void DefaultWindowProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	
}

BOOL CreateWindowTimer(LPWindow lpWindow)
{
	uint8 err;

	if(lpWindow->tmrCallBack == NULL) return FALSE;
	if(lpWindow->pTimer != NULL) return TRUE;

	lpWindow->pTimer = SysTmrCreate(	lpWindow->period,
				 					lpWindow->period, 
									SYS_TMR_OPT_PERIODIC, 
									lpWindow->tmrCallBack, 
									lpWindow, 
									"", 
									&err);
	if(err == SYS_ERR_NONE)
	{
		return TRUE;
	}
	return FALSE;	
}

BOOL CreateWindowTimerEx(LPWindow lpWindow, uint32 delay)
{
	uint8 err;

	if (lpWindow->tmrCallBack == NULL) return FALSE;
	if (lpWindow->pTimer != NULL) return TRUE;

	lpWindow->pTimer = SysTmrCreate(delay, lpWindow->period, 
									SYS_TMR_OPT_PERIODIC, 
									lpWindow->tmrCallBack, 
									lpWindow, 
									"", 
									&err);
	if(err == SYS_ERR_NONE) {
		return TRUE;
	}
	return FALSE;
}

BOOL DestoryWindowTimer(LPWindow lpWindow)
{
	BOOL bResult;
	uint8 err;

	if(lpWindow->pTimer == NULL)  return TRUE;	
	
		
	bResult = SysTmrDel(lpWindow->pTimer, &err);

	lpWindow->pTimer = NULL;

	return bResult;
}

BOOL StartWindowTimer(LPWindow lpWindow)
{
	uint8 err;
	if(lpWindow->pTimer == NULL)  return FALSE;

	return	SysTmrStart(lpWindow->pTimer, &err);
}

BOOL StopWindowTimer(LPWindow lpWindow)
{
	uint8 err;
	if(lpWindow->pTimer == NULL) return FALSE;

	return	SysTmrStop(	lpWindow->pTimer,
						SYS_TMR_OPT_NONE,
						NULL,
	 					&err);
}

void DrawTextLabel(CControl* pControl)
{
	int nStrSize = 0;
	CTextLabel* pTextLabel;
	char*	s;
	pTextLabel =  (CTextLabel*)(pControl->pContent);
	s = pTextLabel->s;

	if(s != NULL)
	{
		nStrSize = strlen((char*)s);	
		if(nStrSize > 0)
		{		
			DrawGbText((char*)s, pControl->x, pControl->y);
			//DrawGbText(":", nStrSize*8, 0);
		}	
	}
}

void DrawSmallFloatLabel(CControl* pControl)
{
	int16 wText;
	char s[16], sPoint[10];

	CFloatLabel* pLabel = (CFloatLabel*)(pControl->pContent);

	//清除控件区域
	EnableScreenFlush(FALSE);

	SetColor(1);
	FillRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	SetColor(0);

	if ((pControl->state & CTRL_VISABLE) == 0) {
		sprintf(s, "");
	} else {
		if( *pLabel->pF > pLabel->max) {
			sprintf(s, "----");
		} else if(*pLabel->pF < pLabel->min) {
			sprintf(s, "----");
		} else {
			sprintf(sPoint, "%%.%df%s", pLabel->nFixPoint, pLabel->unit);
			sprintf(s, sPoint, *pLabel->pF);
		}
	}
	wText = strlen(s)*7;
	if(pControl->sx >= wText) {			//控件宽度大于文本宽度
	
		if ((pControl->align & 0x000F) == TA_LEFT) {
			DrawGbText(s, pControl->x, pControl->y); 
		} else if((pControl->align & 0x000F) == TA_CENTER) {
			DrawGbText(s, pControl->x + (pControl->sx - wText)/2, pControl->y); 
		} else if((pControl->align & 0x000F) == TA_RIGHT) {
			DrawGbText(s, pControl->x + (pControl->sx - wText), pControl->y); 
		}	
	} else {							//左对齐
		DrawGbText(s, pControl->x, pControl->y);
		//DrawSmallText(s, pControl->x, pControl->y);
	}
	//刷新控件区域
	EnableScreenFlush(TRUE);
	FlushRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	//FlushScreen();
}

void DrawBigFloatLabel(CControl* pControl)
{
	int16 wText;
	char s[16], sPoint[10];
	CFloatLabel* pLabel = (CFloatLabel*)(pControl->pContent);

	//清除控件区域
	EnableScreenFlush(FALSE);

	SetColor(1);
	FillRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	SetColor(0);

	if((*pLabel->pF > pLabel->max) || (*pLabel->pF < pLabel->min))
	{
		sprintf(s, "----");
	}
	else
	{
		sprintf(sPoint, "%%.%df%s", pLabel->nFixPoint, pLabel->unit);
		sprintf(s, sPoint, *pLabel->pF);
	}

	wText = strlen(s)*15;
	if(pControl->sx >= wText)			//控件宽度大于文本宽度
	{
		if((pControl->align & 0x000F) == TA_LEFT)
		{
			DrawBigText(s, pControl->x, pControl->y); 
		}
		else if((pControl->align & 0x000F) == TA_CENTER)
		{
			DrawBigText(s, pControl->x + (pControl->sx - wText)/2, pControl->y); 
		}
		else if((pControl->align & 0x000F) == TA_RIGHT)
		{
			DrawBigText(s, pControl->x + (pControl->sx - wText), pControl->y); 
		}
		
	}
	else							//左对齐
	{
		DrawBigText(s, pControl->x, pControl->y);
	}

	//刷新控件区域
	EnableScreenFlush(TRUE);
	FlushRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
}

void DrawStatusImage(CControl* pControl)
{
	CStatusImage* pImg = (CStatusImage*)(pControl->pContent);
	if(pImg->bHot == TRUE)
	{
		DrawImage(pImg->pHotImage, pControl->x, pControl->y, pImg->sx, pImg->sy);
	}
	else
	{
		DrawImage(pImg->pNormalImage, pControl->x, pControl->y, pImg->sx, pImg->sy);
	}
}

void DrawImageCtrl(CControl* pControl)
{
	CImageCtrl* pImg = (CImageCtrl*)(pControl->pContent);
 	DrawImage(pImg->pImage, pControl->x, pControl->y, pImg->sx, pImg->sy);	
	
//	LCD_ShowBMP(pControl->x, pControl->y, pImg->sx, pImg->sy, pImg->pImage);
}

void DrawCustomTextCtrl(CControl* pControl)
{	
	uint16 i;
	CCustomTextCtrl* pText = (CCustomTextCtrl*)(pControl->pContent);
	for(i = 0; i< pText->nCount; i++)
	{
		//暂不支持换行
		DrawImage(pText->pArr + i*pText->nWordSize, pControl->x + i*pText->sx, pControl->y, pText->sx, pText->sy);			
	}	
}

void DrawRectCtrl(CControl* pControl)
{
	CRectCtrl* pRect = (CRectCtrl*)(pControl->pContent);
	DrawRect(pControl->x, pControl->y, pRect->cx, pRect->cy);	
}

BOOL CreateCartoonCtrl(LPCartoonCtrl lpCartoonCtrl)
{
	uint8 err;

	if(lpCartoonCtrl->tmrCallBack == NULL) return FALSE;
	if(lpCartoonCtrl->pTimer != NULL) return TRUE;

	lpCartoonCtrl->pTimer = SysTmrCreate(lpCartoonCtrl->period,
					 					lpCartoonCtrl->period, 
										SYS_TMR_OPT_PERIODIC, 
										lpCartoonCtrl->tmrCallBack, 
										lpCartoonCtrl, 
										"", 
										&err);
	if(err == SYS_ERR_NONE)
	{
		return TRUE;
	}
	return FALSE;
}

BOOL DestoryCartoonCtrl(LPCartoonCtrl lpCartoonCtrl)
{
	uint8 err;
	if(lpCartoonCtrl->pTimer == NULL)  return TRUE;	
		
	return	SysTmrDel(lpCartoonCtrl->pTimer, &err);
}

BOOL StartCartoonCtrl(LPCartoonCtrl lpCartoonCtrl)
{
	uint8 err;
	if(lpCartoonCtrl->pTimer == NULL) return FALSE;

	return	SysTmrStart(lpCartoonCtrl->pTimer, &err);
}

BOOL StopCartoonCtrl(LPCartoonCtrl lpCartoonCtrl)
{
	BOOL bResult;
	uint8 err;

	if(lpCartoonCtrl->pTimer == NULL) return TRUE;

	bResult = SysTmrStop(	lpCartoonCtrl->pTimer,
						SYS_TMR_OPT_NONE,
						NULL,
	 					&err);

	lpCartoonCtrl->pTimer = NULL;

	return bResult;
}

void CartoonTmrCallBack(void *ptmr, void *parg)
{
	GuiMsgInfo msg;
	LPCartoonCtrl lpCartoonCtrl = (LPCartoonCtrl)parg;
	if(lpCartoonCtrl->cartoonState == CARTOON_STATE_ALTER)
	{
		if(lpCartoonCtrl->pControl->pParent == g_pCurWindow)
		{
			lpCartoonCtrl->cartoonState = CARTOON_STATE_ALTER;
			lpCartoonCtrl->nFrame++;
			if(lpCartoonCtrl->nFrame > 1) lpCartoonCtrl->nFrame = 0;

			msg.ID = WM_UPDATECTRL;

			//测试窗体移动
//			g_pCurWindow->nViewPosX += 11;
//			g_pCurWindow->nViewPosY += 9;	
//			if(g_pCurWindow->nViewPosX > 128) g_pCurWindow->nViewPosX = -128;
//			if(g_pCurWindow->nViewPosY > 64) g_pCurWindow->nViewPosY = -64;
//			msg.ID = WM_SHOW;
			//测试窗体完
			msg.pWindow = g_pCurWindow;
			msg.wParam = (uint32)(lpCartoonCtrl->pControl);
			GuiMsgQueuePost(&msg);
		}
	}
}

void DrawCartoonCtrl(CControl* pControl)
{
	CCartoonCtrl* pCartoon = (CCartoonCtrl*)(pControl->pContent);
	if(pCartoon->cartoonState == CARTOON_STATE_NORMAL)
	{
		DrawImage(pCartoon->pNormalImage, pControl->x, pControl->y, pCartoon->sx, pCartoon->sy);
	}
	else if(pCartoon->cartoonState == CARTOON_STATE_HOT)
	{
		DrawImage(pCartoon->pHotImage, pControl->x, pControl->y, pCartoon->sx, pCartoon->sy);
	}
	else if(pCartoon->cartoonState == CARTOON_STATE_ALTER)
	{
		if(pCartoon->nFrame == 0)
		{
			DrawImage(pCartoon->pNormalImage, pControl->x, pControl->y, pCartoon->sx, pCartoon->sy);
		}
		else
		{
			DrawImage(pCartoon->pHotImage, pControl->x, pControl->y, pCartoon->sx, pCartoon->sy);
		} 
	}		
}



void GuiMsgQueueCreate(void)
{
	uint8 err;
	
	GuiMsgInfoStart	= 0;
	GuiMsgInfoEnd	= 0;

	GuiMem = SysMemCreate(GuiMsgQBuf, 8, 50, &err);
	m_pEvtGuiMsg = SysQCreate(&m_GuiMsgPointArr[0], GUIMSGSIZE);	
}
uint8 *Get_GuiMemCpy(uint8 *buff, u32 len)
{
	uint8 err;
	uint8 *pBuf;	
	
	pBuf = (uint8 *)SysMemGet(GuiMem, &err);
	if (pBuf == NULL) return NULL;
	memset(pBuf, 0, 50);
	memcpy(pBuf, buff, len);
	
	return pBuf;
}

void Put_GuiMem(uint8 *buff)
{
	if (buff == NULL)	return;
	SysMemPut(GuiMem, buff);
}

uint8 GuiMsgQueuePost(GuiMsgInfo* pGuiMsgInfo)
{
	uint8 tmp;


	if (m_pEvtGuiMsg == NULL) return SYS_ERR_Q_EMPTY;

	if ((GuiMsgInfoEnd+1)%GUIMSGSIZE == GuiMsgInfoStart)
	{
		return SYS_ERR_Q_FULL;	//队列满，返回值暂借用SYS_ERR_Q_FULL   
	}
	
	memcpy(&m_GuiMsgInfoArr[GuiMsgInfoEnd], pGuiMsgInfo, sizeof(GuiMsgInfo));

	tmp = GuiMsgInfoEnd;
	GuiMsgInfoEnd = (GuiMsgInfoEnd + 1)%GUIMSGSIZE;	 

	return SysQPost(m_pEvtGuiMsg, (void*)&m_GuiMsgInfoArr[tmp]);	
}

void* GuiMsgQueuePend(GuiMsgInfo* pGuiMsgInfo, uint8* err)
{
	void* ret;
	
	ret = SysQPend(m_pEvtGuiMsg, 0, err);						//永远等待
	GuiMsgInfoStart = (GuiMsgInfoStart + 1) % GUIMSGSIZE;
	memcpy(pGuiMsgInfo, (GuiMsgInfo*)ret, sizeof(GuiMsgInfo));
	
	Put_GuiMem((uint8 *)pGuiMsgInfo->lParam);
	
	return ret;
}

//发送窗口消息
uint8 PostWindowMsg(LPWindow pWindow, uint16 nID, uint32 wParam, uint32 lParam)
{
	GuiMsgInfo msg;

	msg.pWindow = pWindow;
	msg.ID = nID;
	msg.wParam = wParam;
	msg.lParam = lParam;

	return GuiMsgQueuePost(&msg);	
}



