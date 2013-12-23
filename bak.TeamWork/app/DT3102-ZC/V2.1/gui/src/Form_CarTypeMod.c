#include "Form_CarTypeMod.h"
#include "Ctr_Attr.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "Form_Main.h"
#include "Form_MsgBox.h"
#include "Common.h"
#include "sys_param.h"

extern  CControl  gStatusBar;

static void Form_CarTypeMod_Timer(void *ptmr, void *parg);
static void Form_CarTypeMod_Draw(LPWindow pWindow);
static void Form_CarTypeMod_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

extern SCarTypeMod gCarTypeMod[20];
static char gCarTypeNo = 0;
static char* gsCarTypeEn[] = {
    "关闭",
    "开启",
};
/*
 * 强制轴重
 */
DEF_LIST_DLG(gWD_CarTypeModEn, 20, 20, 200, 88, "车型修正", gsCarTypeEn, sizeof(gsCarTypeEn)/sizeof(char *), 1, 0);

__align(8) static char gShowStr[8][10];
static CAttrItem  _attritem[] = {
   {"使能",gShowStr[0],1},
   {"车辆轴型",gShowStr[1],2},
   {"5km/h",gShowStr[2],3},
   {"10km/h",gShowStr[3],4},
   {"15km/h",gShowStr[4],5},
   {"20km/h",gShowStr[5],6},
   {"25km/h",gShowStr[6],7},
   {"30km/h",gShowStr[7],8},
};

DEF_ATTR_CTRL(mCarTypeMod, &gWD_CarTypeMod, 0, 20, 240, 108, "车型修正",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mCarTypeMod,
};

CWindow  gWD_CarTypeMod = {
    (LPControl *)marrLPControl,
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
    Form_CarTypeMod_Timer,
    Form_CarTypeMod_Draw,
    Form_CarTypeMod_Proc
};

void Form_CarTypeMod_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_CarTypeMod_Draw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;
    char ctemp[8];
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
    sprintf(ctemp,"(%d/20)",gCarTypeNo+1);
    DrawGbText(ctemp,190,21);
	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}
void Form_CarTypeMod_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
	int16 flag=0;
    char  cTemp[10]="";
    char  i;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(3);
			CTRL_CONTENT(mCarTypeMod).focus = 0;

		case WM_SHOW:
            sprintf(gShowStr[0],"%s",gsCarTypeEn[gCarTypeMod[gCarTypeNo].en]);
            sprintf(gShowStr[1],"%s",gCarTypeMod[gCarTypeNo].AxleType);
            sprintf(gShowStr[2],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_5);
            sprintf(gShowStr[3],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_10);
            sprintf(gShowStr[4],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_15);
            sprintf(gShowStr[5],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_20);
            sprintf(gShowStr[6],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_25);
            sprintf(gShowStr[7],"%d‰",gCarTypeMod[gCarTypeNo].SpeedMod_30);
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
			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_UP:
                    if (CTRL_CONTENT(mCarTypeMod).focus > 0) {
                        CTRL_CONTENT(mCarTypeMod).focus--;
    					guiMsgInfo.ID = WM_SHOW;
    					GuiMsgQueuePost(&guiMsgInfo);					
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mCarTypeMod).focus + 1 < CTRL_CONTENT(mCarTypeMod).total) {
                        CTRL_CONTENT(mCarTypeMod).focus++;
    					guiMsgInfo.ID = WM_SHOW;
    					GuiMsgQueuePost(&guiMsgInfo);					
                    }
					break;

				case KEY_LEFT:
                    gCarTypeNo = (gCarTypeNo+20-1)%20;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_RIGHT: 
                    gCarTypeNo = (gCarTypeNo+1)%20;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_OK:
                    switch (CTRL_CONTENT(mCarTypeMod).focus) {

                    case 0:
                        CTRL_CONTENT(gWD_CarTypeModEn).focus = 0;
                        CTRL_CONTENT(gWD_CarTypeModEn).select = 0;
						flag = 	LoadListDlg(&gWD_CarTypeModEn);
						if(flag >= 0)	
    					{
							gCarTypeMod[gCarTypeNo].en = flag;
						}
						flag=0;
                        break;

                    case 1:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入车辆轴型", cTemp, cTemp, 10)) {
                            if(strlen(cTemp)) {
                                for(i=0;i<strlen(cTemp);i++) {
                                    if((cTemp[i]<'1')||(cTemp[i]>'8')) {
                                        break;
                                    }
                                }
                                if((strlen(cTemp)<2)||(i!=strlen(cTemp))) {
                                    MsgBoxDlg(&g_MsgBoxDlg, "输入错误", "车辆轴型不合法");
                                } else {
                                    strcpy((char *)gCarTypeMod[gCarTypeNo].AxleType,cTemp);
                                }
                            }
    					}
                        break;
                        
                    case 2:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_5= atoi(cTemp);
                            }
    					}
                        break;
                        
                    case 3:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_10= atoi(cTemp);
                            }
    					}
                        break;
                        
                    case 4:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_15= atoi(cTemp);
                            }
    					}
                        break;
                        
                    case 5:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_20= atoi(cTemp);
                            }
    					}
                        break;
                        
                    case 6:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_25= atoi(cTemp);
                            }
    					}
                        break;
                        
                    case 7:
                        memset(cTemp,0,sizeof(cTemp));
    					if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 4)) {
                            if(strlen(cTemp)) {
    							gCarTypeMod[gCarTypeNo].SpeedMod_30= atoi(cTemp);
                            }
    					}
                        break;

                    default:
                        break;
                    }
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_BACK:
                    CarTypeModParamStore();
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = pWindow->pParentWindow;
					}
					else
					{
						g_pCurWindow = &gWD_Main;
					}
                    PostWindowMsg(WM_LOAD,0,0);
					break;

				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}



