#include "Form_TimeDate.h"
#include "Form_Main.h"
#include "Ctr_Attr.h"
#include "Form_TimeSet.h"
#include "Form_DateSet.h"
//#include "rtc.h"
#include "includes.h"
#include "app_rtc.h"

extern  CControl  gStatusBar; 		//状态栏

static void Form_TimeDate_Timer(void *ptmr, void *parg);
static void Form_TimeDate_Draw(LPWindow pWindow);
static void Form_TimeDate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


static char stime[6];
static char sdate[30];
static CAttrItem  _attritem[] = {
   {"时间",stime,1},
   {"日期",sdate,2},
};


DEF_ATTR_CTRL(mTimeAttr, &gWD_TimeDate, 0, 20, 240, 108, "时间与日期",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mTimeAttr,
};

CWindow  gWD_TimeDate = {
    marrLPControl,
	sizeof(marrLPControl)/sizeof(LPControl),
    0,
    0,
    0,
    240,
    128,
    WS_VISABLE,
	NULL,//(pParentWindow)
    10,//5
    NULL,
    Form_TimeDate_Timer,
    Form_TimeDate_Draw,
    Form_TimeDate_Proc
};

// void LoadSetDlg(CWindow* pDlg)
// {
// 	uint8 err;
// 	GuiMsgInfo guiMsgInfo;		//GUI消息结构实体

// 	//设置父窗口
// 	pDlg->pParentWindow = g_pCurWindow;	

// 	//发送加载消息
// 	PostWindowMsg(WM_LOAD, NULL, 0);
// 	
// 	//消息循环
// 	while(TRUE)
// 	{
// 		GuiMsgQueuePend(&guiMsgInfo, &err);		//等待GUI队列消息
// 		if(err == SYS_NO_ERR) 
// 		{
// 			if(guiMsgInfo.ID == WM_RETURN)
// 			{
// 				g_pCurWindow = pDlg->pParentWindow;
// 				PostWindowMsg(WM_SHOW, 0, 0);
// 				break;
// 			}
// 			else
// 			{
// 				pDlg->ProcFunc(pDlg, &guiMsgInfo);
// 			}
// 		}  
// 	}	
// }

void Form_TimeDate_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_TimeDate_Draw(LPWindow pWindow)
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
void Form_TimeDate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
	static CSysTime* time = NULL;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
//    		GetSysTime(&time);
			time = Get_System_Time();
    		sprintf(stime, "%02d:%02d", time->hour, time->min);		
    		sprintf(sdate, "%04d-%02d-%02d", time->year, time->mon, time->day);		
    		//sprintf(sdate, "%04d-%02d-%02d    星期%s", time.Year, time.Month, time.Day,WeekDays[time.Week]);		
			//CreateWindowTimerEx(pWindow, 1);
			//StartWindowTimer(pWindow);		//启动主窗体定时器
		case WM_SHOW:
			pWindow->DrawFunc(pWindow);
			break;

		case WM_UPDATECTRL:
			pControl = (CControl*)(pGuiMsgInfo->wParam);
			if(pControl != NULL)
			{
				pControl->DrawFunc(pControl);
			} 			
			break;

		case WM_TIMEUPDATE:
			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&gStatusBar, 0);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_UP:
                    if (CTRL_CONTENT(mTimeAttr).focus > 0) {
                        CTRL_CONTENT(mTimeAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mTimeAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mTimeAttr).focus + 1 < CTRL_CONTENT(mTimeAttr).total) {
                        CTRL_CONTENT(mTimeAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mTimeAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
                    if (CTRL_CONTENT(mTimeAttr).focus == 0) {
                        gWD_TimeSet.pParentWindow = g_pCurWindow;
                        g_pCurWindow = &gWD_TimeSet;
						guiMsgInfo.pWindow = g_pCurWindow;
    					guiMsgInfo.ID = WM_LOAD;
    					GuiMsgQueuePost(&guiMsgInfo);
                    }else {
                        gWD_DateSet.pParentWindow = g_pCurWindow;
                        g_pCurWindow = &gWD_DateSet;
						guiMsgInfo.pWindow = g_pCurWindow;
    					guiMsgInfo.ID = WM_LOAD;
    					GuiMsgQueuePost(&guiMsgInfo);
                    }
					break;

				case KEY_RIGHT: 
					break;

				case KEY_BACK:
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = pWindow->pParentWindow;
					}
					else
					{
						g_pCurWindow = &gWD_Main;
					}
                    PostWindowMsg(g_pCurWindow, WM_LOAD,0,0);
					break;

				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}



