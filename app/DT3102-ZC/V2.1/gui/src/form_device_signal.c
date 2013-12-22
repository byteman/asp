#include "form_device_signal.h"
#include "Form_Main.h"
#include "Form_MsgBox.h"

#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "ioctrl.h"
#include "dt3102_io.h"

extern  CControl  gStatusBar; 		//状态栏
				  
static void Form_Device_Draw(LPWindow pWindow);
static void Form_Device_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


static char device_att_str[11][8] = {{0}};
static DeviceSignal sDeviceSignal = {0};

//定义属性页
static CAttrItem  _attritem[] = {
   {"光幕1信号电平",device_att_str[0],1},
   {"光幕1故障电平",device_att_str[1],2},
   {"光幕2信号电平",device_att_str[2],3},
   {"光幕2故障电平",device_att_str[3],4},
   {"地感1信号电平",device_att_str[4],5},
   {"地感1故障电平",device_att_str[5],6},
   {"地感2信号电平",device_att_str[6],7},
   {"地感2故障电平",device_att_str[7],8},
   {"轮轴信号电平",device_att_str[8],9},
   {"轮轴故障电平",device_att_str[9],10},
   {"轮轴双轮电平",device_att_str[10],11},
};

DEF_ATTR_CTRL(mDeviceSignalAttr, &gWD_DeviceSignal, 0, 20, 240, 108, "外设电平设置",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
	&gStatusBar,
	&mDeviceSignalAttr,
};

CWindow  gWD_DeviceSignal = {		 //窗体
    marrLPControl,
	sizeof(marrLPControl)/sizeof(LPControl),
    0,
    0,
    0,
    240,
    128,
    WS_VISABLE,
	NULL,//(pParentWindow)
    0,
    NULL,
    NULL,
    Form_Device_Draw,
    Form_Device_Proc
};

static void Show_Device_Signal(void)
{
	//光幕1
	if(sDeviceSignal.Signal.LC1EventSignal) {
		strcpy(device_att_str[0],"高电平");
	} else {
		strcpy(device_att_str[0],"低电平");
	}
	
	if(sDeviceSignal.Signal.LC1ErrSignal) {
		strcpy(device_att_str[1],"高电平");
	} else {
		strcpy(device_att_str[1],"低电平");
	}

	//光幕2
	if(sDeviceSignal.Signal.LC2EventSignal) {
		strcpy(device_att_str[2],"高电平");
	} else {
		strcpy(device_att_str[2],"低电平");
	}

	if(sDeviceSignal.Signal.LC2ErrSignal) {
		strcpy(device_att_str[3],"高电平");
	} else {
		strcpy(device_att_str[3],"低电平");
	}

	//前地感
	if(sDeviceSignal.Signal.GC1EventSignal) {
		strcpy(device_att_str[4],"高电平");
	} else {
		strcpy(device_att_str[4],"低电平");
	}

	
	if(sDeviceSignal.Signal.GC1ErrSignal) {
		strcpy(device_att_str[5],"高电平");
	} else {
		strcpy(device_att_str[5],"低电平");
	}

	//后地感
	if(sDeviceSignal.Signal.GC2EventSignal) {
		strcpy(device_att_str[6],"高电平");
	} else {
		strcpy(device_att_str[6],"低电平");
	}

	
	if(sDeviceSignal.Signal.GC2ErrSignal) {
		strcpy(device_att_str[7],"高电平");
	} else {
		strcpy(device_att_str[7],"低电平");
	}

	//轮轴识别器
	if(sDeviceSignal.Signal.TyreEventSignal) {
		strcpy(device_att_str[8],"高电平");
	} else {
		strcpy(device_att_str[8],"低电平");
	}

	if(sDeviceSignal.Signal.TyreErrSignal) {
		strcpy(device_att_str[9],"高电平");
	} else {
		strcpy(device_att_str[9],"低电平");
	}

	
	if(sDeviceSignal.Signal.TyreDoubleSignal) {
		strcpy(device_att_str[10],"高电平");
	} else {
		strcpy(device_att_str[10],"低电平");
	}
}

void Form_Device_Draw(LPWindow pWindow)
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

void Form_Device_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();							//清屏
			SysTimeDly(3);
			CTRL_CONTENT(mDeviceSignalAttr).focus = 0;	//光标停留位置

		case WM_SHOW:
			//获取默认的设备电平值
			sDeviceSignal.value = Get_Device_Signal();
			//显示设备电平
			Show_Device_Signal();
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
                    if (CTRL_CONTENT(mDeviceSignalAttr).focus > 0) {
                        CTRL_CONTENT(mDeviceSignalAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mDeviceSignalAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mDeviceSignalAttr).focus + 1 < CTRL_CONTENT(mDeviceSignalAttr).total) {
                        CTRL_CONTENT(mDeviceSignalAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mDeviceSignalAttr, 0);
                    }
					break;

				case KEY_LEFT:
					sDeviceSignal.value |= (1 << CTRL_CONTENT(mDeviceSignalAttr).focus);
					Show_Device_Signal();
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mDeviceSignalAttr, 0);
					break;

				case KEY_OK:
					if(Set_Device_Signal(sDeviceSignal.value))
					{
						MsgBoxDlg(&g_MsgBoxDlg, "成功提示", "参数设置保存成功!");
					}
					else
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置保存失败,请重新设置");
					}
					break;

				case KEY_RIGHT: 
					sDeviceSignal.value &= (~(1 << CTRL_CONTENT(mDeviceSignalAttr).focus));
					Show_Device_Signal();
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mDeviceSignalAttr, 0);
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





