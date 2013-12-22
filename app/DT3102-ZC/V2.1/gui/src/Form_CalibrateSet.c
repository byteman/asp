#include "Form_CalibrateSet.h"
#include "Form_Main.h"
#include "Ctr_Attr.h"
#include "Form_MsgBox.h"
#include "../Inc/TextEdit.h"

#include "Common.h"
#include "wet_algorim.h"
#include "weight_param.h"

extern  CControl  gStatusBar; 		//状态栏

static void Form_CalibrateSet_Draw(LPWindow pWindow);
static void Form_CalibrateSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

static char DynamicCoefficientSet[10] = {0};
static char StaticCoefficientSet[10] = {0};
static char Axle3_K_Value[10] = {0};
static char Axle4_K_Value[10] = {0};

static CAttrItem  _attritem[] = {
	{"动态系数设置",DynamicCoefficientSet, 1},
	{"静态系数设置",StaticCoefficientSet, 2},
	{"3轴K系数设置", Axle3_K_Value, 3},
	{"4轴K系数设置", Axle4_K_Value, 4}
};

DEF_ATTR_CTRL(mCalibrateSetAttr, &gWD_CalibrateSet, 0, 20, 240, 108, "标定系数设置",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mCalibrateSetAttr,
};

CWindow  gWD_CalibrateSet = {
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
    Form_CalibrateSet_Draw,
    Form_CalibrateSet_Proc
};

void Form_CalibrateSet_Draw(LPWindow pWindow)
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
void Form_CalibrateSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	char   cTemp[10] = {0};

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(3);
			CTRL_CONTENT(mCalibrateSetAttr).focus = 0;

		case WM_SHOW:
            sprintf(DynamicCoefficientSet,"%d",0);
            sprintf(StaticCoefficientSet,"%f", gWetPar.Scaler_K); 
            sprintf(Axle3_K_Value,"%f", gWetPar.PreAxle1_K); 
            sprintf(Axle4_K_Value,"%f", gWetPar.PreAxle2_K); 
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
                    if (CTRL_CONTENT(mCalibrateSetAttr).focus > 0) {
                        CTRL_CONTENT(mCalibrateSetAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mCalibrateSetAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mCalibrateSetAttr).focus + 1 < CTRL_CONTENT(mCalibrateSetAttr).total) {
                        CTRL_CONTENT(mCalibrateSetAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mCalibrateSetAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					switch (CTRL_CONTENT(mCalibrateSetAttr).focus) 
					{
						case 0:
							if( LoadEditDlg(&g_TextEditWindow, "动态标定系数参数", cTemp, cTemp, 8));
					          break;
					          
						case 1:
							if( LoadEditDlg(&g_TextEditWindow, "静态标定系数参数", cTemp, cTemp, 8)) 
							{
								Set_ScalerStatic_K(atof(cTemp));
							}
							break;

						case 2:
							if( LoadEditDlg(&g_TextEditWindow, "3轴K系数", cTemp, cTemp, 8)) 
							{
								Set_3Axle_Static_K(atof(cTemp));
							}
							break;

						case 3:
							if( LoadEditDlg(&g_TextEditWindow, "4轴K系数", cTemp, cTemp, 8)) 
							{
								Set_4Axle_Static_K(atof(cTemp));
							}
							break;
										   
						 default:
							 break;
					   }
					   PostWindowMsg(pWindow, WM_SHOW,0,0);
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

