#include "Form_SpeedMod.h"
#include "Ctr_Attr.h"
#include "Form_Resource.h"
#include "Ctr_StatusBar.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "Form_Main.h"
#include "Form_MsgBox.h"



#include "sys_config.h"
#include "weight_param.h"
#include "sys_param.h"

extern  CControl  gStatusBar;

static void Form_SpeedMod_Timer(void *ptmr, void *parg);
static void Form_SpeedMod_Draw(LPWindow pWindow);
static void Form_SpeedMod_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


static char cSpeed_0k[10] ="";
static char cSpeed_5k[10] ="";
static char cSpeed_10k[10]="";
static char cSpeed_15k[10]="";
static char cSpeed_20k[10]="";
static char cSpeed_25k[10]="";
static char cSpeed_30k[10]="";
static char cFollow_0k[10] ="";
static char cFollow_2P5k[10] ="";
static char cFollow_5k[10]="";
static char cFollow_7P5k[10] ="";
static char cFollow_10k[10]="";
static char cFollow_12P5k[10]="";
static char cFollow_15k[10]="";
static char cFollow_20k[10]="";
static char cFollow_25k[10]="";
static char cFollow_30k[10]="";
static const CAttrItem  _attritem[] = {
   {"0km/h",cSpeed_0k,1},
   {"5km/h",cSpeed_5k,2},
   {"10km/h",cSpeed_10k,3},
   {"15km/h",cSpeed_15k,4},
   {"20km/h",cSpeed_20k,5},
   {"25km/h",cSpeed_25k,6},
   {"30km/h",cSpeed_30k,7},
	 
   {"跟车0km/h",cFollow_0k,8},
   {"跟车2.5km/h",cFollow_2P5k,9},
   {"跟车5km/h",cFollow_5k,10},
   {"跟车7.5km/h",cFollow_7P5k,11},
   {"跟车10km/h",cFollow_10k,12},
   {"跟车12.5km/h",cFollow_12P5k,13},
	 {"跟车15km/h",cFollow_15k,14},
   {"跟车20km/h",cFollow_20k,15},
   {"跟车25km/h",cFollow_25k,16},
   {"跟车30km/h",cFollow_30k,17},	 
};

DEF_ATTR_CTRL(mSpeedMod, &gWD_SpeedMod, 0, 20, 240, 108, "速度修正",(CAttrItem *)(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mSpeedMod,
};

CWindow  gWD_SpeedMod = {
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
    Form_SpeedMod_Timer,
    Form_SpeedMod_Draw,
    Form_SpeedMod_Proc
};

void Form_SpeedMod_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

void Form_SpeedMod_Draw(LPWindow pWindow)
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
void Form_SpeedMod_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
//	int16 flag=0;
  char  cTemp[10]="";
	int32  ulTemp;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(3);
			CTRL_CONTENT(mSpeedMod).focus = 0;

		case WM_SHOW:
            sprintf(cSpeed_0k,"%d‰",gWetPar.Speed_0k);
            sprintf(cSpeed_5k,"%d‰",gWetPar.Speed_5k);
            sprintf(cSpeed_10k,"%d‰",gWetPar.Speed_10k);
            sprintf(cSpeed_15k,"%d‰",gWetPar.Speed_15k);
            sprintf(cSpeed_20k,"%d‰",gWetPar.Speed_20k);
            sprintf(cSpeed_25k,"%d‰",gWetPar.Speed_25k);
            sprintf(cSpeed_30k,"%d‰",gWetPar.Speed_30k);
		
            sprintf(cFollow_0k,"%d‰",gWetPar.Follow_0k);
            sprintf(cFollow_2P5k,"%d‰",gWetPar.Follow_2P5k);
            sprintf(cFollow_5k,"%d‰",gWetPar.Follow_5k);		
            sprintf(cFollow_7P5k,"%d‰",gWetPar.Follow_7P5k);
            sprintf(cFollow_10k,"%d‰",gWetPar.Follow_10k);
            sprintf(cFollow_12P5k,"%d‰",gWetPar.Follow_12P5k);
						sprintf(cFollow_15k,"%d‰",gWetPar.Follow_15k);
            sprintf(cFollow_20k,"%d‰",gWetPar.Follow_20k);
            sprintf(cFollow_25k,"%d‰",gWetPar.Follow_25k);
            sprintf(cFollow_30k,"%d‰",gWetPar.Follow_30k);		
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
                    if (CTRL_CONTENT(mSpeedMod).focus > 0) {
                        CTRL_CONTENT(mSpeedMod).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mSpeedMod, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mSpeedMod).focus + 1 < CTRL_CONTENT(mSpeedMod).total) {
                        CTRL_CONTENT(mSpeedMod).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mSpeedMod, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_RIGHT: 
					break;

				case KEY_OK:
					switch (CTRL_CONTENT(mSpeedMod).focus) {
					case 0:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 || ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed0k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_0k = ulTemp;
							}
						}
						break;
                        
					case 1:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 || ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed5k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_5k = ulTemp;
							}
						}
						break;
													
					case 2:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 || ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed10k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_10k = ulTemp;
							}
						}
						break;
											 
					 case 3:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 || ulTemp < 0)
							{
								MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed15k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_15k = ulTemp;
							}
						}
						break;
												
					 case 4:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 ||  ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed20k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_20k = ulTemp;
							}
						}
						break;
					
					 case 5:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
											
							if(ulTemp > 10000 || ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
								if(!Param_Write(GET_U16_OFFSET(ParamSpeed25k), &ulTemp, sizeof(u16)))
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
									break;
								}
								gWetPar.Speed_25k = ulTemp;
							}
						}
						break;
						
					 case 6:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamSpeed30k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Speed_30k = ulTemp;
							}
						}
						break;
						
					 case 7:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow0k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_0k = ulTemp;
							}
						}
						break;			

					 case 8:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow2P5k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_2P5k = ulTemp;
							}
						}
						break;	

					 case 9:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow5k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_5k = ulTemp;
							}
						}
						break;		

					 case 10:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow7P5k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_7P5k = ulTemp;
							}
						}
						break;	

					 case 11:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow10k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_10k = ulTemp;
							}
						}
						break;		

					 case 12:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow12P5k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_12P5k = ulTemp;
							}
						}
						break;							

					 case 13:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow15k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_15k = ulTemp;
							}
						}
						break;	

					 case 14:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow20k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_20k = ulTemp;
							}
						}
						break;

					 case 15:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow25k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_25k = ulTemp;
							}
						}
						break;		

					 case 16:
						memset(cTemp,0,sizeof(cTemp));
						if( LoadEditDlg(&g_TextEditWindow, "请输入修正值(‰)", cTemp, cTemp, 5)) 
						{
							ulTemp = atoi(cTemp);
							
							if(ulTemp > 10000 | ulTemp < 0)
							{
									MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入修正值0-10000");
							}
							else
							{
									
									if(!Param_Write(GET_U16_OFFSET(ParamFollow30k), &ulTemp, sizeof(u16)))
									{
										MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
										break;
									}
									gWetPar.Follow_30k = ulTemp;
							}
						}
						break;							
											
					 default:
							 break;
				}
			guiMsgInfo.pWindow = pWindow;
			guiMsgInfo.ID = WM_SHOW;
			GuiMsgQueuePost(&guiMsgInfo);					
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



