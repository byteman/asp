#include "Form_VoiceSet.h"
#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "Form_Main.h"
// #include "SysParam.h"
#include "Ctr_Statusbar.h"
#include "sys_param.h"

#include "app_beep.h"

extern  CControl  gStatusBar; 		//×´Ì¬À¸
extern 	CStatusBar CTRL_CONTENT(gStatusBar);
extern  SSYSSET gSysSet;


static void Form_VoiceSet_Draw(LPWindow pWindow);
static void Form_VoiceSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

static char keyvoice[6]="";
static CAttrItem  _attritem[] = {
   {"°´¼üÒô",keyvoice,1},
};


DEF_ATTR_CTRL(mVoiceAttr, &gWD_VoiceSet, 0, 20, 240, 108, "ÉùÒôÉèÖÃ",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);
char* cVoiceItem[] = 
{
    "¹Ø±Õ",
    "´ò¿ª",
};
DEF_LIST_DLG(gWD_KeyVoice, 20, 20, 200, 88, "°´¼üÒô", cVoiceItem, sizeof(cVoiceItem)/sizeof(char *), 1, 0);


static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mVoiceAttr,
};

CWindow  gWD_VoiceSet = {
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
    NULL,
    Form_VoiceSet_Draw,
    Form_VoiceSet_Proc
};


void Form_VoiceSet_Draw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;

	//Çå³ýÏÔ´æ
	EraseBuffer();

	//½ûÖ¹»æÍ¼
	SetRedraw(FALSE);

	//½ûÖ¹Ë¢ÆÁ
	EnableScreenFlush(FALSE);

	//ÉèÖÃÊÓÍ¼
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

	//»æÖÆ¿Ø¼þ
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
			lpControl->DrawFunc(lpControl);
		}
	}
	//Ê¹ÄÜË¢ÆÁ
	EnableScreenFlush(TRUE);

	//Ë¢ÆÁ
	FlushScreen();

	//Ê¹ÄÜ»æÍ¼
	SetRedraw(TRUE);
}
void Form_VoiceSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	s16 flag=0;

	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			//CreateWindowVoicerEx(pWindow, 1);
			//StartWindowVoicer(pWindow);		//Æô¶¯Ö÷´°Ìå¶¨Ê±Æ÷
		case WM_SHOW:
            sprintf(keyvoice,"%s",cVoiceItem[gSysSet.Beep]);
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
                    if (CTRL_CONTENT(mVoiceAttr).focus > 0) {
                        CTRL_CONTENT(mVoiceAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mVoiceAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mVoiceAttr).focus + 1 < CTRL_CONTENT(mVoiceAttr).total) {
                        CTRL_CONTENT(mVoiceAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mVoiceAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
                    if (CTRL_CONTENT(mVoiceAttr).focus == 0) {
                        CTRL_CONTENT(gWD_KeyVoice).focus  = gSysSet.Beep;
                        CTRL_CONTENT(gWD_KeyVoice).select = gSysSet.Beep;
						flag = 	LoadListDlg(&gWD_KeyVoice);
                        if(flag >= 0)
						{
                            gSysSet.Beep = flag;
							SysSetParamSave();
							if(flag)
							    Beep_Power(1);      //¿ªÆô·äÃùÆ÷
							else
							    Beep_Power(0);      //¹Ø±Õ	   
						}
						flag = 0;
						EnableVoiceIcon(gSysSet.Beep);
                    }else {
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
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


