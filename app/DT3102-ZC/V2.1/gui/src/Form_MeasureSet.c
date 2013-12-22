
#include "Form_MeasureSet.h"
#include "Form_Main.h"
#include "Ctr_Attr.h"
#include "Ctr_DlgList.h"
#include "../Inc/TextEdit.h"

#include "Common.h"
#include "weight_param.h"
#include "sys_param.h"
#include "wet_algorim.h"
#include "Form_MsgBox.h"

extern  CControl  gStatusBar; 		//状态栏
//extern SMatricPara gMatricPara;
extern gWetParam gWetPar;			//全局称重参数
extern SSYSSET   gSysSet;

/*
 * 计量单位
 */
char* gcWet_Unit[] = 
{
    "kg",
    "t",
};

//时间单位
char *time_Uint[] = 
{
	"s",
	"m",
	"h",
};

typedef struct 
{
    uint8  ucDivValueStaLabel;		//静态分度值选项
    uint8  ucDivValueDynaLabel;		//动态分度值
    uint8  ucBootZeroLabel;			//开机置零范围
    uint8  ucManualZeroLabel;			//手动置零范围
    uint8  ucZeroTarkRangeLabel;    //零点跟踪范围
    uint8  ucZeroTarkTimeLabel;		//零点跟踪时间
    uint8  ucZeroTarkSpdLabel;		//零点跟踪速度
} SWetParmSet;

static SWetParmSet gWetParmFlag;						//称重参数文件内局部静态变量



static void Form_WetSet_Timer(void *ptmr, void *parg);
static void Form_WetSet_Draw(LPWindow pWindow);
static void Form_WetSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


// DEF_LIST_DLG(gWD_WetUnit, 20, 20, 200, 88, "计量单位", gcWet_Unit, sizeof(gcWet_Unit)/sizeof(char *), 1, 0);

/*
 * 静态分度值
 */
static const uint8 ucWetDivValue_Sta[] = {
    1,
		2,
    5,
    10,
    20,
		50,
		100,
		200,
};

static char* cWet_DivValue_Sta[] = {
    "1",
		"2",
    "5",
    "10",
    "20",
		"50",
		"100",
		"200",
};
DEF_LIST_DLG(gWD_WetDivValue_Sta, 20, 20, 200, 88, "静态分度值", cWet_DivValue_Sta, sizeof(cWet_DivValue_Sta)/sizeof(char *), 1, 0);

/*
 * 动态分度值
 */
static const uint8 ucWetDivValue_Dyna[] = {
    10,
    20,
    50,
    100,
};

static char* cWet_DivValue_Dyna[] = {
    "10",
    "20",
    "50",
    "100",
};
DEF_LIST_DLG(gWD_WetDivValue_Dyna, 20, 20, 200, 88, "动态分度值", cWet_DivValue_Dyna, sizeof(cWet_DivValue_Dyna)/sizeof(char *), 1, 0);


/*
 * 开机置零范围
 */
static const u16 usWetBootZeroRange[] = {
    0,
    2,
    5,
    10,
    20,
    //50,
    //100,
};
static char* cWet_BootZeroRange[] = {
    "关闭",
    "2% F.S",
    "5% F.S",
    "10% F.S",
    "20% F.S",
    //"50% F.S",
    //"100% F.S",
};
DEF_LIST_DLG(gWD_WetBootZeroRange, 20, 20, 200, 88, "开机置零范围", cWet_BootZeroRange, sizeof(cWet_BootZeroRange)/sizeof(char *), 1, 0);


/*
 * 手动置零范围
 */
static const u16 usWetManualZeroRange[] = {
    0,
    1,
    2,
    3,
    4,
    //10,
    //20,
    //50,
	//100,
};

static char* cWet_ManualZeroRange[] = {
    "关闭",
    "1% F.S",
    "2% F.S",
    "3% F.S",
    "4% F.S",
    //"10% F.S",
    //"20% F.S",
    //"50% F.S",
	//"100% F.S",
};
DEF_LIST_DLG(gWD_WetManualZeroRange, 20, 20, 200, 88, "手动置零范围", cWet_ManualZeroRange, sizeof(cWet_ManualZeroRange)/sizeof(char *), 1, 0);

/*
 * 零位跟踪时间
 */
static const uint8 fWetZeroTrackTime[] = {
    0,
    1,
	2,
    3,
    4,
	5
};

static char* cWet_ZeroTrackTime[] = {
    "不跟踪",
	"1S",
	"2S",
    "3S",
    "4S",
	"5S",
};
DEF_LIST_DLG(gWD_WetZeroTrackTime, 20, 20, 200, 88, "零位跟踪时间", cWet_ZeroTrackTime, sizeof(cWet_ZeroTrackTime)/sizeof(char *), 1, 0);

/*
 * 零位跟踪范围
 */
static uint8 fWetZeroTrackRange[] = {
    0,
    1,
    2,
    3,
    4,
	5,
	6
};
static char* cWet_ZeroTrackRange[] = {
    "0",
    "0.1e",
    "0.2e",
	"0.5e",
	"1e",
	"2e",
	"5e"
};
DEF_LIST_DLG(gWD_WetZeroTrackRange, 20, 20, 200, 88, "零位跟踪范围", cWet_ZeroTrackRange, sizeof(cWet_ZeroTrackRange)/sizeof(char *), 1, 0);

/*
 * 零位跟踪速度
 */
//static uint8 const fWetZeroTrackSpd[] = {
//		0,
//    1,
//    2,
//    3,
//		4,
//		5,
//};
static char* cWet_ZeroTrackSpd[] = {
		"0 d",
		"1 d",
    "2 d",
    "3 d",
    "4 d",
		"5 d",
};
DEF_LIST_DLG(gWD_WetZeroTrackSpd, 20, 20, 200, 88, "零位跟踪速度", cWet_ZeroTrackSpd, sizeof(cWet_ZeroTrackSpd)/sizeof(char *), 1, 0);

static char cMaxWet[12]="";
static char cMaxTrafficWet[12] = "";
//static char cZeroTrackSpeed[5]="";
static CAttrItem  _attritem[] = {
   {"静态分度值",NULL,1},
   {"动态分度值",NULL,2},
   {"最大称量",cMaxWet,3},
   {"开机置零范围",NULL,6},
   {"手动置零范围",NULL,7},
   {"零位跟踪时间",NULL,8},
   {"零位跟踪范围",NULL,9},   
   {"零位跟踪速度",NULL,10},
   {"红绿灯重量阈值",cMaxTrafficWet,11}
};

DEF_ATTR_CTRL(mWetAttr, &gWD_MeasureSet, 0, 20, 240, 108, "计量参数设置",(_attritem), sizeof(_attritem)/sizeof(CAttrItem), 0, CTRL_VISABLE);

static LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mWetAttr,
};

CWindow  gWD_MeasureSet = {
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
    Form_WetSet_Timer,
    Form_WetSet_Draw,
    Form_WetSet_Proc
};

//初始化界面函数，从铁电中读取相应的数据显示
void Form_MeasureSet_Init(void)
{
   memset(&gWetParmFlag,0,sizeof(SWetParmSet));
	//静态分度值
	switch(gWetPar.StaticDivValue)
	{
		case 1:
			gWetParmFlag.ucDivValueStaLabel = 0;
			break;
		case 2:
			gWetParmFlag.ucDivValueStaLabel = 1;
			break;
		case 5:
			gWetParmFlag.ucDivValueStaLabel = 2;
			break;
		case 10:
			gWetParmFlag.ucDivValueStaLabel = 3;
			break;
		case 20:
			gWetParmFlag.ucDivValueStaLabel = 4;
			break;
		case 50:
			gWetParmFlag.ucDivValueStaLabel = 5;
			break;
		case 100:
			gWetParmFlag.ucDivValueStaLabel = 6;
			break;
		case 200:
			gWetParmFlag.ucDivValueStaLabel = 7;
			break;
		default: 
			gWetParmFlag.ucDivValueStaLabel = 0;
			break;
	}
	
	//动态分度值
	switch(gWetPar.DynamicDivValue)
	{
		case 10:
			gWetParmFlag.ucDivValueDynaLabel = 0;
			break;
		case 20:
			gWetParmFlag.ucDivValueDynaLabel = 1;
			break;
		case 50:
			gWetParmFlag.ucDivValueDynaLabel = 2;
			break;
		case 100:
			gWetParmFlag.ucDivValueDynaLabel = 3;
			break;
		default: 
			gWetParmFlag.ucDivValueDynaLabel = 0;
			break;
	}

	//开机置零范围
	switch(gWetPar.PowerZero)
	{
		case 0:
			gWetParmFlag.ucBootZeroLabel = 0;		//关闭
			break;
		case 2:
			gWetParmFlag.ucBootZeroLabel = 1;		//2%
			break;
		case 5:
			gWetParmFlag.ucBootZeroLabel = 2;		//5%
			break;
		case 10:
			gWetParmFlag.ucBootZeroLabel = 3;		//10%
			break;
		case 20:
			gWetParmFlag.ucBootZeroLabel = 4;		//20%
			break;
		default:
			gWetParmFlag.ucBootZeroLabel = 0;
			break;
	}
	
	//手动置零范围
	switch(gWetPar.HandlZero)
	{
		case 0:
			gWetParmFlag.ucManualZeroLabel = 0;		//关闭
			break;
		case 1:
			gWetParmFlag.ucManualZeroLabel = 1;		//1%
			break;
		case 2:
			gWetParmFlag.ucManualZeroLabel = 2;		//2%
			break;
		case 3:
			gWetParmFlag.ucManualZeroLabel = 3;		//3%
			break;
		case 4:
			gWetParmFlag.ucManualZeroLabel = 4;		//4%
			break;
		default: 
			gWetParmFlag.ucManualZeroLabel = 0;
			break;
	}
	
	//零位跟踪时间
	switch(gWetPar.ZeroTruckTick/DYNAMIC_AD_RATE)
	{
		case 0:
			gWetParmFlag.ucZeroTarkTimeLabel = 0;		//不跟踪
			break;
		case 1:
			gWetParmFlag.ucZeroTarkTimeLabel = 1;		//1S
			break;
		case 2:
			gWetParmFlag.ucZeroTarkTimeLabel = 2;		//2S
			break;
		case 3:
			gWetParmFlag.ucZeroTarkTimeLabel = 3;		//3S
			break;
		case 4:
			gWetParmFlag.ucZeroTarkTimeLabel = 4;		//4S
			break;
		case 5:
			gWetParmFlag.ucZeroTarkTimeLabel = 5;		//5S
			break;
		default: 
			gWetParmFlag.ucZeroTarkTimeLabel = 0;
			gWetPar.ZeroTruckTick = 0;
			break;
	}

	switch(gWetPar.ZeroTruckRang)
	{
		case 0:
			gWetParmFlag.ucZeroTarkRangeLabel = 0;		//0
			break;
		case 1:
			gWetParmFlag.ucZeroTarkRangeLabel = 1;		//0.1e/s
			break;
		case 2:
			gWetParmFlag.ucZeroTarkRangeLabel = 2;		//0.2e/s
			break;
		case 3:
			gWetParmFlag.ucZeroTarkRangeLabel = 3;		//0.5e/s
			break;
		case 4:
			gWetParmFlag.ucZeroTarkRangeLabel = 4;		//1e/s
			break;
		case 5:
			gWetParmFlag.ucZeroTarkRangeLabel = 5;		//2e/s
			break;
		case 6:
			gWetParmFlag.ucZeroTarkRangeLabel = 6;		//5e/s
			break;
		default: 
			gWetParmFlag.ucZeroTarkRangeLabel = 0;
			gWetPar.ZeroTruckRang = 0;
			break;
	}

	switch(gWetPar.ZeroTruckSpeed)
	{
		case 0:
			gWetParmFlag.ucZeroTarkSpdLabel = 0;		//0
			break;
		case 1:
			gWetParmFlag.ucZeroTarkSpdLabel = 1;		//1
			break;
		case 2:
			gWetParmFlag.ucZeroTarkSpdLabel = 2;		//2
			break;
		case 3:
			gWetParmFlag.ucZeroTarkSpdLabel = 3;		//3
			break;
		case 4:
			gWetParmFlag.ucZeroTarkSpdLabel = 4;		//4
			break;
		case 5:
			gWetParmFlag.ucZeroTarkSpdLabel = 5;		//5
			break;
		case 6:
			gWetParmFlag.ucZeroTarkSpdLabel = 6;
			break;
		default: 
			gWetParmFlag.ucZeroTarkSpdLabel = 0;
			gWetPar.ZeroTruckSpeed = 0;
			break;
	}
}

void Form_WetSet_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
	if(lpWindow != NULL)
	{
	}
}

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

static void WetSet_Fun(LPWindow pWindow)
{
	int32  ulTemp;
	int16 flag=0;
	GuiMsgInfo guiMsgInfo;
	char   cTemp[30]="";
	char   cBuff[30]="";
	
	switch (CTRL_CONTENT(mWetAttr).focus) {
		case 0:
			  //静态分度值
			CTRL_CONTENT(gWD_WetDivValue_Sta).focus = gWetParmFlag.ucDivValueStaLabel;
			CTRL_CONTENT(gWD_WetDivValue_Sta).select = gWetParmFlag.ucDivValueStaLabel;
			flag =  LoadListDlg(&gWD_WetDivValue_Sta);	  //绘制子菜单
			if(flag >= 0)   
			{
				  gWetParmFlag.ucDivValueStaLabel = flag;
				  gWetPar.StaticDivValue = ucWetDivValue_Sta[gWetParmFlag.ucDivValueStaLabel];
			}
			if(!Param_Write(GET_U8_OFFSET(ParamStaticDivValue),&gWetPar.StaticDivValue, sizeof(u8)))//判断写入是否成功
			{
			  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
			  gWetParmFlag.ucDivValueStaLabel = 0;
			  break;
			} 						  
			Set_Scaler_StaticDrv(gWetPar.StaticDivValue);//算法使用设置的静态分度值
			flag=0;
			break;
				  
		case 1:
						  //动态分度值
			CTRL_CONTENT(gWD_WetDivValue_Dyna).focus = gWetParmFlag.ucDivValueDynaLabel;
			CTRL_CONTENT(gWD_WetDivValue_Dyna).select = gWetParmFlag.ucDivValueDynaLabel;
			flag =  LoadListDlg(&gWD_WetDivValue_Dyna);
			if(flag >= 0)   
			{
				gWetParmFlag.ucDivValueDynaLabel = flag;
				gWetPar.DynamicDivValue = ucWetDivValue_Dyna[gWetParmFlag.ucDivValueDynaLabel];
			}
			if(!Param_Write(GET_U8_OFFSET(ParamDynamicDivValue),&gWetPar.DynamicDivValue, sizeof(u8)))
			{
				MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
				gWetParmFlag.ucDivValueDynaLabel = 0;
				break;
			}
			Set_Scaler_DynamicDrv(gWetPar.DynamicDivValue);
			flag=0;
		    break;
	
		case 2:
			sprintf(cBuff,"请输入最大称量 (%s)",gcWet_Unit[gSysSet.unit]);
			//最大称量
			if( LoadEditDlg(&g_TextEditWindow, cBuff, cTemp, cTemp, 6)) 
			{
				ulTemp = atoi(cTemp);
				if (ulTemp) 
				{
					if(!Param_Write(GET_U32_OFFSET(ParamMaxWet),&ulTemp, sizeof(u32)))
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
						break;
					}

					gWetPar.MaxWet= CurVal2Kg(atoi(cTemp));
					if (gWetPar.MaxWet > 200000)
						gWetPar.MaxWet = 200000;
					if (gWetPar.MaxWet < 10000)
						gWetPar.MaxWet = 10000;
				}
			}

			break;
																																				  
		case 3:
					  //开机置零范围
			CTRL_CONTENT(gWD_WetBootZeroRange).focus = gWetParmFlag.ucBootZeroLabel;
			CTRL_CONTENT(gWD_WetBootZeroRange).select = gWetParmFlag.ucBootZeroLabel;
			flag = LoadListDlg(&gWD_WetBootZeroRange);
			if(flag >= 0)
			{
			   gWetParmFlag.ucBootZeroLabel = flag;
			   gWetPar.PowerZero =usWetBootZeroRange[gWetParmFlag.ucBootZeroLabel]; 				  
			}
			if(!Param_Write(GET_U8_OFFSET(ParamPowerZero),&gWetPar.PowerZero, sizeof(u8)))
			{
				  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
				  gWetParmFlag.ucBootZeroLabel = 0;
				  break;
			}
			flag = 0; 					  
			break;
	
		case 4:
				  //手动置零范围
		  CTRL_CONTENT(gWD_WetManualZeroRange).focus = gWetParmFlag.ucManualZeroLabel;
		  CTRL_CONTENT(gWD_WetManualZeroRange).select = gWetParmFlag.ucManualZeroLabel;
		  flag	= LoadListDlg(&gWD_WetManualZeroRange);   
		  if(flag >= 0)
		  {
			  gWetParmFlag.ucManualZeroLabel  = flag;  
			  gWetPar.HandlZero = usWetManualZeroRange[gWetParmFlag.ucManualZeroLabel];
		  }
		  if(!Param_Write(GET_U8_OFFSET(ParamHandlZero),&gWetPar.HandlZero, sizeof(u8)))
		  {
			  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
			  gWetParmFlag.ucManualZeroLabel  = 0;
			  break;
		  }
		  flag = 0;
		  break;
	
		case 5:
					  //零位跟踪时间
		  CTRL_CONTENT(gWD_WetZeroTrackTime).focus = gWetParmFlag.ucZeroTarkTimeLabel;
		  CTRL_CONTENT(gWD_WetZeroTrackTime).select = gWetParmFlag.ucZeroTarkTimeLabel;
		  flag = LoadListDlg(&gWD_WetZeroTrackTime);
		  if(flag >= 0) 
		  {
			  gWetParmFlag.ucZeroTarkTimeLabel = flag;	   
			  gWetPar.ZeroTruckTick = DYNAMIC_AD_RATE*fWetZeroTrackTime[gWetParmFlag.ucZeroTarkTimeLabel];				   
		  }
		  if(!Param_Write(GET_U16_OFFSET(ParamZeroTruckTick),&gWetPar.ZeroTruckTick, sizeof(u16)))
		  {
			  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
			  gWetParmFlag.ucZeroTarkTimeLabel = 0;  
			  break;
		  }
		  flag = 0;
		  break;
	
		  case 6:
		  //零位跟踪范围
			  CTRL_CONTENT(gWD_WetZeroTrackRange).focus = gWetParmFlag.ucZeroTarkRangeLabel;
			  CTRL_CONTENT(gWD_WetZeroTrackRange).select = gWetParmFlag.ucZeroTarkRangeLabel;
			  flag = LoadListDlg(&gWD_WetZeroTrackRange);
			  if(flag >= 0)
			  { 
				  gWetParmFlag.ucZeroTarkRangeLabel = flag;    
				  gWetPar.ZeroTruckRang= fWetZeroTrackRange[gWetParmFlag.ucZeroTarkRangeLabel]; 			
			  }
			  if(!Param_Write(GET_U8_OFFSET(ParamZeroTruckRang),&gWetPar.ZeroTruckRang, sizeof(u8)))
			  {
				  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
				  gWetParmFlag.ucZeroTarkRangeLabel = 0;
				  break;
			  }
			  flag = 0;
			  break;

		case 7:
		  //零位跟踪速度
		  CTRL_CONTENT(gWD_WetZeroTrackRange).focus = gWetParmFlag.ucZeroTarkRangeLabel;
		  CTRL_CONTENT(gWD_WetZeroTrackRange).select = gWetParmFlag.ucZeroTarkRangeLabel;
		  flag = LoadListDlg(&gWD_WetZeroTrackRange);
		  if(flag >= 0)
		  { 
			  gWetParmFlag.ucZeroTarkSpdLabel = flag;    
			  gWetPar.ZeroTruckSpeed= fWetZeroTrackRange[gWetParmFlag.ucZeroTarkSpdLabel]; 			
		  }
		  if(!Param_Write(GET_U8_OFFSET(ParamZeroTruckSpeed),&gWetPar.ZeroTruckSpeed, sizeof(u8)))
		  {
			  MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
			  gWetParmFlag.ucZeroTarkSpdLabel = 0;
			  break;
		  }
		  flag = 0;
		  break;

		case 8:
			sprintf(cBuff,"请输入重量阈值 (kg)");
			//最大称量
			if( LoadEditDlg(&g_TextEditWindow, cBuff, cTemp, cTemp, 5)) 
			{
				ulTemp = atoi(cTemp);
				if (ulTemp >= 50) 
				{
					if(!Param_Write(GET_U32_OFFSET(ParamTrafficWet),&ulTemp, sizeof(u32)))
					{
						MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "参数设置失败，请重新设置");
						break;
					}
		
					gWetPar.MaxTrafficWet= ulTemp;  //CurVal2Kg(atoi(cTemp));
				}
				else
				{
					MsgBoxDlg(&g_MsgBoxDlg, "错误提示", "阈值需要大于等于50kg!");
					break;
				}
			}
		
			break;

		default:
			break;
	}
	guiMsgInfo.pWindow = pWindow;
	guiMsgInfo.ID = WM_SHOW;
	GuiMsgQueuePost(&guiMsgInfo); 	
	Init_Wet_Par();
}

void Form_WetSet_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	
	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			SysTimeDly(15);
			CTRL_CONTENT(mWetAttr).focus = 0;
      		Form_MeasureSet_Init();

		case WM_SHOW:
            _attritem[0].str = cWet_DivValue_Sta[gWetParmFlag.ucDivValueStaLabel];
            _attritem[1].str = cWet_DivValue_Dyna[gWetParmFlag.ucDivValueDynaLabel];
			//最大称量			
			sprintf(cMaxWet,"%d (%s)",gWetPar.MaxWet,"kg");
            _attritem[3].str = cWet_BootZeroRange[gWetParmFlag.ucBootZeroLabel];
            _attritem[4].str = cWet_ManualZeroRange[gWetParmFlag.ucManualZeroLabel];
            _attritem[5].str = cWet_ZeroTrackTime[gWetParmFlag.ucZeroTarkTimeLabel];//[gWetPar.ZeroTruckTick/800];
			_attritem[6].str = cWet_ZeroTrackRange[gWetParmFlag.ucZeroTarkRangeLabel];  
			_attritem[7].str = cWet_ZeroTrackRange[gWetParmFlag.ucZeroTarkSpdLabel];
			snprintf(cMaxTrafficWet, 11, "%d (%s)", gWetPar.MaxTrafficWet, "kg");
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
                    if (CTRL_CONTENT(mWetAttr).focus > 0) {
                        CTRL_CONTENT(mWetAttr).focus--;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mWetAttr, 0);
                    }
					break;
				case KEY_DOWN:
                    if (CTRL_CONTENT(mWetAttr).focus + 1 < CTRL_CONTENT(mWetAttr).total) {
                        CTRL_CONTENT(mWetAttr).focus++;
            			PostWindowMsg(pWindow, WM_UPDATECTRL, (uint32)&mWetAttr, 0);
                    }
					break;

				case KEY_LEFT:
					break;

				case KEY_OK:
					WetSet_Fun(pWindow);
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



