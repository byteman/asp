#include "Form_USB_update.h"
#include "Form_Main.h"
#include "Form_MainMenu.h"
#include "Form_MsgBox.h"
#include "includes.h"
#include "Form_Main.h"
#include "Form_Resource.h"
#include "includes.h"
#include "system_init.h"

#include "driver.h"
#include "task_def.h"
#include "task_timer.h"
#include "key_thread.h"
#include "timer2.h"
#include "debug_info.h"


//错误类型
static int8* errnoArr[]={
	"初始化出错",			// 0
	"挂载或获取文件出错",	// 1 		//usb device errno information
	"打开文件出错,请检查文件",// 2
	"CRC文件出错",			// 3
	"读取升级文件不完全",	// 4
	"CRC校验出错",			// 5
	"升级文件过大",			// 6
	"擦除或存储FLASH出错",	// 7
	"按确认键开始",			// 8		//gui information
	"开始检测U盘,请耐心等待",			// 9
	"按键不可用,升级中...",	// 10
	"未检测到U盘,请重新插入U盘",	// 11
	"升级成功,新系统重启后生效",	// 12
	"升级失败",						// 13
	"升级超时,请拔出U盘后重新开始"	// 14
};

volatile static u8 count=0;	//定时器计数
static u8 OverTimeFlag=0;	//0-未超时，1-已超时
static HANDLE Tim2_dev=HANDLENULL;
extern  CControl  gStatusBar; 		//状态栏

static void Form_USB_Updata_Draw(LPWindow pWindow);
static void Form_USB_Updata_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


DEF_TEXT_LABEL(mInformation, &gWD_USB_Updata, 10, 60, 90, 20, CTRL_VISABLE,NULL);
DEF_TEXT_LABEL(mTips, &gWD_USB_Updata, 10, 90, 90, 20,0,"如果6秒后没有提示,请拔出u盘!");//"可按确认键重新开始");


static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mInformation,
	&mTips
};

CWindow  gWD_USB_Updata = {
    (LPControl *)marrLPControl,
	sizeof(marrLPControl)/sizeof(LPControl),
    0,
    0,
    0,
    240,
    128,
    WS_VISABLE,
	NULL,//(pParentWindow)
    1,//1 tick=10ms
    NULL,
    NULL,			//Form_USB_Updata_Timer,
    Form_USB_Updata_Draw,
    Form_USB_Updata_Proc
};


//system timer to check updata time out
static void USB_timeout(void)
{
//	GuiMsgInfo guiMsgInfo={WM_SHOW,0xf0f0,14};
	if(++count==60)		//10s
	{
		Key_Enable(TRUE);	//open key	
//		OverTimeFlag=1;		
//		GuiMsgQueuePost(&guiMsgInfo);
	}
}
static void Form_USB_Updata_Draw(LPWindow pWindow)
{
	uint16 i;
	LPControl	lpControl;
	
	EraseBuffer();//清除显存		
	SetRedraw(FALSE);//禁止绘图
	EnableScreenFlush(FALSE);//禁止刷屏
	//设置视图
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

    DrawGbText("一键式U盘升级", 0, 25);
    DrawHoriLine(0, 25+16, 240);
    DrawHoriLine(0, 25+18, 240);
	for(i = 0; i < pWindow->nNbControls; i++)//绘制控件
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
			lpControl->DrawFunc(lpControl);
		}
	}	
	EnableScreenFlush(TRUE);//使能刷屏	
	FlushScreen();//刷屏
	SetRedraw(TRUE);//使能绘图
}

static void Form_USB_Updata_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
	u8 err;
	switch(pGuiMsgInfo->ID)	{
	case WM_LOAD:
		ClearScreen();
		mInformationCC.s="按确认键开始";
		SysTimeDly(15);
		OverTimeFlag=0;
		if(Tim2_dev==HANDLENULL && TIM2_Config(USB_timeout,200,Mode_500US)==1)
		{
			Tim2_dev=DeviceOpen(CHAR_TIM2,0,&err);		
			if(err!=HVL_NO_ERR)
			{
				debug(Debug_Error,"create hard timer error!!!\r\n");
			}
		}
	case WM_SHOW:
		//mInformationCC.s=errnoArr[8];	//default;
		mTips.state=0;
		if(pGuiMsgInfo->wParam==0xf0f0)	//post by usb thread or OK key
		{
			mInformationCC.s = (char*)errnoArr[pGuiMsgInfo->lParam];
			if(pGuiMsgInfo->lParam==9)		//开始检测,由OK键触发
			{
				Key_Enable(FALSE);			//close key
				SysTaskResume(USBPrio);		//resume usb therad			
			}
			else if(pGuiMsgInfo->lParam==10) //检测到U盘,开始升级并计时
			{
				mTips.state=CTRL_VISABLE;
				if(DeviceIoctl(Tim2_dev,TIM2_CMDTimOpen,0)==0)	//开启定时器
				{
					debug(Debug_Error,"start timer 6 error!!!\r\n");
				}
// 				debug(Debug_Notify,"start tick:%d\r\n",Get_Sys_Tick());
			}
			else
			{
				//hard tiemr
				if(DeviceIoctl(Tim2_dev,TIM2_CMDTimClose,0)==0)
				{
					debug(Debug_Error,"stop timer 6 error!!!\r\n");;
				}
				DeviceIoctl(Tim2_dev,TIM2_CMDClearCnt,0);
				count=0;
				if(pGuiMsgInfo->lParam!=14)
				{
//					mTips.state=CTRL_VISABLE;
					Key_Enable(TRUE);	//open key
				}
				if(pGuiMsgInfo->lParam<8)
					OverTimeFlag=0;
// 				debug(Debug_Notify,"end tick:%d\r\n",Get_Sys_Tick());
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
		switch(pGuiMsgInfo->wParam)	{
		case KEY_OK:
			if(OverTimeFlag)
				break;
			guiMsgInfo.ID=WM_SHOW;
			guiMsgInfo.wParam=0xf0f0;
			guiMsgInfo.lParam=9;
			guiMsgInfo.pWindow = pWindow;
			GuiMsgQueuePost(&guiMsgInfo);
			break;
		case KEY_BACK:
			if(pWindow->pParentWindow != NULL)
			{
				g_pCurWindow =pWindow->pParentWindow;	//&gWD_TimeDate;
			}
			else
			{
				g_pCurWindow = &gWD_Main;
			}
			PostWindowMsg(g_pCurWindow, WM_LOAD,0,0);
			guiMsgInfo.pWindow = g_pCurWindow;			
			guiMsgInfo.ID = WM_LOAD;
			GuiMsgQueuePost(&guiMsgInfo);
			break;
		default:
			break;
		}
	}
}



