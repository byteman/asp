#include "Form_DateSet.h"
#include "Form_Main.h"
#include "Form_Resource.h"

#include "includes.h"
#include "app_rtc.h"


#define  YEARMAX  2100
#define  YEARMIN  2000
extern  CControl  gStatusBar; 		//状态栏

static void Form_DateSet_Timer(void *ptmr, void *parg);
static void Form_DateSet_Draw(LPWindow pWindow);
static void Form_DateSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


static char year[5];
static char month[3];
static char day[3];

//static char week[3];

//static char hour[3];
//static char minu[3];
//static char sec[3];

DEF_TEXT_LABEL(mDateYY, &gWD_DateSet, 20, 60, 40, 20,CTRL_VISABLE|CTRL_FOCUS,year);
DEF_TEXT_LABEL(mDateMM, &gWD_DateSet, 60, 60, 22, 20,CTRL_VISABLE,month);
DEF_TEXT_LABEL(mDateDD, &gWD_DateSet, 82, 60, 22, 20,CTRL_VISABLE,day);

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mDateYY,
	&mDateMM,
	&mDateDD,
};

CWindow  gWD_DateSet = {
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
    Form_DateSet_Timer,
    Form_DateSet_Draw,
    Form_DateSet_Proc
};


void Form_DateSet_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_DateSet_Draw(LPWindow pWindow)
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

    DrawGbText("日期", 0, 25);

    DrawHoriLine(0, 25+16, 240);
    DrawHoriLine(0, 25+18, 240);
	//绘制控件
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
    		if(lpControl->state & CTRL_FOCUS)
    		{
                DrawImage(Img_Up, lpControl->x, lpControl->y-13, 12, 12);
                DrawImage(Img_Down, lpControl->x, lpControl->y+15, 12, 12);
                //DrawGbText("▲", lpControl->x, lpControl->y-14);
                //DrawGbText("▲", lpControl->x, lpControl->y+15);
    			InvertColor();
            }
			lpControl->DrawFunc(lpControl);
    		if(lpControl->state & CTRL_FOCUS)
    		{
    			InvertColor();
            }
		}
	}
    DrawGbText("-", 52, 60);
    DrawGbText("-", 74, 60);
	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}
void Form_DateSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
//	static CSysTime time;
	static CSysTime time = {0};
	static CSysTime *SetDate = NULL;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
//    		GetSysTime(&time);
		SetDate = Get_System_Time();
			time.year = SetDate->year;
			time.mon	=	SetDate->mon;
			time.day	= SetDate->day;
			time.week	= SetDate->week;
			time.hour	= SetDate->hour;
			time.min	= SetDate->min;
			time.sec	=	SetDate->sec;
			//CreateWindowTimerEx(pWindow, 1);
			//StartWindowTimer(pWindow);		//启动主窗体定时器
		case WM_SHOW:
    		sprintf(year, "%02d", time.year);		
    		sprintf(month, "%02d", time.mon);		
    		sprintf(day, "%02d", time.day);		
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
                    if(mDateYY.state & CTRL_FOCUS) {
                        time.year++;
											if (time.year > YEARMAX) {
                            time.year = YEARMIN;
                        }
                        if((time.mon == 2)&&(time.day > 28)) {
                            if (time.day > 29) {
                                time.day = 29;
                            }else if(time.year % 100) {
                                if(time.year % 4) {
                                    time.day=28;//非闰年
                                }
                            } else if(time.year % 400){
                                time.day=28;//非闰年
                            }
                        }
                    } else if(mDateMM.state & CTRL_FOCUS) {
                        time.mon++;
                        if (time.mon > 12) {
                            time.mon = 1;
                        }
                        switch(time.mon) {
                        case 2:
                            if(time.day > 28) {
                                if (time.day > 29) {
                                    time.day = 29;
                                }else if(time.year % 100) {
                                    if(time.year % 4) {
                                        time.day=28;//非闰年
                                    }
                                } else if(time.year % 400){
                                    time.day=28;//非闰年
                                }
                            }
                            break;
                            
                        case 4:
                        case 6:
                        case 9:
                        case 11:
                            if(time.day > 30) {
                                time.day = 30;
                            }
                            break;

                        default :
                            break;
                        }
                    } else if(mDateDD.state & CTRL_FOCUS) {
                        time.day++;
                        switch(time.mon) {
                        case 2:
                            if(time.day > 28) {
                                if (time.day > 29) {
                                    time.day = 1;
                                }else if(time.year % 100) {
                                    if(time.year % 4) {
                                        time.day=1;//非闰年
                                    }
                                } else if(time.year % 400){
                                    time.day=1;//非闰年
                                }
                            }
                            break;
                            
                        case 4:
                        case 6:
                        case 9:
                        case 11:
                            if(time.day > 30) {
                                time.day = 1;
                            }
                            break;

                        default :
                            if (time.day > 31) {
                                time.day = 1;
                            }
                            break;
                        }
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;
				case KEY_DOWN:
                    if(mDateYY.state & CTRL_FOCUS) {
                        if (time.year > YEARMIN) {
                            time.year--;
                        } else {
                            time.year = YEARMAX;
                        }
                        if((time.mon == 2)&&(time.day > 28)) {
                            if (time.day > 29) {
                                time.day = 29;
                            }else if(time.year % 100) {
                                if(time.year % 4) {
                                    time.day=28;//非闰年
                                }
                            } else if(time.year % 400){
                                time.day=28;//非闰年
                            }
                        }
                    } else if(mDateMM.state & CTRL_FOCUS) {
                        if (time.mon > 1) {
                            time.mon--;
                        } else {
                            time.mon = 12;
                        }
                        switch(time.mon) {
                        case 2:
                            if(time.day > 28) {
                                if (time.day > 29) {
                                    time.day = 29;
                                }else if(time.year % 100) {
                                    if(time.year % 4) {
                                        time.day=28;//非闰年
                                    }
                                } else if(time.year % 400){
                                    time.day=28;//非闰年
                                }
                            }
                            break;
                            
                        case 4:
                        case 6:
                        case 9:
                        case 11:
                            if(time.day > 30) {
                                time.day = 30;
                            }
                            break;

                        default :
                            break;
                        }
                    } else if(mDateDD.state & CTRL_FOCUS) {
                        if (time.day > 1) {
                            time.day--;
                        } else {
                            time.day = 31;
                        }
                        switch(time.mon) {
                        case 2:
                            if(time.day > 28) {
                                time.day=28;
                                if(time.year % 100) {
                                    if(!(time.year % 4)) {
                                        time.day=29;//闰年
                                    }
                                } else if(!(time.year % 400)){
                                    time.day=29;//闰年
                                }
                            }
                            break;
                            
                        case 4:
                        case 6:
                        case 9:
                        case 11:
                            if(time.day > 30) {
                                time.day = 30;
                            }
                            break;

                        default :
                            break;
                        }
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_LEFT:
                    if(mDateDD.state & CTRL_FOCUS) {
                        mDateDD.state &=~CTRL_FOCUS;
                        mDateMM.state |= CTRL_FOCUS;
                    }else if(mDateMM.state & CTRL_FOCUS) {
                        mDateMM.state &=~CTRL_FOCUS;
                        mDateYY.state |= CTRL_FOCUS;
                    }
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_OK:
//            SetSysDate(time.Year, time.Month, time.Day);
				      Set_System_Time(&time);
        			SysTimeDly(100);
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = pWindow->pParentWindow;
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
                    if(mDateYY.state & CTRL_FOCUS) {
                        mDateYY.state &=~CTRL_FOCUS;
                        mDateMM.state |= CTRL_FOCUS;
                    } else if(mDateMM.state & CTRL_FOCUS) {
                        mDateMM.state &=~CTRL_FOCUS;
                        mDateDD.state |= CTRL_FOCUS;
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



