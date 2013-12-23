#include "Form_CalibrateZero.h"

#include "Ctr_StatusBar.h"
#include "Form_Main.h"

#include "Form_Resource.h"
#include "../Inc/TextEdit.h"
#include "../Inc/Menu.h"

#include "Form_MsgBox.h"
#include "Common.h"
// #include "WetParam.h"	
#include "Form_CalibrateInput.h"

#include "wet_algorim.h"		//整车式
#include "sys_config.h"
#include "weight_param.h"

#include "string.h"

extern  CControl  gStatusBar; 		//状态栏
extern	CWindow		gWD_CalibMenu;

// static SCalibrationData mCalib;
__align(8) static char sScale_Title[16] = "";
__align(8) static char sCali_Title[32] = "";
// __align(8) static char sAD_Value[16] = "";
// __align(8) static char sZero_Value[16] = "";

static float sAD_Value   = 0.0;
static float sZero_Value   = 0.0;

//static long fZero_Value = 0;
BOOL CalibrateZeroFlag = FALSE;   //标定完成标志



static void Form_Calibrate_Timer(void *ptmr, void *parg);
static void Form_Calibrate_Draw(LPWindow pWindow);
static void Form_Calibrate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

DEF_SMALL_FLOAT_LABLE(mAD_Attr,   &gWD_CalibrateZero, 45,  44, 	170,  14, CTRL_VISABLE, &sAD_Value, 2,  -5000000, 5000000,    "", TA_LEFT);
DEF_SMALL_FLOAT_LABLE(mZero_Attr,   &gWD_CalibrateZero, 45,  66, 	170,  14, CTRL_VISABLE, &sZero_Value, 2,  -5000000, 5000000,    "", TA_LEFT);


DEF_STATUS_IMAGE_CTRL(m_CalibZero,   &gWD_CalibrateZero, 200, 21, 14,  14, CTRL_VISABLE, Img_zero, Img_nonezero, TRUE);
DEF_STATUS_IMAGE_CTRL(mCalibZeroSteadyImage, &gWD_CalibrateZero, 220, 21, 14, 14, CTRL_VISABLE, Img_steady, Img_unsteady, FALSE);



static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mAD_Attr,
    &mZero_Attr,
	&m_CalibZero,
	&mCalibZeroSteadyImage
};

CWindow  gWD_CalibrateZero = {
    (LPControl *)marrLPControl,
	sizeof(marrLPControl)/sizeof(LPControl),
    0,
    0,
    0,
    240,
    128,
    WS_VISABLE,
		NULL,//(pParentWindow)
    5,//5
    NULL,
    Form_Calibrate_Timer,
    Form_Calibrate_Draw,
    Form_Calibrate_Proc
};

static WetStatue wet = {0};

void Form_Calibrate_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
// 	uint32 tmp = 0;

	Get_Scaler_WetPar(&wet);
	

	if(lpWindow != NULL) {
    	if(g_pCurWindow == &gWD_CalibrateZero) 
		{
			sAD_Value = wet.ScalerAD;
			sZero_Value = gWetPar.ScalerZeroAD;
			if(CTRL_CONTENT(mCalibZeroSteadyImage).bHot != wet.ScalerStableFlag) 
			{
				CTRL_CONTENT(mCalibZeroSteadyImage).bHot = wet.ScalerStableFlag;
				//PostWindowMsg(WM_UPDATECTRL,(u32)&mCalibZeroSteadyImage,0);
			}
			if(CTRL_CONTENT(m_CalibZero).bHot != wet.ScalerZeroFlag) 
			{
				CTRL_CONTENT(m_CalibZero).bHot = wet.ScalerZeroFlag;
				PostWindowMsg(lpWindow, WM_UPDATECTRL,(u32)&m_CalibZero,0);
     		}
    		PostWindowMsg(lpWindow, WM_UPDATECTRL, 0, 0);
			//PostWindowMsg(WM_UPDATECTRL, (uint32)&mZero_Attr, 0);
    	}

    }
}

void Form_Calibrate_Draw(LPWindow pWindow)
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
 // sprintf(sZero_Value,"%d",fZero_Value);

    
    sprintf(sCali_Title,"%s",sScale_Title);
	DrawGbText("零位标定", 112, 88);
	DrawGbText("按确定键开始标定，否则请返回", 42, 110);
    
    DrawGbText("AD码",5,44);
    DrawGbText("零点",5,66);

	DrawGbText(sCali_Title, 5, 21);
    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);
    DrawHoriLine(0, 127, 240);
    DrawVertLine(40, 40, 88);
//     DrawVertLine(140, 40, 44);

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



void Form_Calibrate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
    BOOL ret = FALSE;// ret1 = FALSE;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			sprintf(sScale_Title,"零点标定");
            
			ClearScreen();
			WeightFregDiv(TRUE, 10);
			SysTimeDly(3);
			CreateWindowTimer(&gWD_CalibrateZero);

		case WM_SHOW:
			
			StartWindowTimer(&gWD_CalibrateZero);
			pWindow->DrawFunc(pWindow);
			break;

		case WM_UPDATECTRL:            
			mAD_Attr.DrawFunc(&mAD_Attr);
		    mZero_Attr.DrawFunc(&mZero_Attr);
			mCalibZeroSteadyImage.DrawFunc(&mCalibZeroSteadyImage);
			m_CalibZero.DrawFunc(&m_CalibZero);
#if 0		
			pControl = (CControl*)(pGuiMsgInfo->wParam);
			if(pControl != NULL)
			{
	         if (pControl->state & CTRL_VISABLE)
		         pControl->DrawFunc(pControl);
			} 			
#endif		
			break;

		case WM_TIMEUPDATE:
			gStatusBar.DrawFunc(&gStatusBar);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_UP:
					break;
				case KEY_DOWN:
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					ret = Weight_Calc_Zero();
					
					if(ret) 
					{
						Scaler_Reset_Zero();
						CalibrateZeroFlag = TRUE;		//标定完成标志
						DrawGbText("零点标定成功, 请按返回键退出", 42, 110);
					} 
					else 
					{
						MsgBoxDlg(&g_MsgBoxDlg, "零点标定失败", "请重新标定零点");
						g_pCurWindow = &gWD_CalibrateZero;
    					g_pCurWindow->pParentWindow = &gWD_CalibMenu;
    					PostWindowMsg(g_pCurWindow, WM_LOAD, 1, NULL);	
    			}			
					break;

				case KEY_RIGHT: 
					break;

				case KEY_BACK:
					WeightFregDiv(FALSE, 0);
          StopWindowTimer(&gWD_CalibrateZero);
//           CalibrateProc();
//           CalibParamSync();
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

				case KEY_1:
				case KEY_2:
				case KEY_3:
				case KEY_4:
				case KEY_5:
				case KEY_6:
				case KEY_7:
				case KEY_8:
				case KEY_9:
					break;
				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}



