#include "Form_MainMenu.h"
#include "Form_MeasureSet.h"
#include "Ctr_StatusBar.h"
#include "Keyboard.h"
#include "../Inc/Menu.h"
#include "Form_Main.h"
#include "Form_Query.h"
#include "Form_TimeDate.h"
#include "Form_SystemSet.h"

#include "Form_CalibrateZero.h"
#include "Form_CalibrateSet.h"
#include "Form_CalibrateWeight.h"
#include "Form_CalibrateWeightMode1.h"
#include "Form_CalibrateAxleWeight.h"
#include "Form_ADChannel.h"
#include "Form_Version.h"

#include "Form_PassWord.h"

#include "Form_MsgBox.h"
#include "Form_NetSet.h"

#include "Form_SpeedMod.h"
#include "Form_OverLoadSet.h"
#include "Form_USB_update.h"
#include "form_warning.h"
#include "form_device_signal.h"

#include "Form_MeasureSet.h"
#include "Form_InPortTest.h"
#include "Form_OutPortTest.h"
#include "sys_config.h"
#include "device_info.h"

extern  CControl  gStatusBar;

extern  BOOL CalibrateZeroFlag;		//标定完成标志

// #define	ReadDemarcateIO()		(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_10))

/*
 *  标定菜单
 */
DEF_MENU_ITEM(mnCalibraA,       "零点标定",     0, NULL);
DEF_MENU_ITEM(mnCalibraB,       "重量标定",     1, NULL);
DEF_MENU_ITEM(mnCalibraAxle,     "轴标定",      2, NULL);

DEF_MENU_ITEM(mnCalibraSet,     "标定系数",      3, NULL);
DEF_MENU_ITEM(mnADChannel,     	"AD 通道 ",      4, NULL);

static LPMenuItem	m_CalibMenuItems[] = 
{
	&mnCalibraA,
	&mnCalibraB,	
	&mnCalibraAxle,
	&mnCalibraSet,
	&mnADChannel,
};
void FormCalibMenuProc(struct _CWindow* pParentWindow, uint32 nCmd);

DEF_MENU(mCalibMenu, "标定", m_CalibMenuItems, NULL, FormCalibMenuProc);	 
DEF_MENU_WND(gWD_CalibMenu, &mCalibMenu, NULL);


void FormCalibMenuProc(struct _CWindow* pParentWindow, uint32 nCmd)
{	
	switch(nCmd)
	{
		case 0:
            if(Get_AD_ErrStatus(1)) 
            {
                MsgBoxDlg(&g_MsgBoxDlg, "信息提示", "主秤传感器异常,不能标定");
            } 
            else 
            {
                g_pCurWindow = &gWD_CalibrateZero;
    			g_pCurWindow->pParentWindow = &gWD_CalibMenu;
    			PostWindowMsg(g_pCurWindow, WM_LOAD, 1, NULL);	
            }
			break;
        
		case 1:
             if(Get_AD_ErrStatus(1)) 
             {
                 MsgBoxDlg(&g_MsgBoxDlg, "信息提示", "主秤传感器异常,不能标定");
             } 
             else 
             {
				if(SCS_ZC_1 == Get_System_Mode())
				{
					g_pCurWindow = &gWD_CalibrateWeightMode1;
					g_pCurWindow->pParentWindow = &gWD_CalibMenu;
					PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	
				}
				else
				{		
					if(CalibrateZeroFlag)
					{
						g_pCurWindow = &gWD_CalibrateWeight;
						g_pCurWindow->pParentWindow = &gWD_CalibMenu;
						PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	
					}
					else 
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "未进行零点标定，请标定零点");
					}
				}
            }
			break;

		case 2:
             if(0)//Get_AD_ErrStatus(3) || Get_AD_ErrStatus(4)) 
             {
                 MsgBoxDlg(&g_MsgBoxDlg, "信息提示", "轴传感器异常,不能标定");
             } 
             else 
             {	
				if(CalibrateZeroFlag)
				{
					g_pCurWindow = &gWD_CalibrateAxleWeight;
					g_pCurWindow->pParentWindow = &gWD_CalibMenu;
					PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	
				}
				else 
				{
					MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "未进行零点标定，请标定零点");
				}
            }
			break;
        
		case 3:
            g_pCurWindow = &gWD_CalibrateSet;
			g_pCurWindow->pParentWindow = &gWD_CalibMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	
			break;
		
		case 4:
            g_pCurWindow = &gWD_ADChannel;
			g_pCurWindow->pParentWindow = &gWD_CalibMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, 2, NULL);	
			break;
            
		default:
			break;
	}
}

/*
 *  主菜单
 */
DEF_MENU_ITEM(mnQuery,	    	"查询",     	0, 	NULL);
DEF_MENU_ITEM(mnErrorQuery,		"故障查询", 	1, NULL);
DEF_MENU_ITEM(mnMeasureSet,		"称重设置", 	2, 	NULL);  
DEF_MENU_ITEM(mnTimeSet,			"时间设置", 	3, 	NULL);

DEF_MENU_ITEM(mnNetSet, 			"网络设置", 	4, 	NULL);
DEF_MENU_ITEM(mnSoftVer,    	"版本查询", 	5, 	NULL);

DEF_MENU_ITEM(mnSpeedCorr,  	"速度修正", 	6, 	NULL);
DEF_MENU_ITEM(mnWeightSet,		"系统设置", 	7, 	NULL);
DEF_MENU_ITEM(mnOverLoad,   	"超载设置", 	8, 	NULL);


DEF_MENU_ITEM(mnCalibrate,		"标定",     	9, 	NULL);

DEF_MENU_ITEM(mnPassWord,  		"密码设置", 	10, 	NULL);
DEF_MENU_ITEM(mnInPortTest,   "输入调试", 	11, NULL);
DEF_MENU_ITEM(mnOutPortTest,  "输出调试", 	12, NULL);
DEF_MENU_ITEM(mnUsbTest,   "U盘升级", 13, NULL);
DEF_MENU_ITEM(mnDeviceSignal, "外设电平", 14, NULL);


static LPMenuItem	m_MainMenuItems[] = 
{
    //&gStatusBar,
	&mnQuery,
	&mnErrorQuery,
	&mnMeasureSet,	  	
	&mnTimeSet,
	&mnNetSet,
	&mnSoftVer,	  	
	&mnSpeedCorr,	  	
	&mnWeightSet,	  	
	&mnOverLoad,	  	
	&mnCalibrate,	  
	&mnPassWord,
	&mnInPortTest,
	&mnOutPortTest,	
	&mnUsbTest,
	&mnDeviceSignal
};

void FormMainMenuProc(struct _CWindow* pParentWindow, uint32 nCmd);

DEF_MENU(mMainMenu, "主菜单", m_MainMenuItems, NULL, FormMainMenuProc);	 
DEF_MENU_WND(gWD_MainMenu, &mMainMenu, &gWD_Main);

static volatile BOOL SuperUserFlag = FALSE;

BOOL Get_SuperUser_Use(void)
{
	return SuperUserFlag;
}


void Set_SuperUser_Use(BOOL flag)
{
	SuperUserFlag = flag;
}


void FormMainMenuProc(struct _CWindow* pParentWindow, uint32 nCmd)
{
	int pwd = 0;
	
	switch(nCmd)
	{
		case 0:
			g_pCurWindow = &gWD_DataMenu;//gWD_Query;						//查询
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
			
		case 1:												//故障
			g_pCurWindow = &gWD_Warning;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
		
        
		case 2:												//计量设置---称重参数设置
			pwd = CheckPassWord();
			if(pwd == 0) {
				MsgBoxDlg(&g_MsgBoxDlg, "错误", "密码错误");
				break; 
			} else if(pwd == 2) {
				SuperUserFlag = TRUE;
			}
			
			g_pCurWindow = &gWD_MeasureSet;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;

		case 3:
			g_pCurWindow = &gWD_TimeDate;					//时间
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;


		case 4:												//网络
            g_pCurWindow = &gWD_NetSet;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
			    
		case 5:												//版本
			g_pCurWindow = &gWD_Version;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
			    
		    
		case 6:												//速度
		   if(FALSE == CheckPassWord()) {
				MsgBoxDlg(&g_MsgBoxDlg, "错误", "密码错误");
				break; 
			}
			g_pCurWindow = &gWD_SpeedMod;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
            
		case 7:												//系统设置
			if(FALSE == CheckPassWord()) {
				MsgBoxDlg(&g_MsgBoxDlg, "错误", "密码错误");
				break; 
			}
			g_pCurWindow = &gWD_SystemSet;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
            
		case 8:												//超载
			if(FALSE == CheckPassWord()) {
						MsgBoxDlg(&g_MsgBoxDlg, "错误", "密码错误");
						break; 
			}
			g_pCurWindow = &gWD_OverLoadSet;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
            

						
		case 9:										    //标定
			if(IsBbKeyOpen() == FALSE) 
			{
				MsgBoxDlg(&g_MsgBoxDlg, "错误", "请打开标定开关");
				break; 
			}           
			g_pCurWindow = &gWD_CalibMenu;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
		    break;
			
		case 10:										    //密码
            g_pCurWindow = &gWD_PassWord;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
            
		case 11:										    //输入调试

			g_pCurWindow = &gWD_InPortTest;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
		
		case 12:										    //输出调试
			g_pCurWindow = &gWD_OutPortTest;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;
           
		case 13:
			g_pCurWindow = &gWD_USB_Updata;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);	
			break;

		case 14:
			if(FALSE == CheckPassWord()) {
				MsgBoxDlg(&g_MsgBoxDlg, "错误", "密码错误");
				break; 
			}
			g_pCurWindow = &gWD_DeviceSignal;
			g_pCurWindow->pParentWindow = &gWD_MainMenu;
			PostWindowMsg(g_pCurWindow, WM_LOAD, NULL, NULL);
			break;
           
		default:
			break;
	}
}



