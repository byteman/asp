/* 
 * History:
 * 1. 2013-1-8 缩小超级密码的使用域;
 */
#include "Form_PassWord.h"
#include "Ctr_Attr.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "../Inc/TextEdit.h"
#include "Form_Main.h"
#include "form_mainmenu.h"
#include "Form_MsgBox.h"
#include "Common.h"

#include "sys_param.h"

extern  CControl  gStatusBar;
extern  SSYSSET gSysSet; // 0:kg  1:t

//超级密码
static const char* sSuperPwd = "666888";

static void Form_PassWord_Timer(void *ptmr, void *parg);
static void Form_PassWord_Draw(LPWindow pWindow);
static void Form_PassWord_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

static char oldpw[20]  = "";
static char newpw1[20] = "";
static char newpw2[20] = "";
static char pwinfo[10] = "";
static CAttrItem  _attritem[] = {
   {"原密码",oldpw,1},
   {"新密码",newpw1,2},
   {"密码确认",newpw2,3},
   {"保存密码",pwinfo,4},
};

DEF_ATTR_CTRL(mPassWord, &gWD_PassWord, 0, 20, 240, 108, "密码设置",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mPassWord,
};

CWindow  gWD_PassWord = {
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
    Form_PassWord_Timer,
    Form_PassWord_Draw,
    Form_PassWord_Proc
};

void Form_PassWord_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_PassWord_Draw(LPWindow pWindow)
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

static void Check_User_Pwd(const char *psw1, const char *psw2)
{
	if(strcmp(psw1,psw2))  {
        MsgBoxDlg(&g_MsgBoxDlg,"错误提示","两次输入的新密码不一致");
    }else {
        strcpy(pwinfo,"已保存");
        strcpy((char *)gSysSet.password,psw1);
        SysSetParamSave();
        MsgBoxDlg(&g_MsgBoxDlg,"提示信息","密码已保存");
    }
}

void Form_PassWord_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
    static char   cPw0[20]="";
    static char   cPw1[20]="";
    static char   cPw2[20]="";
    char   len;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
			CTRL_CONTENT(mPassWord).focus = 0;
            memset(oldpw,0,sizeof(oldpw));
            memset(newpw1,0,sizeof(newpw1));
            memset(newpw2,0,sizeof(newpw2));
            strcpy(pwinfo,"未保存");
            memset(cPw0,0,sizeof(cPw0));
            memset(cPw1,0,sizeof(cPw1));
            memset(cPw2,0,sizeof(cPw2));

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
                    if (CTRL_CONTENT(mPassWord).focus > 0) {
                        CTRL_CONTENT(mPassWord).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mPassWord, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mPassWord).focus + 1 < CTRL_CONTENT(mPassWord).total) {
                        CTRL_CONTENT(mPassWord).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mPassWord, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					switch (CTRL_CONTENT(mPassWord).focus) {

					case 0:
					        memset(cPw0,0,sizeof(cPw0));
					        memset(oldpw,0,sizeof(oldpw));
							if( LoadEditDlg(&g_TextEditWindow, "请输入原密码", cPw0, cPw0, 8)) {
					            len = strlen(cPw0);                            	
					            for (;len;len--) {
					                oldpw[len-1]='*';
								}
							}
					        break;
					        
					    case 1:
					        memset(cPw1,0,sizeof(cPw1));
					        memset(newpw1,0,sizeof(newpw1));
							if( LoadEditDlg(&g_TextEditWindow, "请输入新密码", cPw1, cPw1, 8)) {
					            len = strlen(cPw1);
					            for (;len;len--) {
					                newpw1[len-1]='*';
										}
					            strcpy(pwinfo,"未保存");
							}
					        break;

					    case 2:
					        memset(cPw2,0,sizeof(cPw2));
					        memset(newpw2,0,sizeof(newpw2));
							if( LoadEditDlg(&g_TextEditWindow, "请重新输入新密码", cPw2, cPw2, 8)) {
					            len = strlen(cPw2);
					            for (;len;len--) {
					                newpw2[len-1]='*';
										}
					            strcpy(pwinfo,"未保存");
							}
					        break;
					        
					    case 3:                  	
							if(strcmp(cPw0, sSuperPwd) == 0) {
								Check_User_Pwd(cPw1, cPw2);
					            break;
					        }
					        				
					        if(strcmp(cPw0,(char *)gSysSet.password)) {
					            MsgBoxDlg(&g_MsgBoxDlg,"错误提示","原密码错误");
					        } else {
								Check_User_Pwd(cPw1, cPw2);
							}
					        break;

					    default:
					        break;
					}
					guiMsgInfo.pWindow = pWindow;
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
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


int CheckPassWord(void)
{
    char cbuf[10]="";
    int ret = 0;

	if( LoadEditDlg(&g_TextEditWindow, "请输入密码", cbuf, cbuf, 8)) 
	{
        if(0 == strcmp(cbuf,(char *)gSysSet.password) ) 
        {
        	ret = 1;
        } 

        if( 0 == strcmp(cbuf,sSuperPwd)) 
        {
            ret = 2;
        }
	}
		
  	return ret;
}

BOOL IsBbKeyOpen(void)           //标定开关
{
	BOOL ret = TRUE;
	
    if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10) == Bit_SET) 
    {      
    	ret = FALSE;

    	if(Get_SuperUser_Use()) 
    	{
    		ret = TRUE;
    		Set_SuperUser_Use(FALSE);
    	}
    } 

	return ret;
}



