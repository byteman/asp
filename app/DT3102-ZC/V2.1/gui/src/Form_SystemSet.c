#include "includes.h"
#include "Form_SystemSet.h"
#include "Form_Main.h"
#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "Ctr_StatusBar.h"
#include "Common.h"
#include "Form_MsgBox.h"

#include "sys_config.h"
#include "weight_param.h"
#include "wet_algorim.h"
#include "sys_param.h"
#include "app_beep.h"
#include "Speed_check.h"

extern  CControl  gStatusBar; 		//状态栏

extern SSYSSET   gSysSet;

static void Form_WetSet_Draw(LPWindow pWindow);
static void Form_WetSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

//系统模式
static char cSysMod[9] = {0};					//计量模式
static char* cSysModItem[] = 
{
    "SCS-ZC-1",
    "SCS-ZC-2",
	"SCS-ZC-3",
    "SCS-ZC-4",
	"SCS-ZC-5",
	"SCS-ZC-6",
	"SCS-ZC-7"
};
DEF_LIST_DLG(gWD_SysModSet, 20, 20, 200, 88, "系统模式", cSysModItem, sizeof(cSysModItem)/sizeof(char *), 1, 0);


static char keyvoice_set[5]={0};
static char* cVoiceSetItem[] = 
{
    "关闭",
    "打开",
};
DEF_LIST_DLG(gWD_KeyVoiceSet, 20, 20, 200, 88, "按键音", cVoiceSetItem, sizeof(cVoiceSetItem)/sizeof(char *), 1, 0);

static char cLongCarMode[9] = {0};					//长车模式
static char* cLongCarModeItem[] = 
{
    "自动称重",
    "分段称重",
};
DEF_LIST_DLG(gWD_LongCarModeSet, 20, 20, 200, 88, "长车模式选择", cLongCarModeItem, sizeof(cLongCarModeItem)/sizeof(char *), 1, 0);

// BGFlag
static char cBGFlag[4] = {0};					//道闸标志  有或无
static char* cBGFlagItem[] = 
{
	    "无",
		"有"
};
DEF_LIST_DLG(gWD_BGFlagSet, 20, 20, 200, 88, "道闸配置", cBGFlagItem, sizeof(cBGFlagItem)/sizeof(char *), 0, 1);

//TrafficSignalFlag
static char cTrafficSignalFlag[4] = {0};					//红绿灯标志  有或无
static char* cTrafficSignalFlagItem[] = 
{
	"无",
    "有",
};
DEF_LIST_DLG(gWD_TrafficSignalFlagSet, 20, 20, 200, 88, "红绿灯配置", cTrafficSignalFlagItem, sizeof(cTrafficSignalFlagItem)/sizeof(char *), 0, 1);

static char cDebugLevel[9] = {0};					//调试打印等级
static char* cDebugLevelItem[] = 
{
    "不输出",
    "错误信息",
	"业务信息",
    "警告信息",
	"通知信息",
	"周期信息"
};
DEF_LIST_DLG(gWD_DebugLevelSet, 20, 20, 200, 88, "高于当前等级的信息不输出", cDebugLevelItem, sizeof(cDebugLevelItem)/sizeof(char *), 1, 0);


static char cStaticFiltering[4] = {0};
static char cDynaFiltering[4] = {0};
static char cMaxSpeed[4]={0};
static char cLC2GCLength[8] = {0};
static char cAxleLength[8] = {0};

static char cParamUpWet[8] = {0};					//整秤上秤阈值
static char cParamDownWet[8] = {0};					//整秤下秤阈值
static char cParamAxle1UpWet[8] = {0};				//上秤端轴上阈值
static char cParamAxle1DownWet[8] = {0};			//上秤端轴下阈值
static char cParamAxle2Upwet[8] = {0};				//下秤端轴上阈值
static char cParamAxle2DownWet[8] = {0};			//下秤端轴下阈值
static char cParamAxle3Upwet[8] = {0};				//弯板轴上阈值
static char cParamAxle3DownWet[8] = {0};			//弯板轴下阈值

static CAttrItem  _attritem[] = {
	{"计量模式设置",cSysMod,1},
	{"静态滤波等级",cStaticFiltering,2},
	{"动态滤波等级",cDynaFiltering,3},
	{"超速报警速度",cMaxSpeed,4},
	{"光幕地感距离",cLC2GCLength, 5},
	{"前轴识别器宽度",cAxleLength, 6},
	{"声音开关",keyvoice_set,7},
	{"整秤上秤阈值",cParamUpWet,8},
	{"整秤下秤阈值",cParamDownWet,9},
	{"上称端轴上阈值",cParamAxle1UpWet,10},
	{"上称端轴下阈值",cParamAxle1DownWet,11},
	{"下称端轴上阈值",cParamAxle2Upwet,12},   
	{"下称端轴下阈值",cParamAxle2DownWet,13},
	{"弯板上阈值",cParamAxle3Upwet,14},   
	{"弯板下阈值",cParamAxle3DownWet,15},
	{"长车模式设置",cLongCarMode,16},
	{"道闸配置",cBGFlag,17},
	{"红绿灯配置",cTrafficSignalFlag,18},
	{"调试打印等级",cDebugLevel,19},
};

DEF_ATTR_CTRL(mSystemAttr, &gWD_SystemSet, 0, 20, 240, 108, "系统参数设置",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mSystemAttr,
};

CWindow  gWD_SystemSet = {
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
    NULL,
    Form_WetSet_Draw,
    Form_WetSet_Proc
};

void Form_WetSet_Draw(LPWindow pWindow)
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

static void Set_AxleThreshold(uint8 flag, uint16 ulTemp, uint16 twet, uint16 *swet, uint32 addr)
{
	if((ulTemp < twet) && flag) {
		MsgBoxDlg(&g_MsgBoxDlg,"错误提示","上阈值必须大于下阈值");
	} else if((ulTemp > twet) && !flag) {
		MsgBoxDlg(&g_MsgBoxDlg,"错误提示","下阈值必须小于上阈值");
	} else if(ulTemp > 2000 || ulTemp < 20) {
		MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入20--2000的值");
	} else {
		if(!Param_Write(GET_U16_OFFSET(addr), &ulTemp, sizeof(u16))) {
			MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
			return;
		}
		*swet = ulTemp;
		Init_Wet_Par();
	}
}

static void WetSet_Business(void)
{
    char   cTemp[12]="";
	u16    ulTemp = 0;
	int16  flag=0;
	
	switch (CTRL_CONTENT(mSystemAttr).focus) {
		case 0:
			if (CTRL_CONTENT(mSystemAttr).focus == 0) {
				CTRL_CONTENT(gWD_SysModSet).focus  = Get_System_Mode();
				CTRL_CONTENT(gWD_SysModSet).select = Get_System_Mode();
				flag =	LoadListDlg(&gWD_SysModSet);
				if((flag >= SCS_ZC_1) && (flag != SCS_ZC_4) && (flag < SCS_CNT)) {	
					Set_System_Mode((SysMod)flag);	//设置系统模式
					MsgBoxDlg(&g_MsgBoxDlg,"重启提示", "模式修改成功, 请重启!");
				} else if(flag == 3) {
					MsgBoxDlg(&g_MsgBoxDlg,"错误提示","此模式已取消,请重新选择");
				}
			}else {
				;
			}
			flag = 0;
			break;
				
		case 1:
			//静态滤波等级
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入静态滤波等级(1-10)", cTemp, cTemp, 2)) 
			{
				if(strlen(cStaticFiltering)) 
				{
					ulTemp = atoi(cTemp);
						
					if(ulTemp > 10 | ulTemp < 1)
					{
						MsgBoxDlg(&g_MsgBoxDlg,"错误提示","没有此等级,请重新输入");
					}
					else
					{
						if(!Param_Write(GET_U8_OFFSET(ParamStaticFilter), &ulTemp, sizeof(u8)))
						{
							MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
							break;
						}
						gWetPar.StaticFilterLevel = ulTemp;
					}
					
				 }
			} 
			break;
							
		case 2:
			//动态滤波等级cDynaFiltering
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入动态滤波等级(1-10)", cTemp, cTemp, 2)) 
			{
				if(strlen(cDynaFiltering)) 
				{
					ulTemp = atoi(cTemp);

					if(ulTemp > 10 | ulTemp < 1)
					{
						MsgBoxDlg(&g_MsgBoxDlg,"错误提示","没有此等级,请重新输入");
					}
					else
					{
						if(!Param_Write(GET_U8_OFFSET(ParamDynamicFilter), &ulTemp, sizeof(u8)))
						{
							MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败，请重新设置");
							break;
						}
						gWetPar.DynamicFilterLevel = ulTemp;
					}
									
				}
			} 
			break;
				
		case 3://超速报警速度
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入超速报警速度(km/h)", cTemp, cTemp, 2)) 
			{
				if(strlen(cMaxSpeed)) 
				{
					ulTemp = atoi(cTemp);

					if(ulTemp > 40 | ulTemp < 5)
					{
						MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入5--40的值");
					}
					else
					{
						if(!Param_Write(GET_U8_OFFSET(ParamSuperSpeedLevel), &ulTemp, sizeof(u8)))
						{
							MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
							break;
						}	
						gWetPar.SuperSpeedLevel = ulTemp;
					}
					
				}
			}
			break;
	
		case 4://光幕地感距离
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入光幕地感距离(cm)", cTemp, cTemp, 3)) 
			{
				if(strlen(cLC2GCLength)) 
				{
					ulTemp = atoi(cTemp);
					if(ulTemp > 600 | ulTemp < 1)
					{
						MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入1~600的值");
					}
					else
					{
						//ulTemp = ulTemp * 10;
						if(!Param_Write(GET_U16_OFFSET(ParamSpeedLength), &ulTemp, sizeof(u16)))
						{
							MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
							break;
						}	
						gWetPar.Length= ulTemp;
						Set_Speed_Length(gWetPar.Length * 10);
					}
				}
			}
			break;

		case 5: //前轴宽度
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入前轴宽度(cm)", cTemp, cTemp, 3)) 
			{
				if(strlen(cAxleLength)) 
				{
					ulTemp = atoi(cTemp);
					
					if(ulTemp > 600 | ulTemp < 1)
					{
							MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入1~600的值");
					}
					else
					{
						//ulTemp = ulTemp * 10;
						if(!Param_Write(GET_U16_OFFSET(ParamAxleLength), &ulTemp, sizeof(u16)))
						{
							MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
							break;
						}	
						gWetPar.AxleLength= ulTemp;
						Init_Wet_Par();
					}
				}
			}
			break;
							
		case 6:  //蜂鸣器声音开关			 
			if (CTRL_CONTENT(mSystemAttr).focus == 6) {
				CTRL_CONTENT(gWD_KeyVoiceSet).focus  = gSysSet.Beep;
				CTRL_CONTENT(gWD_KeyVoiceSet).select = gSysSet.Beep;
				flag =	LoadListDlg(&gWD_KeyVoiceSet);

				if(flag >= 0)
				{
					gSysSet.Beep = flag;
					if(flag)
						Set_Beep_Flag(1);
					else
						Set_Beep_Flag(0);	   //关闭	   
				}
				EnableVoiceIcon(gSysSet.Beep);
			}else {
				;
			}
			break;
													
		case 7://整秤上称阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入整秤上称阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				
				if(ulTemp > 2000 || ulTemp < 20)
				{
					MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入20--2000的值");
				}
				else
				{
					if(!Param_Write(GET_U16_OFFSET(ParamUpWet), &ulTemp, sizeof(u16)))
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
						break;
					}
					gWetPar.TotalUpWet = ulTemp;
				}
			}				
			break;
	
		case 8://整秤下称阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入整秤下称阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				
				if(ulTemp > 2000 || ulTemp < 20)
				{
						MsgBoxDlg(&g_MsgBoxDlg,"错误提示","请输入20--2000的值");
				}
				else
				{
					if(!Param_Write(GET_U16_OFFSET(ParamDownWet), &ulTemp, sizeof(u16)))
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数保存失败,请重新设置");
						break;
					}
					gWetPar.TotalDownWet = ulTemp;
				}
			}	
			break;
	
		case 9://上秤端轴上阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入上秤端轴上阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(1, ulTemp, gWetPar.PreAxleDownWet, &gWetPar.PreAxleUpWet, ParamPreAxleUpWet);
			}

			break;
		
		case 10://上秤端轴下阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入上秤端轴下阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(0, ulTemp, gWetPar.PreAxleUpWet, &gWetPar.PreAxleDownWet, ParamPreAxleDownWet);
			}
			break;
	
		case 11://下秤端轴上阈值		
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入下秤端轴上阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(1, ulTemp, gWetPar.BackAxleDownWet, &gWetPar.BackAxleUpWet, ParamBackAxleUpWet);
			}	
			break;
			
		case 12://下秤端轴下阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入下秤端轴下阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(0, ulTemp, gWetPar.BackAxleUpWet, &gWetPar.BackAxleDownWet, ParamBackAxleDownWet);
			}
			break;

		case 13://弯板上阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入弯板上阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(1, ulTemp, gWetPar.WbAxleDownWet, &gWetPar.WbAxleUpWet, ParamWbAxleUpWet);
			}
			break;
		
		case 14://弯板下阈值
			memset(cTemp,0,sizeof(cTemp));
			if( LoadEditDlg(&g_TextEditWindow, "请输入弯板下阈值20-2000", cTemp, cTemp, 4)) 
			{
				ulTemp = atoi(cTemp);
				Set_AxleThreshold(0, ulTemp, gWetPar.WbAxleUpWet, &gWetPar.WbAxleDownWet, ParamWbAxleDownWet);
			}
			break;
								
		case 15://长车模式
			if (CTRL_CONTENT(mSystemAttr).focus == 15) {
				CTRL_CONTENT(gWD_LongCarModeSet).focus	= Get_LongCar_Mode();
				CTRL_CONTENT(gWD_LongCarModeSet).select = Get_LongCar_Mode();
				flag =	LoadListDlg(&gWD_LongCarModeSet);
				if(flag >= 0)
				{							
					Set_LongCar_Mode((LongCarMod)flag);  //设置长车模式
				}
				}else {
					;
				}
				flag = 0;

				break;
								
		case 16://道闸标志设置
			if (CTRL_CONTENT(mSystemAttr).focus == 16) {
				CTRL_CONTENT(gWD_BGFlagSet).focus  = Get_BGFlag();
				CTRL_CONTENT(gWD_BGFlagSet).select = Get_BGFlag();
				flag =	LoadListDlg(&gWD_BGFlagSet);
				if(flag >= 0)
				{
					Set_BGFlag(flag);  //设置道闸标志
				}
			}else {
			}
			flag = 0;

			break;
									
		case 17://红绿灯设置
			if (CTRL_CONTENT(mSystemAttr).focus == 17) {
				CTRL_CONTENT(gWD_TrafficSignalFlagSet).focus  = Get_TrafficSignal_Flag();
				CTRL_CONTENT(gWD_TrafficSignalFlagSet).select = Get_TrafficSignal_Flag();
				flag =	LoadListDlg(&gWD_TrafficSignalFlagSet);
				if(flag >= 0)
				{	
					Set_TrafficSignal_Flag(flag);  //设置红绿灯模式
				}
			}else {
				;
			}
			flag = 0;

			break;
									
		case 18://调试打印等级
			if (CTRL_CONTENT(mSystemAttr).focus == 18) 
			{
				CTRL_CONTENT(gWD_DebugLevelSet).focus  = Get_Debug_Level();
				CTRL_CONTENT(gWD_DebugLevelSet).select = Get_Debug_Level();
				flag =	LoadListDlg(&gWD_DebugLevelSet);
				Set_Debug_Level(flag);	//设置调试打印等级
			}
			else 
			{
			}
			flag = 0;

			break;
	
		default:
			break;
	}
}

void Form_WetSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(2);
			CTRL_CONTENT(mSystemAttr).focus = 0;

		case WM_SHOW:
			gSysSet.Beep = Get_Beep_Flag();
			strncpy(cSysMod, cSysModItem[Get_System_Mode()], 8);	//计量模式选择
			snprintf(cStaticFiltering, 3,"%d", gWetPar.StaticFilterLevel);   //静态滤波等级
			snprintf(cDynaFiltering, 3,"%d", gWetPar.DynamicFilterLevel);		//动态滤波等级
			snprintf(cMaxSpeed, 3,"%dkm/h", gWetPar.SuperSpeedLevel);				//速度阈值
			snprintf(cLC2GCLength, 7, "%dcm", gWetPar.Length);						//光幕地感距离
			snprintf(cAxleLength, 7, "%dcm", gWetPar.AxleLength);						//轴距离
			strncpy(keyvoice_set, cVoiceSetItem[gSysSet.Beep], 4);			//按键音
			snprintf(cParamUpWet, 7,"%d", gWetPar.TotalUpWet);   						//整秤上秤阈值
			snprintf(cParamDownWet, 7,"%d", gWetPar.TotalDownWet);   				//整秤下秤阈值
			snprintf(cParamAxle1UpWet,   7,"%d", gWetPar.PreAxleUpWet);   				//上秤端轴上阈值
			snprintf(cParamAxle1DownWet, 7,"%d", gWetPar.PreAxleDownWet);   		//上秤端轴下阈值
			snprintf(cParamAxle2Upwet,   7,"%d", gWetPar.BackAxleUpWet);   				//下秤端轴上阈值
			snprintf(cParamAxle2DownWet, 7,"%d", gWetPar.BackAxleDownWet);   		//下秤端轴下阈值
			snprintf(cParamAxle3Upwet,   7,"%d", gWetPar.WbAxleUpWet);   				//下秤端轴上阈值
			snprintf(cParamAxle3DownWet, 7,"%d", gWetPar.WbAxleDownWet);   		//下秤端轴下阈值
			strncpy(cLongCarMode, cLongCarModeItem[Get_LongCar_Mode()], 8);			//长车模式
			strncpy(cBGFlag,cBGFlagItem[Get_BGFlag()], 2);											//道闸配置
			strncpy(cTrafficSignalFlag,cTrafficSignalFlagItem[Get_TrafficSignal_Flag()], 2);			//红绿灯配置
			strncpy(cDebugLevel, cDebugLevelItem[Get_Debug_Level()], 8);					//调试打印等级

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
                    if (CTRL_CONTENT(mSystemAttr).focus > 0) {
                        CTRL_CONTENT(mSystemAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mSystemAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mSystemAttr).focus + 1 < CTRL_CONTENT(mSystemAttr).total) {
                        CTRL_CONTENT(mSystemAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mSystemAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					WetSet_Business();
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

