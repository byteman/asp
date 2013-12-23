#include "Ctr_StatusBar.h"
#include "Form_Resource.h"

#include "stm32f4_rtc.h"
#include "app_rtc.h"
#include "app_beep.h"
#include "device_info.h"
#include "sys_config.h"

DEF_STATUS_BAR(gStatusBar, NULL, 0, 0, 240, 15, 1, 1, 1, 1, 0, "kg", "09:15", CTRL_VISABLE);

static char buff[16] = {0};
void Form_StatusBar_Draw(CControl* pControl)
{		
	BOOL LC_Run_Status;
	BOOL LC_Err_Status;
	BOOL GC_Run_Status;
	BOOL GC_Err_Status;
	BOOL BGC_Run_Status;
	BOOL BGC_Err_Status;
	CSysTime* time = NULL;
    CStatusBar *pStatusBar = (CStatusBar *)(pControl->pContent);
    static char timeshow=1;

    Update_System_Time();	//更新rtc时间
	time = Get_System_Time();    //2012.11.21 lxj
	
	EnableScreenFlush(FALSE);
	SetColor(1);
	FillRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	SetColor(0);

	//光幕状态
	LC_Run_Status = Get_LC_Status(2);
	LC_Err_Status = Get_LC_ErrStatus(2);
	if(LC_Err_Status)
	{
		if(LC_Run_Status)
		{
			DrawImage(Img_Gm2, 27, 0, 14, 14);
		}
		else
		{
			DrawImage(Img_Gm1, 27, 0, 14, 14);	
		}
	}
	else
	{
		DrawImage(Img_Gm3, 27, 0, 14, 14); 
	}

    //前地感状态
	GC_Err_Status = Get_GC_ErrStatus(1);
	GC_Run_Status = Get_GC_Status(1);
	if(GC_Err_Status)
	{
		if(GC_Run_Status)
		{
			DrawImage(Img_Dg2, 2, 0, 14, 14);
		}
		else
		{
			DrawImage(Img_Dg1, 2, 0, 14, 14);	
		}
	}
	else
	{
		DrawImage(Img_Dg3, 2, 0, 14, 14);
	}

	
    //后地感状态
	BGC_Err_Status = Get_GC_ErrStatus(2);
	BGC_Run_Status = Get_GC_Status(2);
	if(BGC_Err_Status)
	{
		if(BGC_Run_Status)
		{
			DrawImage(Img_Dg2, 50, 0, 14, 14);
		}
		else
		{
			DrawImage(Img_Dg1, 50, 0, 14, 14);	
		}
	}
	else
	{
		DrawImage(Img_Dg3, 50, 0, 14, 14);
	}


//轮胎识别器	
#if 0    
    switch(pStatusBar->sta_lz){
	    case 0:
	        DrawImage(Img_Lz3, 50, 0, 14, 14);
	        break;
	
	    case 1:
	        DrawImage(Img_Lz0, 50, 0, 14, 14);
	        break;
	
	    case 2:
	        DrawImage(Img_Lz1, 50, 0, 14, 14);
	        break;
	
	    case 3:
	        DrawImage(Img_Lz2, 50, 0, 14, 14);
	        break;
	
	    default:
	        break;
    }
#endif
    
    if(Get_Beep_Flag()) {
        DrawImage(Img_Voice, 75, 0, 14, 14);
    } else {
        DrawImage(Img_VoiceX, 75, 0, 14, 14);
    }

    if(Get_ADChip_ErrStatus() > 0)
    {
		DrawGbText("ADERR", 100, 0);
    }

    if(pStatusBar->time) 
	{
        if((time->sec == 5)||(time->sec == 25)||(time->sec == 45)) {
            timeshow = 0;
        }
        if((time->sec == 10)||(time->sec == 30)||(time->sec == 50)) {
            timeshow = 1;
        }
        if(timeshow) {
    		sprintf(buff, "%02d:%02d:%02d", time->hour, time->min, time->sec);	
            DrawGbText(buff, 184, 0);
        } else {
    		sprintf(buff, "%04d/%02d/%02d", time->year, time->mon, time->day);	
            DrawGbText(buff, 170, 0);
        }
    }

	EnableScreenFlush(TRUE);
	FlushRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
}


 void EnableVoiceIcon(BOOL b)
{
	if(b == 0)
	{
		CTRL_CONTENT(gStatusBar).voice = 0;	
	}
	else
	{
		CTRL_CONTENT(gStatusBar).voice = 1;
	} 
}

void SetGmState(char sta)
{
	CTRL_CONTENT(gStatusBar).sta_gm= sta; 
}

void SetDgState(char sta)
{
	CTRL_CONTENT(gStatusBar).sta_dg= sta; 
}

void SetLzState(char sta)
{
	CTRL_CONTENT(gStatusBar).sta_lz= sta; 
}


