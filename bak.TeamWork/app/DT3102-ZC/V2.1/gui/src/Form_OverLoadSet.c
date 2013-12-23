#include "Form_OverLoadSet.h"
#include "Ctr_Attr.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "Form_Main.h"
#include "Form_MsgBox.h"
#include "Common.h"
#include "sys_param.h"
#include "weight_param.h"


extern  CControl  gStatusBar;

static void Form_OverLoadSet_Timer(void *ptmr, void *parg);
static void Form_OverLoadSet_Draw(LPWindow pWindow);
static void Form_OverLoadSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);



//ParamMax2Wet,				//2Öá³µ×î´óÖØÁ¿
//     ParamMax3Wet,
//     ParamMax4Wet,
//     ParamMax5Wet,
//     ParamMax6Wet,
uint32 Max2AxisOverLoadSet = 6000;
uint32 Max3AxisOverLoadSet = 12000;
uint32 Max4AxisOverLoadSet = 24000;
uint32 Max5AxisOverLoadSet = 48000;
uint32 Max6AxisOverLoadSet = 96000;

static char gShowStr[5][10];
static const CAttrItem  _attritem[] = {
   {"Á½Öá³µ³¬ÔØÉèÖÃ",gShowStr[0],1},
   {"ÈýÖá³µ³¬ÔØÉèÖÃ",gShowStr[1],2},
   {"ËÄÖá³µ³¬ÔØÉèÖÃ",gShowStr[2],3},
   {"ÎåÖá³µ³¬ÔØÉèÖÃ",gShowStr[3],4},
   {"ÁùÖá³µ³¬ÔØÉèÖÃ",gShowStr[4],5},

};

DEF_ATTR_CTRL(mOverLoadSet, &gWD_OverLoadSet, 0, 20, 240, 108, "³¬ÔØÉèÖÃ",(CAttrItem *)(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mOverLoadSet,
};

CWindow  gWD_OverLoadSet = {
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
    Form_OverLoadSet_Timer,
    Form_OverLoadSet_Draw,
    Form_OverLoadSet_Proc
};

void Form_OverLoadSet_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_OverLoadSet_Draw(LPWindow pWindow)
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
void Form_OverLoadSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
    char  cTemp[10]="";
    u32   ulTemp;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(2);
			CTRL_CONTENT(mOverLoadSet).focus = 0;
				
			Param_Read(GET_U32_OFFSET(ParamMax2Wet),   &Max2AxisOverLoadSet,    sizeof(u32));
			Param_Read(GET_U32_OFFSET(ParamMax3Wet),   &Max3AxisOverLoadSet,    sizeof(u32));
			Param_Read(GET_U32_OFFSET(ParamMax4Wet),   &Max4AxisOverLoadSet,    sizeof(u32));
			Param_Read(GET_U32_OFFSET(ParamMax5Wet),   &Max5AxisOverLoadSet,    sizeof(u32));
			Param_Read(GET_U32_OFFSET(ParamMax6Wet),   &Max6AxisOverLoadSet,    sizeof(u32));

		case WM_SHOW:
        sprintf(gShowStr[0],"%d kg",Max2AxisOverLoadSet);
				sprintf(gShowStr[1],"%d kg",Max3AxisOverLoadSet);
				sprintf(gShowStr[2],"%d kg",Max4AxisOverLoadSet);
				sprintf(gShowStr[3],"%d kg",Max5AxisOverLoadSet);
				sprintf(gShowStr[4],"%d kg",Max6AxisOverLoadSet);

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
					if (CTRL_CONTENT(mOverLoadSet).focus > 0) 
					{
						CTRL_CONTENT(mOverLoadSet).focus--;
						PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOverLoadSet, 0);
					}
					break;
				case KEY_DOWN:
					if (CTRL_CONTENT(mOverLoadSet).focus + 1 < CTRL_CONTENT(mOverLoadSet).total) 
					{
						CTRL_CONTENT(mOverLoadSet).focus++;
						PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOverLoadSet, 0);
					}
					break;

				case KEY_LEFT:
					break;

				case KEY_RIGHT: 
					break;

				case KEY_OK:
					switch (CTRL_CONTENT(mOverLoadSet).focus) 
					{
							case 0:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "ÇëÊäÈë³¬ÔØãÐÖµ(kg)", cTemp, cTemp, 5)) 
									{
											if(strlen(cTemp)) 
											{
												ulTemp = atoi(cTemp);
												if(ulTemp > 50000) 
												{
													ulTemp = 50000;
												}
												Max2AxisOverLoadSet= ulTemp;
												gWetPar.Max2AxleWet = ulTemp;
												Param_Write(GET_U32_OFFSET(ParamMax2Wet), &ulTemp, sizeof(u32));
                      }
									}
                  break;
									
							 case 1:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "ÇëÊäÈë³¬ÔØãÐÖµ(kg)", cTemp, cTemp, 5)) 
									{
											if(strlen(cTemp)) 
											{
												ulTemp = atoi(cTemp);
												if(ulTemp > 50000) 
												{
													ulTemp = 50000;
												}
												Max3AxisOverLoadSet= ulTemp;
												gWetPar.Max3AxleWet = ulTemp;
												Param_Write(GET_U32_OFFSET(ParamMax3Wet), &ulTemp, sizeof(u32));
                      }
									}
                  break;
								
							 case 2:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "ÇëÊäÈë³¬ÔØãÐÖµ(kg)", cTemp, cTemp, 5)) 
									{
											if(strlen(cTemp)) 
											{
												ulTemp = atoi(cTemp);
												if(ulTemp > 50000) 
												{
													ulTemp = 50000;
												}
												Max4AxisOverLoadSet= ulTemp;
												gWetPar.Max4AxleWet = ulTemp;
												Param_Write(GET_U32_OFFSET(ParamMax4Wet), &ulTemp, sizeof(u32));
                      }
									}
                  break;
									
								case 3:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "ÇëÊäÈë³¬ÔØãÐÖµ(kg)", cTemp, cTemp, 5)) 
									{
											if(strlen(cTemp)) 
											{
												ulTemp = atoi(cTemp);
												if(ulTemp > 50000) 
												{
													ulTemp = 50000;
												}
												Max5AxisOverLoadSet= ulTemp;
												gWetPar.Max5AxleWet = ulTemp;
												Param_Write(GET_U32_OFFSET(ParamMax5Wet), &ulTemp, sizeof(u32));
                      }
									}
                  break;
									
							case 4:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "ÇëÊäÈë³¬ÔØãÐÖµ(kg)", cTemp, cTemp, 5)) 
									{
											if(strlen(cTemp)) 
											{
												ulTemp = atoi(cTemp);
												if(ulTemp > 50000) 
												{
													ulTemp = 50000;
												}
												Max6AxisOverLoadSet= ulTemp;
												gWetPar.Max6AxleWet = ulTemp;
												Param_Write(GET_U32_OFFSET(ParamMax6Wet), &ulTemp, sizeof(u32));
                      }
									}
                  break;
									
								default:
                  break;
							}
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
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



