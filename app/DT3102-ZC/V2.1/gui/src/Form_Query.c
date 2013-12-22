#include "Form_Query.h"
#include "Form_OverloadQuery.h"
#include "Ctr_ListSelect.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Form_Main.h"
#include "Form_DetailQuery.h"

#include "Common.h"
#include "overtime_queue.h"
#include "data_store_module.h"
#include "sys_param.h"

#include "../Inc/Menu.h"


extern  CControl  gStatusBar; 		//状态栏


static uint16 DataPackNo;
static uint16  CarIndex = 0;
static uint16  VehNo = 0;


static OvertimeData  VehShow;

static BOOL  DataSta=FALSE;


/*
 * 查询菜单
 */
DEF_MENU_ITEM(mnNormalData,       "正常数据",     0, NULL);
DEF_MENU_ITEM(mnOverloadData,       "超限数据",     1, NULL);

static LPMenuItem	m_DataMenuItems[] = 
{
	&mnNormalData,
	&mnOverloadData,	

};
void FormDataMenuProc(struct _CWindow* pParentWindow, uint32 nCmd);

DEF_MENU(mDataMenu, "数据查询", m_DataMenuItems, NULL, FormDataMenuProc);	 
DEF_MENU_WND(gWD_DataMenu, &mDataMenu, NULL);


void FormDataMenuProc(struct _CWindow* pParentWindow, uint32 nCmd)
{
	switch(nCmd)
	{
		case 0:
          g_pCurWindow = &gWD_NormalQuery;
    			g_pCurWindow->pParentWindow = &gWD_DataMenu;
    			PostWindowMsg(g_pCurWindow, WM_LOAD, 1, NULL);	
            
			break;
        
		case 1:
          g_pCurWindow = &gWD_OverloadQuery;
    			g_pCurWindow->pParentWindow = &gWD_DataMenu;
    			PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	

			break;
        

            
		default:
			break;
	}
}




static void Form_NormalQuery_Timer(void *ptmr, void *parg);
static void Form_NormalQuery_Draw(LPWindow pWindow);
static void Form_NormalQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

DEF_TEXT_LABEL(mQuery_Title,    &gWD_NormalQuery, 0, 22, 60, 14, CTRL_VISABLE, "正常数据查询");
DEF_TEXT_LABEL(mTime_Title,     &gWD_NormalQuery, 5, 44, 30, 18, CTRL_VISABLE, "时间");
DEF_TEXT_LABEL(mSpeed_Title,    &gWD_NormalQuery, 5, 66, 30, 18, CTRL_VISABLE, "车速");
DEF_TEXT_LABEL(mAxleGroup_Title,&gWD_NormalQuery,125,66, 30, 18, CTRL_VISABLE, "轴组");
DEF_TEXT_LABEL(mAxleNum_Title,  &gWD_NormalQuery, 5, 88, 30, 18, CTRL_VISABLE, "轴数");
DEF_TEXT_LABEL(mAxleType_Title, &gWD_NormalQuery,125,88, 30, 18, CTRL_VISABLE, "轴型");
DEF_TEXT_LABEL(mWet_Title,      &gWD_NormalQuery, 5, 110, 30, 18, CTRL_VISABLE,"总重");

void Get_VehShow(OvertimeData *pData)
{
	memcpy(pData, &VehShow, sizeof(OvertimeData));
}
static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mQuery_Title,
    &mTime_Title,
    &mSpeed_Title,
    &mAxleGroup_Title,
    &mAxleNum_Title,
    &mAxleType_Title,
    &mWet_Title,
};

CWindow  gWD_NormalQuery = {
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
    Form_NormalQuery_Timer,
    Form_NormalQuery_Draw,
    Form_NormalQuery_Proc
};

void Form_NormalQuery_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_NormalQuery_Draw(LPWindow pWindow)
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
	
    if(DataPackNo < 1) 
    {
	    DrawGbText("无记录", 192, 22);
    } 
    else 
    {
		if(DataSta)
		{
			sprintf(cBuff,"%d/%d",CarIndex,DataPackNo);
			DrawGbText(cBuff, 230-strlen(cBuff)*7, 22);
			sprintf(cBuff,"%d年%02d月%02d日%02d:%02d:%02d",VehShow.year,
												VehShow.mon,
												VehShow.day,
												VehShow.hour,
												VehShow.min,
												VehShow.sec);
			DrawGbText(cBuff, 47, 44);
			sprintf(cBuff,"%d.%dkm/h",VehShow.speed/10, VehShow.speed%10);
			DrawGbText(cBuff, 47, 66);
			sprintf(cBuff,"%d",VehShow.AxleGroupNum);
			DrawGbText(cBuff, 167, 66);
			sprintf(cBuff,"%d",VehShow.AxleNum);
			DrawGbText(cBuff, 47, 88);
			for(i=0;i<VehShow.AxleNum;i++) {
				cBuff[i]= '0'+VehShow.AxleType[i];
			}
			cBuff[i]= 0;
			DrawGbText(cBuff, 167, 88);
			if(GetCurUnit()) {
				sprintf(cBuff,"%0.2f %s",Kg2CurVal(VehShow.TotalWet),GetCurUnitStr());
			} else {
				sprintf(cBuff,"%0.0f %s",Kg2CurVal(VehShow.TotalWet),GetCurUnitStr());
			}
			DrawGbText(cBuff, 47, 110);
			
		} 
		else 
		{
			DrawGbText("数据异常", 100, 110);
			sprintf(cBuff,"%d/%d",CarIndex,DataPackNo);
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
void Form_NormalQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
    OvertimeData *pVeh=NULL;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
			DataPackNo = Get_Flash_NormalCarNum();
			CarIndex = DataPackNo;
			VehNo = Get_Flash_NormalCarNumRear();
			if(VehNo == 0) VehNo = MAX_NORMAL_VEHICLE_NUMBER;
     		SysTimeDly(2);
            
		case WM_SHOW:
            if(DataPackNo >= 1) 
            {
                pVeh = (OvertimeData *)Query_Normal_Vehicle(VehNo, sizeof(OvertimeData));
                if(pVeh) 
                {
                    memcpy((char *)&VehShow,(char *)pVeh,sizeof(OvertimeData));
                    if((pVeh->AxleNum < 2) || (pVeh->AxleNum > 8))
                    {
                    	DataSta = FALSE;
                    }
                    else
                    {
                    	DataSta = TRUE;
                    }
                } 
                else 
                {
                    DataSta = FALSE;
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
					if(CarIndex > 1)
					{
						CarIndex--;
						VehNo--;
						if(VehNo == 0) VehNo = MAX_NORMAL_VEHICLE_NUMBER;
					}		
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;
				case KEY_RIGHT: 
				case KEY_DOWN:
					if(CarIndex < DataPackNo)
					{
						CarIndex++;
						VehNo++;
						if(VehNo > MAX_NORMAL_VEHICLE_NUMBER) VehNo = 1;
					}		
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;


				case KEY_OK:
					if(DataSta)
					{
						g_pCurWindow = &gWD_DetailQuery;
                        g_pCurWindow->pParentWindow = &gWD_NormalQuery;
						guiMsgInfo.pWindow = g_pCurWindow;
						guiMsgInfo.ID = WM_LOAD;
						guiMsgInfo.wParam = (uint32)&VehShow;
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





