#include "Form_OverLoadDetailQuery.h"
#include "Form_Main_Axle.h"
#include "Ctr_ListSelect.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Form_Main.h"

#include "Form_OverLoadQuery.h"
#include "Common.h"

#include "overtime_queue.h"
#include "data_store_module.h"
#include "sys_param.h"



static OvertimeData  OverLoadDetailVehShow;
static s8  AxleGroupShow = 0;

static void Form_OverLoadDetailQuery_Timer(void *ptmr, void *parg);
static void Form_OverLoadDetailQuery_Draw(LPWindow pWindow);
static void Form_OverLoadDetailQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


static LPControl marrLPControl[] = 
{
    &gStatusBar,
};

CWindow  gWD_OverLoadDetailQuery = {
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
    Form_OverLoadDetailQuery_Timer,
    Form_OverLoadDetailQuery_Draw,
    Form_OverLoadDetailQuery_Proc
};

void Form_OverLoadDetailQuery_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_OverLoadDetailQuery_Draw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;
	char cBuff[36];
//    u8 ucTemp;

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

    if(AxleGroupShow < 0) {
    	DrawGbText("轴数: ", 5, 22);
//     	DrawGbText("轴组重: ", 100, 22);
    	DrawGbText("轴", 18, 44);
    	DrawGbText("轴重", 71, 44);
    	DrawGbText("轴型", 131, 44);
    	DrawGbText("轴距", 191, 44);
    } else {
		sprintf(cBuff,"轴数: %d/%d",(AxleGroupShow),OverLoadDetailVehShow.AxleGroupNum);
		DrawGbText(cBuff, 5, 22);
			
		sprintf(cBuff,"%d",AxleGroupShow);				//轴
		DrawGbText(cBuff, 18, 66);
			
		sprintf(cBuff,"%d",OverLoadDetailVehShow.AxleWet[AxleGroupShow-1]);				//轴重
		DrawGbText(cBuff, 71, 66);
			
		if((AxleGroupShow) == 1)															//判断轴型
		{
			DrawGbText("单轮", 131, 66);													//第一个轴为单轮
		}
		else
			DrawGbText("双轮", 131, 66);													//其它轴型默认为双轮
			
		sprintf(cBuff,"%d m",(int)(OverLoadDetailVehShow.AxleLen[AxleGroupShow-1]*0.01f));				//轴距
			DrawGbText(cBuff, 191, 66);
		
		DrawGbText("轴", 18, 44);
		DrawGbText("轴重", 71, 44);
		DrawGbText("轴型", 131, 44);
		DrawGbText("轴距", 191, 44);			 
    }
    
    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);

    DrawVertLine(50, 40, 44);
    DrawVertLine(120, 40, 44);
    DrawVertLine(170, 40, 44);


	//使能刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//使能绘图
	SetRedraw(TRUE);
}
void Form_OverLoadDetailQuery_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
			Get_OverLoadVehShow(&OverLoadDetailVehShow);
			if(OverLoadDetailVehShow.AxleGroupNum) 
			{
				AxleGroupShow = 1;
			} 
			else
			{
				AxleGroupShow = 0;
			}

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

				case KEY_OK:
					break;

				case KEY_RIGHT: 
					if(AxleGroupShow < OverLoadDetailVehShow.AxleNum )
					{
						AxleGroupShow++;
					}		
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;

				case KEY_LEFT:
					if(AxleGroupShow>1)
					{
						AxleGroupShow--;
					}		
					PostWindowMsg(pWindow, WM_SHOW, 0, 0);
					break;
                    
				case KEY_BACK:
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = pWindow->pParentWindow;
                        PostWindowMsg(g_pCurWindow, WM_SHOW,0,0);
					}
					else
					{
						g_pCurWindow = &gWD_Main;
                        PostWindowMsg(g_pCurWindow, WM_LOAD,0,0);
					}
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



