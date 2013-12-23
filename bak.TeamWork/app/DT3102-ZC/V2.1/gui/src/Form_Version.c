#include "Form_Version.h"
#include "Form_Main.h"
#include "sys_param.h"
#include "sys_config.h"

#include "system_init.h"


extern  CControl  gStatusBar; 		//状态栏

static void Form_Version_Timer(void *ptmr, void *parg);
static void Form_Version_Draw(LPWindow pWindow);
static void Form_Version_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

__align(8) static char gCpuID[16]={0};
__align(8) static char mUpDate_XHval[8]={0};		  	//2个字符	ASCII
__align(8) static char mUpDate_VersionVal[24]={0};	  	//10个字符
__align(8) static char mUpDate_PlatformVal[24]={0};	  	//10个字符
__align(8) static char mUpDate_NumVal[32]={0};	  		//6*4(24字节)个字符	ASCII
__align(8) static char mUpDate_Mode[16] = {0};

DEF_TEXT_LABEL(mUpDate_XH_Title, &gWD_Version, 2, 25, 150, 18, CTRL_VISABLE, "终端型号：");
DEF_TEXT_LABEL(mUpDate_XHVal_Title, &gWD_Version, 75, 25, 150, 18, CTRL_VISABLE, mUpDate_XHval);
DEF_TEXT_LABEL(mUpDate_Platform_Tile, &gWD_Version, 2, 50, 150, 18, CTRL_VISABLE, "平台版本号：");
DEF_TEXT_LABEL(mUpDate_PlatformVal_Tile, &gWD_Version, 83, 50, 150, 18, CTRL_VISABLE, mUpDate_PlatformVal);

DEF_TEXT_LABEL(mUpDate_Version_Title, &gWD_Version, 2, 70, 150, 18, CTRL_VISABLE, "软件版本号：");
DEF_TEXT_LABEL(mUpDate_VersionVal_Title, &gWD_Version, 83, 70, 150, 18, CTRL_VISABLE, mUpDate_VersionVal);
DEF_TEXT_LABEL(mUpDate_Num_Title, &gWD_Version, 2, 90, 150, 18, CTRL_VISABLE, "机器码：");
DEF_TEXT_LABEL(mUpDate_NumVal_Title, &gWD_Version, 55, 90, 150, 18, CTRL_VISABLE,mUpDate_NumVal);
DEF_TEXT_LABEL(mUpDate_Mode_Title, &gWD_Version, 2, 110, 150, 18, CTRL_VISABLE, "计量模式:");
DEF_TEXT_LABEL(mUpDate_ModeVal_Title, &gWD_Version, 68, 110, 150, 18, CTRL_VISABLE,mUpDate_Mode);


static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mUpDate_XH_Title,
	&mUpDate_XHVal_Title,
	&mUpDate_Platform_Tile,
	&mUpDate_PlatformVal_Tile,
	&mUpDate_Version_Title,
	&mUpDate_VersionVal_Title,
	&mUpDate_Num_Title,
	&mUpDate_NumVal_Title,
	&mUpDate_Mode_Title,
	&mUpDate_ModeVal_Title
};

CWindow  gWD_Version = {
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
    Form_Version_Timer,
    Form_Version_Draw,
    Form_Version_Proc
};


void Form_Version_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_Version_Draw(LPWindow pWindow)
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
    DrawHoriLine(0, 25+16, 240);
    DrawHoriLine(0, 25+18, 240);

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

/****************************************************************************
* 名	称：char HexToAscii(u8 nHex)
* 功	能：单个16进制转ASCII码		   
* 入口参数：nHex	0x0A-0x0F.0x00-0x09
* 出口参数：转换后的ASCII码
* 说	明：16进制0x0A-0x0F.0x00-0x09转ASCII码A-F.0-9
****************************************************************************/
char HexToAscii(u8 nHex)
{
	if(nHex <= 9)	//数字0~9
	{
		return (nHex + 0x30);
	}
	else
	{
		if(nHex >= 0x0A && nHex <= 0x0F)	//字母A~F
		{
			return (nHex + 0x37);
		}
		else
		{
			return 0xFF;
		}
	}
}

/****************************************************************************
* 名	称：void BufferHexToAscii(u8 *pHexBuf,u8 *pAaciiBuf , u16 nHexLen)
* 功	能：多个16进制转ASCII码	   
* 入口参数：*pAaciiBuf	ASCII码存放数组	   结果
			*pHexBuf	16进制数据数组	   源
			nHexLen	转换个数
* 出口参数：无
* 说	明：u8 x[2]={0xAB,0XCD};
			u8 z[4]={0};		转换后结果{65,66,67,68}		 2位转4位
****************************************************************************/
void BufferHexToAscii(u8 *pHexBuf,u8 *pAaciiBuf , u16 nHexLen)
{
	u16 i;
	if(pAaciiBuf == 0)
		return;
	if(pHexBuf == 0)
		return;

	for(i=0; i<nHexLen; i++)
	{
		pAaciiBuf[i<<1] = HexToAscii(pHexBuf[i]>>4);
		if(pAaciiBuf[i] == 0xFF)
			return;

		
		pAaciiBuf[(i<<1)+1] = HexToAscii(pHexBuf[i] & 0x0F);
		if(pAaciiBuf[(i<<1)+1] == 0xFF)
			return;
	}
}

void Form_Version_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{

	CControl* pControl;


	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
    		
		case WM_SHOW:		
			GetCpuIdHex((u8*)gCpuID);
			//型号
			strcpy(mUpDate_XHval,"DT3101");
		
			Get_EMOS_Versions((u8*)mUpDate_PlatformVal);//获得平台的发布版本号
			//版本
			snprintf(mUpDate_VersionVal, 23, "DT3101-V%d.%d.%d", 
				AppVersion1, AppVersion2, AppVersion3);
			//机器码
			BufferHexToAscii((u8*)gCpuID,(u8*)&mUpDate_NumVal,12);	 //3*4
			//计量模式
			snprintf(mUpDate_Mode, 15, "SCS-ZC-%d", (Get_System_Mode() + 1));
			
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



