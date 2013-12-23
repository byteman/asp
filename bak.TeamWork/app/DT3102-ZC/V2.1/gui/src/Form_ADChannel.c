#include "includes.h"
#include "Form_ADChannel.h"

#include "Ctr_StatusBar.h"
#include "Form_Main.h"

#include "Form_Resource.h"
#include "TextEdit.h"
#include "Menu.h"
#include "gdi.h"

#include "wet_algorim.h"		//整车式
#include "sys_config.h"
#include "weight_param.h"
#include "task_timer.h"


#include "Form_MsgBox.h"

#include "string.h"

/*//称重数据接口
 *ad通道由称重计量模式决定
 *SCS_ZC_1:
	PreAxle1AD ScalerAD有效; PreAxle1AD表示秤台第一路AD, ScalerAD表示秤台第二路AD
 *SCS_ZC_2:
	PreAxle1AD ScalerAD有效; PreAxleAD表示前轴识别器AD, ScalerAD表示秤台AD 
 *SCS_ZC_3:
	PreAxle1AD ScalerAD BackAxle有效; PreAxleAD表示前轴识别器AD, ScalerAD表示秤台AD,BackAxle表示后轴识别器AD
 *SCS_ZC_5,SCS_ZC_6:
	PreAxle1AD PreAxle2AD ScalerAD BackAxle有效; PreAxle1AD,PreAxle2AD表示小秤台AD, ScalerAD表示秤台AD,BackAxle表示后轴识别器AD
	
typedef struct {
	s32 Wet;				//整秤分度重量
	s32 Axle1_Wet;			//前轴轴重
  s32 Axle2_Wet;			//后轴轴重
	u32 PreAxle1AD;			//ad值
	u32 PreAxle2AD;
  u32 ScalerAD;
	u32 BackAxleAD;
	u8  ScalerStableFlag;				//秤台稳定标志
	u8	ScalerZeroFlag;				//秤台零位标志
}WetStatue; 

*/



extern  CControl  gStatusBar; 		//状态栏

static float sADChannel1_Value   = 0.0;
static float sADChannel2_Value   = 0.0;
static float sADChannel3_Value   = 0.0;
static float sADChannel4_Value   = 0.0;

 static __align(8) char sADChannel_Title[24] = {0};


//static long *pADValue1,*pADValue2;

static void Form_ADChannel_Timer(void *ptmr, void *parg);
static void Form_ADChannel_Draw(LPWindow pWindow);
static void Form_ADChannel_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);
 
DEF_SMALL_FLOAT_LABLE(mADChannel1,   &gWD_ADChannel, 70,  44, 	170,  14, CTRL_VISABLE, &sADChannel1_Value, 4,  -5000000, 5000000,    "", TA_LEFT);
DEF_SMALL_FLOAT_LABLE(mADChannel2,   &gWD_ADChannel, 70,  66, 	170,  14, CTRL_VISABLE, &sADChannel2_Value, 4,  -5000000, 5000000,    "", TA_LEFT);
DEF_SMALL_FLOAT_LABLE(mADChannel3,   &gWD_ADChannel, 70,  88, 	170,  14, CTRL_VISABLE, &sADChannel3_Value, 4,  -5000000, 5000000,    "", TA_LEFT);
DEF_SMALL_FLOAT_LABLE(mADChannel4,   &gWD_ADChannel, 70,  110, 	170,  14, CTRL_VISABLE, &sADChannel4_Value, 4,  -5000000, 5000000,    "", TA_LEFT);




static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mADChannel1,
    &mADChannel2,
	&mADChannel3,
	&mADChannel4,
};

CWindow  gWD_ADChannel = {
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
    Form_ADChannel_Timer,
    Form_ADChannel_Draw,
    Form_ADChannel_Proc
};

static WetStatue wet_chanle = {0};

void Form_ADChannel_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	
#if SYS_CRITICAL_METHOD == 3  	 
    SYS_CPU_SR  cpu_sr = 0;
#endif

	Get_Scaler_WetPar(&wet_chanle);
	if(lpWindow != NULL) {
    	if(g_pCurWindow == &gWD_ADChannel) 
		{
			SYS_ENTER_CRITICAL();
			sADChannel1_Value = wet_chanle.ScalerAD;
			sADChannel2_Value = wet_chanle.BackAxleAD;		//
			sADChannel3_Value = wet_chanle.PreAxle1AD;
			sADChannel4_Value = wet_chanle.PreAxle2AD;   //
			SYS_EXIT_CRITICAL();

 			PostWindowMsg(lpWindow, WM_UPDATECTRL, 0, 0);		
    	}
    }
}

void Form_ADChannel_Draw(LPWindow pWindow)
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
    
    DrawGbText("AD通道1",5,44);
    DrawGbText("AD通道2",5,66);
    DrawGbText("AD通道3",5,88);
    DrawGbText("AD通道4",5,110);

    snprintf(sADChannel_Title, 23, "通道的AD值 SCS-ZC-%d", (Get_System_Mode() + 1));

	DrawGbText(sADChannel_Title, 5, 21);
    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);
    DrawHoriLine(0, 106, 240);
    DrawHoriLine(0, 127, 240);
    DrawVertLine(60, 40, 88);

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

void Form_ADChannel_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			WeightFregDiv(TRUE, 10);
			SysTimeDly(3);
			CreateWindowTimer(&gWD_ADChannel);
			StartWindowTimer(&gWD_ADChannel);

		case WM_SHOW:
			pWindow->DrawFunc(pWindow);
			break;

		case WM_UPDATECTRL:
			mADChannel1.DrawFunc(&mADChannel1);
			mADChannel2.DrawFunc(&mADChannel2);
			mADChannel3.DrawFunc(&mADChannel3);
			mADChannel4.DrawFunc(&mADChannel4);
			break;

		case WM_TIMEUPDATE:
//			gWD_ADChannel.DrawFunc(&gWD_ADChannel);
			gStatusBar.DrawFunc(&gStatusBar);
//			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
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
				    //MsgBoxDlg(&g_MsgBoxDlg, "整车式零点标定成功", "将进入整车式砝码重量输入流程");
//				
//					g_pCurWindow = &gWD_CalibrateInput;					//时间
//					g_pCurWindow->pParentWindow = &gWD_CalibrateWeight;
//					PostWindowMsg(WM_LOAD, NULL, NULL);	
//				
					break;

				case KEY_RIGHT: 
					break;

				case KEY_BACK:
					WeightFregDiv(FALSE, 0);
          			StopWindowTimer(&gWD_ADChannel);
//	           		 CalibrateProc();
//	           		 CalibParamSync();
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



