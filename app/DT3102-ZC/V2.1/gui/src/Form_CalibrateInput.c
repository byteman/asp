#include "Form_CalibrateInput.h"
#include "Ctr_Attr.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "Form_Main.h"
#include "Form_MsgBox.h"
#include "guiwindows.h"

// #include "WetParam.h"

#include "sys_config.h"
#include "weight_param.h"
#include "Form_CalibrateWeight.h"

extern  CControl  gStatusBar;

static void Form_CalibrateInput_Timer(void *ptmr, void *parg);
static void Form_CalibrateInput_Draw(LPWindow pWindow);
static void Form_CalibrateInput_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

static BOOL CalibrateWeightFlag = FALSE;
static BOOL CalibratePositionFlag = FALSE;

__align(8) static char cCalibrateWeight[16]="";
__align(8) static char cCalibratePosition [16]="";
static CAttrItem  _attritem[] = {

   {"输入砝码重量",cCalibrateWeight,1},
   {"输入标定位置",cCalibratePosition,2},
   
};

DEF_ATTR_CTRL(mCalibrateInput, &gWD_CalibrateInput, 0, 20, 240, 108, "输入标定砝码重量",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
		&mCalibrateInput,
};

CWindow  gWD_CalibrateInput = {
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
    Form_CalibrateInput_Timer,
    Form_CalibrateInput_Draw,
    Form_CalibrateInput_Proc
};

void Form_CalibrateInput_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_CalibrateInput_Draw(LPWindow pWindow)
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
void Form_CalibrateInput_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
//	GuiMsgInfo guiMsgInfo;
//	int16 flag=0;
  char  cTemp[10]="";
	u32   ulTemp;
//	char len;
	

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(3);
			CTRL_CONTENT(mCalibrateInput).focus = 0;
			memset(cCalibrateWeight,0,sizeof(cCalibrateWeight));
      memset(cCalibratePosition,0,sizeof(cCalibratePosition));

		case WM_SHOW:
            
//       sprintf(cCalibrateWeight,"%d",cCalibrateWeight[0]);
//       sprintf(cCalibratePosition,"%d",cCalibratePosition[0]);
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
				  if (CTRL_CONTENT(mCalibrateInput).focus > 0) {
					CTRL_CONTENT(mCalibrateInput).focus--;
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mCalibrateInput, 0);
				  }
				break;
				case KEY_DOWN:
				  if (CTRL_CONTENT(mCalibrateInput).focus + 1 < CTRL_CONTENT(mCalibrateInput).total) {
					CTRL_CONTENT(mCalibrateInput).focus++;
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mCalibrateInput, 0);
				  }
				break;

				case KEY_LEFT:
					break;

				case KEY_RIGHT: 
					break;

				case KEY_OK:
					
						if(CalibrateWeightFlag == TRUE && CalibratePositionFlag == TRUE)
						{
								MsgBoxDlg(&g_MsgBoxDlg, "", "进入整车式重量标定流程");
				
 								g_pCurWindow = &gWD_CalibrateWeight;					//重量标定
								g_pCurWindow->pParentWindow = &gWD_CalibrateInput;
								PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
								break;
						}
						else
						{
								
							switch (CTRL_CONTENT(mCalibrateInput).focus) {
							case 0:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "请输入标定砝码重量", cTemp, cTemp, 4)) 
									{
										CalibrateWeightFlag = TRUE;
										cCalibrateWeight[0] = atoi(cTemp);
										sprintf(cCalibrateWeight,"%d",cCalibrateWeight[0]);
										PostWindowMsg(pWindow, WM_SHOW,0,0);	

									}
									break;
                
							case 1:
									memset(cTemp,0,sizeof(cTemp));
									if( LoadEditDlg(&g_TextEditWindow, "请输入标定位置", cTemp, cTemp, 4)) 
									{
										CalibratePositionFlag = TRUE;
										ulTemp = atoi(cTemp);
										cCalibratePosition[0] = ulTemp;
										sprintf(cCalibratePosition,"%d",cCalibratePosition[0]);
										PostWindowMsg(pWindow, WM_SHOW,0,0);	

									}
                 break;
                        
             default:
                 break;
						}
                        
						
          }
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



