#include "includes.h"
#include "Form_Warning.h"
#include "form_query.h"
#include "Form_Main.h"
#include "sys_param.h"
#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"
#include "device_info.h"
#include "io.h"

#include "adjob.h"

extern  CControl  gStatusBar; 		//状态栏
				  
static void Form_Warning_Timer(void *ptmr, void *parg);
static void Form_Warning_Draw(LPWindow pWindow);
static void Form_Warning_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

#define AttritemLen 9+2

static char attritemstr[AttritemLen][16];
static int gSysStatus;

//定义属性页
static const CAttrItem  _attritem[] = {
   {"前地感状态",attritemstr[0],1},
   {"后地感状态",attritemstr[1],2},
   {"分车光幕状态",attritemstr[2],3},
   {"传感器1状态",attritemstr[3],4},
   {"传感器2状态",attritemstr[4],5},
   {"传感器3状态",attritemstr[5],6},
   {"传感器4状态",attritemstr[6],7},
   {"AD芯片状态",attritemstr[7],8},
   {"轮胎识别器状态",attritemstr[8],9},
		{"串口1测试",attritemstr[9],10},
		{"SDK串口测试",attritemstr[10],11},
};

DEF_ATTR_CTRL(mWarningAttr, &gWD_Warning, 0, 20, 240, 108, "设备状态",(CAttrItem *)(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static const LPControl marrLPControl[] = 
{
	&gStatusBar,
	&mWarningAttr,
};

CWindow  gWD_Warning = {		 //窗体
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
    Form_Warning_Timer,
    Form_Warning_Draw,
    Form_Warning_Proc
};

//获取光幕、地感、轮胎识别器、传感器及AD转换芯片是否故障的状态
void Device_status(void)
{
	int ADChipErr = 0;
	char buf[16] = {0};
	
	if(Get_GC_ErrStatus(1))//判断地感状态，双秤台只是用前地感
	{
		sprintf(attritemstr[0],"正常");
	}
	else 
	{
		sprintf(attritemstr[0],"故障");
	}

	if(Get_GC_ErrStatus(2))//判断地感状态，双秤台只是用前地感
	{
		sprintf(attritemstr[1],"正常");
	}
	else 
	{
		sprintf(attritemstr[1],"故障");
	}

	if(Get_LC_ErrStatus(2))   //判断光幕状态,双秤台只是用后光幕
	{
		sprintf(attritemstr[2],"正常");
	}
	else 
	{
		sprintf(attritemstr[2],"故障");
	}

	if(Get_AD_ErrStatus(1))
	{
		sprintf(attritemstr[3],"异常");
	}
	else 
	{
		sprintf(attritemstr[3],"正常");
	}	

	if(Get_AD_ErrStatus(2))
	{
		sprintf(attritemstr[4],"异常");
	}
	else 
	{
		sprintf(attritemstr[4],"正常");
	}	

	if(Get_AD_ErrStatus(3))
	{
		sprintf(attritemstr[5],"异常");
	}
	else 
	{
		sprintf(attritemstr[5],"正常");
	}	

	if(Get_AD_ErrStatus(4))
	{
		sprintf(attritemstr[6],"异常");
	}
	else 
	{
		sprintf(attritemstr[6],"正常");
	}	

	ADChipErr = Get_ADChip_ErrStatus();
	if(ADChipErr > 0)
	{
 		if(ADChipErr & ERROR_AD1) snprintf(buf, 15, "1 ");
 		if(ADChipErr & ERROR_AD2) snprintf(buf+2,15-2,"2 ");
 		if(ADChipErr & ERROR_AD3) snprintf(buf+2,15-4,"3 ");
 		if(ADChipErr & ERROR_AD4) snprintf(buf+2,15-6,"4 ");
 		snprintf(attritemstr[7], 15, "%s路异常", buf);
	}
	else if(ADChipErr == 0)
	{
		sprintf(attritemstr[7],"正常");
	}	
	else
	{
		;
	}
	
	if(Get_Tire_ErrStatus())//判断轮胎识别器状态
	{
		sprintf(attritemstr[8],"正常");
	}
	else 
	{
		sprintf(attritemstr[8],"异常");
	}	
}

void Form_Warning_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
    static u32 Error_Status_Temp=0;
	if(lpWindow != NULL)
	{
		gSysStatus = Get_Device_Status();
		
		if(Error_Status_Temp != gSysStatus)	   //当错误发生变化
		{	
			Error_Status_Temp = gSysStatus;	   
		}
		
		Device_status();

		PostWindowMsg(lpWindow, WM_UPDATECTRL, (uint32)&mWarningAttr, 0);
	}
}

void Form_Warning_Draw(LPWindow pWindow)
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

//add_by_StreakingMCU
char m_USART6_test_flag = 0;//SDK串口测试标志
#include "serial_protocol.h"
#include "overtime_queue.h"
void Form_Warning_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	TaskMsg msg = {0};

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();							//清屏
			SysTimeDly(3);

			//串口1
			sprintf(attritemstr[9],"短接TR按确认");	
			//SDK串口
			sprintf(attritemstr[10],"短接TR按确认");

			CTRL_CONTENT(mWarningAttr).focus = 0;	//光标停留位置
			CreateWindowTimer(&gWD_Warning);
			StartWindowTimer(&gWD_Warning);

		case WM_SHOW:
			//设置故障内容
			Device_status();								
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
//			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
			gStatusBar.DrawFunc(&gStatusBar);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case KEY_UP:
                    if (CTRL_CONTENT(mWarningAttr).focus > 0) {
                        CTRL_CONTENT(mWarningAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mWarningAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mWarningAttr).focus + 1 < CTRL_CONTENT(mWarningAttr).total) {
                        CTRL_CONTENT(mWarningAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mWarningAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					//SDK串口测试
					if(0x0A == CTRL_CONTENT(mWarningAttr).focus)
					{
						m_USART6_test_flag = 0;
						//发送SDK测试消息
						msg.msg_id = Msg_USARTSDKTest;
						Task_QPost(&DataManagerMsgQ,  &msg);
						
						//延时200ms
						SysTimeDly(20);
						if(1 == m_USART6_test_flag)	//如果收到发送的数据
						{
							sprintf(attritemstr[10],"正常        ");
						}
						else
						{
							sprintf(attritemstr[10],"异常        ");
						}
							
					}
					//串口1测试
					if(0x09 == CTRL_CONTENT(mWarningAttr).focus)
					{
						
					}
					
					PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mWarningAttr, 0);
					break;

				case KEY_RIGHT: 
					break;

				case KEY_BACK:
					StopWindowTimer(&gWD_Warning);
// 					DestoryWindowTimer(&gWD_Warning);
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



