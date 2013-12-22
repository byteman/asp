#include "Form_OutPortTest.h"
#include "Form_Main.h"
// #include "SysParam.h"
#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "ioctrl.h"

extern  CControl  gStatusBar; 		//×´Ì¬À¸

extern OUTTEST	OutTest;
				  
static void Form_Warning_Timer(void *ptmr, void *parg);
static void Form_Warning_Draw(LPWindow pWindow);
static void Form_Warning_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);
void OutPortState(void);


static char attritemstr[5][20];

//¶¨ÒåÊôÐÔÒ³
static CAttrItem  _attritem[] = {
   {"Êä³ö×´Ì¬1",attritemstr[0],1},
   {"Êä³ö×´Ì¬2",attritemstr[1],2},
   {"Êä³ö×´Ì¬3",attritemstr[2],3},
   {"Êä³ö×´Ì¬4",attritemstr[3],4},

};

DEF_ATTR_CTRL(mOutPortAttr, &gWD_OutPortTest, 0, 20, 240, 108, "Êä³öµ÷ÊÔ×´Ì¬",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
  &gStatusBar,
	&mOutPortAttr,
};

CWindow  gWD_OutPortTest = {		 //´°Ìå
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
    Form_Warning_Timer,
    Form_Warning_Draw,
    Form_Warning_Proc
};

void Form_Warning_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;

	if(lpWindow != NULL)
	{

	}
}

void Form_Warning_Draw(LPWindow pWindow)
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

	//»æÖÆ¿Ø¼þ
	for(i = 0; i < pWindow->nNbControls; i++)
	{
		lpControl = *(pWindow->pLPControls + i);
		if(lpControl->state & CTRL_VISABLE)
		{
			lpControl->DrawFunc(lpControl);
		}
	}
	//Ê¹ÄÜË¢ÆÁ
	EnableScreenFlush(TRUE);

	//Ë¢ÆÁ
	FlushScreen();

	//Ê¹ÄÜ»æÍ¼
	SetRedraw(TRUE);
}

void Form_Warning_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();							//ÇåÆÁ
			SysTimeDly(3);
			CTRL_CONTENT(mOutPortAttr).focus = 0;	//¹â±êÍ£ÁôÎ»ÖÃ
// 			CreateWindowTimer(&gWD_OutPortTest);
// 			StartWindowTimer(&gWD_OutPortTest);

		case WM_SHOW:	
			OutPortTest();
			OutPortState();
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
// 			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
			gStatusBar.DrawFunc(&gStatusBar);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_UP:
                    if (CTRL_CONTENT(mOutPortAttr).focus > 0) {
                        CTRL_CONTENT(mOutPortAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOutPortAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mOutPortAttr).focus + 1 < CTRL_CONTENT(mOutPortAttr).total) {
                        CTRL_CONTENT(mOutPortAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOutPortAttr, 0);
                    }
					break;

				case KEY_LEFT:
					if(0 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_SetBits( GPIOE , GPIO_Pin_0 );
					}

					if(1 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_SetBits( GPIOE , GPIO_Pin_1 );
					}

					if(2 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_SetBits( GPIOE , GPIO_Pin_2 );
					}

					if(3 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_SetBits( GPIOE , GPIO_Pin_3 );
					}
					OutPortTest();
					OutPortState();
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOutPortAttr, 0);
					break;

				case KEY_OK:
					break;

				case KEY_RIGHT: 
					if(0 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_ResetBits( GPIOE , GPIO_Pin_0 );
					}

					if(1 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_ResetBits( GPIOE , GPIO_Pin_1 );
					}

					if(2 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_ResetBits( GPIOE , GPIO_Pin_2 );
					}

					if(3 == CTRL_CONTENT(mOutPortAttr).focus )
					{
						GPIO_ResetBits( GPIOE , GPIO_Pin_3 );
					}
					OutPortTest();
					OutPortState();
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mOutPortAttr, 0);
					break;

				case KEY_BACK:
					StopWindowTimer(&gWD_OutPortTest);
					DestoryWindowTimer(&gWD_OutPortTest);
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

void OutPortState(void)
{
			if(OutTest.Out0) {
                sprintf(attritemstr[0],"¸ß");
			} else {
                sprintf(attritemstr[0],"µÍ");
			}
			
			if(OutTest.Out1) {
                sprintf(attritemstr[1],"¸ß");
			} else {
                sprintf(attritemstr[1],"µÍ");
			}
			
			if(OutTest.Out2) {
                sprintf(attritemstr[2],"¸ß");
			} else {
                sprintf(attritemstr[2],"µÍ");
			}
			
			if(OutTest.Out3) {
                sprintf(attritemstr[3],"¸ß");
			} else {
                sprintf(attritemstr[3],"µÍ");
			}	
}


