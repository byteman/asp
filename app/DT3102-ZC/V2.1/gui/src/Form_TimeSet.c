#include "Form_TimeSet.h"
#include "Form_Main.h"
#include "Form_Resource.h"
#include "Form_TimeDate.h"
//#include "rtc.h"
//#include "pcf8685t_init.h"
#include "includes.h"
#include "app_rtc.h"

extern  CControl  gStatusBar; 		//×´Ì¬À¸

static void Form_TimeSet_Timer(void *ptmr, void *parg);
static void Form_TimeSet_Draw(LPWindow pWindow);
static void Form_TimeSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

//static char year[5];
//static char month[3];
//static char day[3];

//static char week[3];

static char hour[3];
static char minu[3];
static char sec[3];

DEF_TEXT_LABEL(mTimeHH, &gWD_TimeSet, 20, 60, 22, 20,CTRL_VISABLE|CTRL_FOCUS,hour);
DEF_TEXT_LABEL(mTimeMM, &gWD_TimeSet, 42, 60, 22, 20,CTRL_VISABLE,minu);
DEF_TEXT_LABEL(mTimeSS, &gWD_TimeSet, 64, 60, 22, 20,CTRL_VISABLE,sec);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mTimeHH,
	&mTimeMM,
	&mTimeSS,
};

CWindow  gWD_TimeSet = {
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
    Form_TimeSet_Timer,
    Form_TimeSet_Draw,
    Form_TimeSet_Proc
};


void Form_TimeSet_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_TimeSet_Draw(LPWindow pWindow)
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

    DrawGbText("Ê±¼ä", 0, 25);

    DrawHoriLine(0, 25+16, 240);
    DrawHoriLine(0, 25+18, 240);
	//»æÖÆ¿Ø¼þ
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
    		if(lpControl->state & CTRL_FOCUS)
    		{
                DrawImage(Img_Up, lpControl->x, lpControl->y-13, 12, 12);
                DrawImage(Img_Down, lpControl->x, lpControl->y+15, 12, 12);
                //DrawGbText("¡ø", lpControl->x, lpControl->y-14);
                //DrawGbText("¡ø", lpControl->x, lpControl->y+15);
    			InvertColor();
            }
			lpControl->DrawFunc(lpControl);
    		if(lpControl->state & CTRL_FOCUS)
    		{
    			InvertColor();
            }
		}
	}
    DrawGbText(":", 35, 60);
    DrawGbText(":", 56, 60);
	//Ê¹ÄÜË¢ÆÁ
	EnableScreenFlush(TRUE);

	//Ë¢ÆÁ
	FlushScreen();

	//Ê¹ÄÜ»æÍ¼
	SetRedraw(TRUE);
}
void Form_TimeSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
	static CSysTime time = {0};
	static CSysTime* SetTime ;
	
//	SetTime = &time;
	
// 	CSysTime defaultTime={2012,11,27,1,18,39,5};
//		CSysTime defaultTime={0};

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
//    		GetSysTime(&time);
			SetTime = Get_System_Time();
			time.year = SetTime->year;
			time.mon	=	SetTime->mon;
			time.day	= SetTime->day;
			time.week	= SetTime->week;
			time.hour	= SetTime->hour;
			time.min	= SetTime->min;
			time.sec	=	SetTime->sec;
			//CreateWindowTimerEx(pWindow, 1);
			//StartWindowTimer(pWindow);		//Æô¶¯Ö÷´°Ìå¶¨Ê±Æ÷
		case WM_SHOW:
// 				sprintf(year, "%02d", time->year);		
//     		sprintf(month, "%02d", time->mon);		
//     		sprintf(day, "%02d", time->day);
// 		
// 			  sprintf(week, "%02d", time->week);
		
    		sprintf(hour, "%02d", time.hour);		
    		sprintf(minu, "%02d", time.min);		
    		sprintf(sec, "%02d", time.sec);		
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
                    if(mTimeHH.state & CTRL_FOCUS) {
                        (time.hour)++;
                        if ((time.hour) > 23) {
                            time.hour = 0;
                        }
                    } else if(mTimeMM.state & CTRL_FOCUS) {
                        time.min++;
                        if (time.min > 59) {
                            time.min = 0;
                        }
                    } else if(mTimeSS.state & CTRL_FOCUS) {
                        time.sec++;
                        if (time.sec > 59) {
                            time.sec = 0;
                        }
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;
				case KEY_DOWN:
                    if(mTimeHH.state & CTRL_FOCUS) {
                        if (time.hour ) {
                            (time.hour)--;
                        } else {
                            time.hour = 23;
                        }
                    } else if(mTimeMM.state & CTRL_FOCUS) {
                        if (time.min ) {
                            time.min--;
                        } else {
                            time.min = 59;
                        }
                    } else if(mTimeSS.state & CTRL_FOCUS) {
                        if (time.sec ) {
                            time.sec--;
                        } else {
                            time.sec = 59;
                        }
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_LEFT:
                    if(mTimeSS.state & CTRL_FOCUS) {
                        mTimeSS.state &=~CTRL_FOCUS;
                        mTimeMM.state |= CTRL_FOCUS;
                    }else if(mTimeMM.state & CTRL_FOCUS) {
                        mTimeMM.state &=~CTRL_FOCUS;
                        mTimeHH.state |= CTRL_FOCUS;
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_OK:
//                    SetSysTime(time->hour, time->min, time->sec);
					
//					defaultTime = {time->year, time->mon,	time->day, time->week, time->hour, time->min, time->sec};
					
					Set_System_Time(&time);
        			SysTimeDly(100);
					
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = &gWD_TimeDate; //pWindow->pParentWindow;
					}
					else
					{
						g_pCurWindow = &gWD_Main;
					}
					guiMsgInfo.pWindow = g_pCurWindow;
					guiMsgInfo.ID = WM_LOAD;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_RIGHT: 
                    if(mTimeHH.state & CTRL_FOCUS) {
                        mTimeHH.state &=~CTRL_FOCUS;
                        mTimeMM.state |= CTRL_FOCUS;
                    } else if(mTimeMM.state & CTRL_FOCUS) {
                        mTimeMM.state &=~CTRL_FOCUS;
                        mTimeSS.state |= CTRL_FOCUS;
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_BACK:
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow =  pWindow->pParentWindow;//&gWD_TimeDate;
					}
					else
					{
						g_pCurWindow = &gWD_Main;
					}
					PostWindowMsg(g_pCurWindow, WM_LOAD,0,0);
					
// 					guiMsgInfo.ID = WM_LOAD;
// 					GuiMsgQueuePost(&guiMsgInfo);
					break;

				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}



