#include "Form_Main_Veh.h"
#include "Form_Main_Axle.h"
#include "Ctr_StatusBar.h"
#include "Form_Main.h"

#include "Common.h"

#include "overtime_queue.h"


static void Form_Main_Veh_Timer(void *ptmr, void *parg);
static void Form_Main_Veh_Draw(LPWindow pWindow);
static void Form_Main_Veh_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

DEF_TEXT_LABEL(mAxleNum_T,      &gWD_Main_Veh, 5, 22, 60, 14, CTRL_VISABLE, "轴数");
DEF_TEXT_LABEL(mAxleGroup_T,    &gWD_Main_Veh, 5, 44, 30, 18, CTRL_VISABLE, "轴组");
DEF_TEXT_LABEL(mAxleType_T,     &gWD_Main_Veh, 5, 66, 30, 18, CTRL_VISABLE, "轴型");
DEF_TEXT_LABEL(mSpeed_T,        &gWD_Main_Veh, 5, 88, 30, 18, CTRL_VISABLE, "车速");
DEF_TEXT_LABEL(mAccel_T,        &gWD_Main_Veh,120,88, 30, 18, CTRL_VISABLE, "加速度");
DEF_TEXT_LABEL(mVehWet_T,          &gWD_Main_Veh, 5, 110, 30, 18, CTRL_VISABLE,"总重");


static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mAxleNum_T,
	&mAxleGroup_T,
    &mAxleType_T,
    &mSpeed_T,
    &mAccel_T,
    &mVehWet_T,
};

CWindow  gWD_Main_Veh = {
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
    Form_Main_Veh_Timer,
    Form_Main_Veh_Draw,
    Form_Main_Veh_Proc
};

void Form_Main_Veh_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_Main_Veh_Draw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;
	char cBuff[36];

	//清除显存
	EraseBuffer();

	//禁止绘图
	SetRedraw(FALSE);

	//禁止刷屏
	EnableScreenFlush(FALSE);

	//设置视图
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

    DrawHoriLine(0, 18, 240);
    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);
    DrawHoriLine(0, 106, 240);

    DrawVertLine(42,  18, 110);
    DrawVertLine(115, 84, 22);
    DrawVertLine(167, 84, 22);
//     sprintf(cBuff,"%d",gVehWeight.ucAxleCount);
		sprintf(cBuff,"%d",gMainCarInfo.AxleNum);
    DrawGbText(cBuff, 47, 22);
    sprintf(cBuff,"%d",gMainCarInfo.AxleGroupNum);
    DrawGbText(cBuff, 47, 44);
    for(i=0;i<gMainCarInfo.AxleNum;i++) {
        cBuff[i]= '0'+gMainCarInfo.AxleType[i];
    }
    cBuff[i]= 0;
     DrawGbText(cBuff, 47, 66);
    sprintf(cBuff,"%d.%dkm/h",gMainCarInfo.speed/10, gMainCarInfo.speed%10);
    DrawGbText(cBuff, 47, 88);
    sprintf(cBuff,"%d.%dm/s2",gMainCarInfo.speeda/10, gMainCarInfo.speeda%10);
    DrawGbText(cBuff, 172, 88);
    
    if(GetCurUnit()) {
        sprintf(cBuff,"%0.2f %s",Kg2CurVal(gMainCarInfo.TotalWet),GetCurUnitStr());
    } else {
        sprintf(cBuff,"%0.0f %s",Kg2CurVal(gMainCarInfo.TotalWet),GetCurUnitStr());
    }
    DrawGbText(cBuff, 47, 110);
     if(1==gMainCarInfo.OverWetFlag) {
	    DrawGbText("超载", 200, 110);
    } 
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

void Form_Main_Veh_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(2);

				
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
					break;
				case KEY_DOWN:
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					break;

				case KEY_RIGHT: 
					break;

				case KEY_AXIS:
                    g_pCurWindow = &gWD_Main_Axle;
                    g_pCurWindow->pParentWindow = &gWD_Main;
					guiMsgInfo.pWindow = g_pCurWindow;
					guiMsgInfo.ID = WM_LOAD;
					GuiMsgQueuePost(&guiMsgInfo);
                    break;

				case KEY_BACK:
					g_pCurWindow = &gWD_Main;
                    PostWindowMsg(g_pCurWindow, WM_LOAD,0,0);
					break;

                case KEY_PRINT:
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
        case WM_CARIN:
            PostWindowMsg(pWindow, WM_SHOW,0,0);
            break;
        case WM_CARBACK:

            break;
    	case WM_AXELADD:

            break;
    	case WM_AXELSUB:

            break;
    	case WM_VEHWET:

            break;
		
		default:
			break;
	}	
}



