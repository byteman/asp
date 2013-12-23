
#include "Form_OverloadQuery.h"
#include "Ctr_ListSelect.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Form_Main.h"
#include "Form_OverLoadDetailQuery.h"

#include "Common.h"
#include "overtime_queue.h"
#include "data_store_module.h"
#include "sys_param.h"

#include "weight_param.h"

extern  CControl  gStatusBar; 		//状态栏


static uint16 OverLoadDataPackNo = 0;
static uint16  OverLoadVehNo=0;

static OvertimeData  OverLoadVehShow;
static BOOL  OverLoadDataSta=FALSE;



static void Form_OverloadQuery_Timer(void *ptmr, void *parg);
static void Form_OverloadQuery_Draw(LPWindow pWindow);
static void Form_OverloadQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

DEF_TEXT_LABEL(mOverloadQuery_Title,    &gWD_OverloadQuery, 0, 22, 60, 14, CTRL_VISABLE, "超限数据查询");
DEF_TEXT_LABEL(mOverloadTime_Title,     &gWD_OverloadQuery, 5, 44, 30, 18, CTRL_VISABLE, "时间");
DEF_TEXT_LABEL(mOverloadSpeed_Title,    &gWD_OverloadQuery, 5, 66, 30, 18, CTRL_VISABLE, "车速");
DEF_TEXT_LABEL(mOverloadAxleGroup_Title,&gWD_OverloadQuery,125,66, 30, 18, CTRL_VISABLE, "轴组");
DEF_TEXT_LABEL(mOverloadAxleNum_Title,  &gWD_OverloadQuery, 5, 88, 30, 18, CTRL_VISABLE, "轴数");
DEF_TEXT_LABEL(mOverloadAxleType_Title, &gWD_OverloadQuery,125,88, 30, 18, CTRL_VISABLE, "轴型");
DEF_TEXT_LABEL(mOverloadWet_Title,      &gWD_OverloadQuery, 5, 110, 30, 18, CTRL_VISABLE,"总重");

void Get_OverLoadVehShow(OvertimeData *pData)
{
	memcpy(pData, &OverLoadVehShow, sizeof(OvertimeData));
}

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mOverloadQuery_Title,
    &mOverloadTime_Title,
    &mOverloadSpeed_Title,
    &mOverloadAxleGroup_Title,
    &mOverloadAxleNum_Title,
    &mOverloadAxleType_Title,
    &mOverloadWet_Title,
};

CWindow  gWD_OverloadQuery = {
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
    Form_OverloadQuery_Timer,
    Form_OverloadQuery_Draw,
    Form_OverloadQuery_Proc
};

void Form_OverloadQuery_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_OverloadQuery_Draw(LPWindow pWindow)
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

    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);
    DrawHoriLine(0, 106, 240);
	DrawHoriLine(0, 127, 240);
    DrawVertLine(42,  40, 88);
    DrawVertLine(120, 62, 44);
    DrawVertLine(162, 62, 44);

	//绘制控件
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
			lpControl->DrawFunc(lpControl);
		}
	}
	
    if(OverLoadVehNo < 1) 
    {
	    DrawGbText("无记录", 192, 22);
    } 
    else
    {
		if(OverLoadDataSta)
		{
			sprintf(cBuff,"%d/%d",OverLoadVehNo,OverLoadDataPackNo);
			DrawGbText(cBuff, 230-strlen(cBuff)*7, 22);
			sprintf(cBuff,"%d年%02d月%02d日%02d:%02d:%02d", OverLoadVehShow.year,
															OverLoadVehShow.mon,
															OverLoadVehShow.day,
															OverLoadVehShow.hour,
															OverLoadVehShow.min,
															OverLoadVehShow.sec);
			DrawGbText(cBuff, 47, 44);
			sprintf(cBuff,"%d.%dkm/h",OverLoadVehShow.speed/10,OverLoadVehShow.speed%10);
			DrawGbText(cBuff, 47, 66);
			sprintf(cBuff,"%d",OverLoadVehShow.AxleGroupNum);
			DrawGbText(cBuff, 167, 66);
			sprintf(cBuff,"%d",OverLoadVehShow.AxleNum);
			DrawGbText(cBuff, 47, 88);
			for(i=0;i<OverLoadVehShow.AxleNum;i++) {
				cBuff[i]= '0'+OverLoadVehShow.AxleType[i];
			}
			cBuff[i]= 0;
			DrawGbText(cBuff, 167, 88);
			if(GetCurUnit()) {
				sprintf(cBuff,"%0.2f %s",Kg2CurVal(OverLoadVehShow.TotalWet),GetCurUnitStr());
			} else {
				sprintf(cBuff,"%0.0f %s",Kg2CurVal(OverLoadVehShow.TotalWet),GetCurUnitStr());
			}
			DrawGbText(cBuff, 47, 110);
	
			if(OverLoadVehShow.OverWetFlag) {
				DrawGbText("超载", 200, 110);
			}
		}
		else 
		{
			DrawGbText("数据异常", 100, 110);
			sprintf(cBuff,"%d/%d",OverLoadVehNo,OverLoadDataPackNo);
			DrawGbText(cBuff, 230-strlen(cBuff)*7, 22);
		}
    }

	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}

//extern u8 SearchcheckUserData(u32 line_No,UserData_DCBType* User,IndependentWeight_DCBType* Weight);	//查询主存或辅存用户数据
void Form_OverloadQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
    OvertimeData *pOverLoadVeh=NULL;

	
	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);

			OverLoadVehNo = Get_Flash_OverWetCarNum();
			OverLoadDataPackNo = OverLoadVehNo;//总存储车辆信息的条数
            
		case WM_SHOW:
            if(OverLoadVehNo >= 1) 
            {
                pOverLoadVeh = (OvertimeData *)Query_Overweight_Vehicle(OverLoadVehNo,sizeof(OvertimeData));
                if(pOverLoadVeh) 
                {
                    memcpy((char *)&OverLoadVehShow,(char *)pOverLoadVeh,sizeof(OvertimeData));
                    if((pOverLoadVeh->AxleNum < 2) || (pOverLoadVeh->AxleNum > gWetPar.nMaxAxleNum)) 
                    {
                    	OverLoadDataSta = FALSE;
                    }
                    else
                    {
                    	OverLoadDataSta = TRUE;
                    }
                } 
                else 
                {
                    OverLoadDataSta = FALSE;
                }
            }
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
				case KEY_LEFT:
				case KEY_UP:
					if(OverLoadVehNo > 1)
					{
						OverLoadVehNo--;
					}
					
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;
				case KEY_RIGHT: 
				case KEY_DOWN:
					if(OverLoadVehNo < OverLoadDataPackNo)
					{
						OverLoadVehNo++;
					}		
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;


				case KEY_OK:
					if(OverLoadDataSta)
					{
						g_pCurWindow = &gWD_OverLoadDetailQuery;
			            g_pCurWindow->pParentWindow = &gWD_OverloadQuery;
						guiMsgInfo.pWindow = g_pCurWindow;
						guiMsgInfo.ID = WM_LOAD;
						guiMsgInfo.wParam = (uint32)&OverLoadVehShow;
						GuiMsgQueuePost(&guiMsgInfo); 
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

                case KEY_PRINT:
                    #if 0
					//打印数据
					printerQueryInfo.PrinterManagerID = PM_SEARCH;		 //手动打印
					if(pPrinterManagerEvent != NULL)
						OSQPost(pPrinterManagerEvent, (void*)&printerQueryInfo);	//发送打印消
                    #endif
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







