#include "includes.h"
#include "Form_Calibrate.h"
#include "Form_MainMenu.h"
#include "Ctr_StatusBar.h"
#include "Form_Main.h"
#include "Form_Resource.h"
#include "../Inc/TextEdit.h"
#include "../Inc/Menu.h"
#include "Form_MainMenu.h"
#include "Form_MsgBox.h"
#include "Common.h"
#include "sys_Param.h"	

extern  CControl  gStatusBar; 		//状态栏

extern SCalibrationData    gCalibrationDataA;           // A秤台标定数据
extern SCalibrationData    gCalibrationDataB;           // B秤台标定数据
extern SCalibrationParam   gCalibrationParamA;
extern SCalibrationParam   gCalibrationParamB;
extern SWeightMeas gWeightMeas;                         //静态称重输出

//static int8 mFitCount = 0;
//static SFitPoint  mFitPoint[FIT_POINT_MAXNUM] = {0};
static SCalibrationData mCalib;
__align(8) static char sScale_Title[16] = "";
__align(8) static char sCali_Title[32] = "";
__align(8) char sAD1_Value[24] = "";
__align(8) char sAD2_Value[24] = "";
__align(8) static char sZero1_Value[24] = "";
__align(8) static char sZero2_Value[24] = "";
static long fZero_Value1 = 0;
static long fZero_Value2 = 0;
static long lWeights_Value;   // 砝码重
static SCalibrationData* gpCalibrationData = 0;
static SCalibrationParam* gpCalibrationParam = 0;
static long *pADValue1,*pADValue2;


enum  _CalibCount {
    Calib_Point0=0,
    Calib_Point1,
    Calib_Point2,
    Calib_Point3,
    Calib_Point4,
    Calib_Point5,
    Calib_End
};
enum  _CalibStat {
    Calib_Step1=1,
    Calib_Step2,
    Calib_Step3,
    Calib_Step4,
};
static char gCalibCount = Calib_Point0;
static char gCalibStat  = Calib_Step1;

static void Form_Calibrate_Timer(void *ptmr, void *parg);
static void Form_Calibrate_Draw(LPWindow pWindow);
static void Form_Calibrate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

//DEF_TEXT_LABEL(mAD_Title, &gWD_Calibrate,       5, 44, 40, 18, CTRL_VISABLE, "AD码");
//DEF_TEXT_LABEL(mZero_Title, &gWD_Calibrate,     5, 66, 40, 18, CTRL_VISABLE, "零点");
//DEF_TEXT_LABEL(mWeights_Title, &gWD_Calibrate,  5, 88, 40, 18, CTRL_VISABLE, "砝码");
//DEF_TEXT_LABEL(mScale_Title, &gWD_Calibrate,    5, 110, 40, 18, CTRL_VISABLE, "秤台");

DEF_TEXT_LABEL(mAD1_Attr,   &gWD_Calibrate,      45, 44, 90, 18, CTRL_VISABLE, sAD1_Value);
DEF_TEXT_LABEL(mAD2_Attr,   &gWD_Calibrate,      145, 44, 90, 18, CTRL_VISABLE, sAD2_Value);
DEF_TEXT_LABEL(mZero1_Attr, &gWD_Calibrate,      45, 66, 90, 18, CTRL_VISABLE, sZero1_Value);
DEF_TEXT_LABEL(mZero2_Attr, &gWD_Calibrate,      145, 66, 90, 18, CTRL_VISABLE, sZero2_Value);
DEF_STATUS_IMAGE_CTRL(mCalibSteadyImage, &gWD_Calibrate, 220, 21, 14, 14, CTRL_VISABLE, Img_steady, Img_unsteady, FALSE);

static const LPControl marrLPControl[] = 
{
    &gStatusBar,
	&mAD1_Attr,
	&mAD2_Attr,
    &mZero1_Attr,
    &mZero2_Attr,
    &mCalibSteadyImage,
};

CWindow  gWD_Calibrate = {
    (LPControl *)marrLPControl,
	sizeof(marrLPControl)/sizeof(LPControl),
    0,
    0,
    0,
    240,
    128,
    WS_VISABLE,
	NULL,//(pParentWindow)
    5,//5
    NULL,
    Form_Calibrate_Timer,
    Form_Calibrate_Draw,
    Form_Calibrate_Proc
};

void Form_Calibrate_Timer(void *ptmr, void *parg)
{
	LPWindow lpWindow = (LPWindow)parg;
    extern SWeightMeas gWeightMeas;         //静态称重输出
	if(lpWindow != NULL) {
    	if(g_pCurWindow == &gWD_Calibrate) {
            sprintf(sAD1_Value,"%d",*pADValue1);        
            sprintf(sAD2_Value,"%d",*pADValue2);
            if(CTRL_CONTENT(mCalibSteadyImage).bHot != GetWetSteadySta()) {
                CTRL_CONTENT(mCalibSteadyImage).bHot = GetWetSteadySta();
                PostWindowMsg(WM_UPDATECTRL,(u32)&mCalibSteadyImage,0);
            }
            EraseRect(45, 44, 90, 18);
            EraseRect(145, 44, 90, 18);
    		PostWindowMsg(WM_UPDATECTRL, (uint32)&mAD1_Attr, 0);
    		PostWindowMsg(WM_UPDATECTRL, (uint32)&mAD2_Attr, 0);
    	}
    }
}

void Form_Calibrate_Draw(LPWindow pWindow)
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
    sprintf(sZero1_Value,"%d",fZero_Value1);
    sprintf(sZero2_Value,"%d",fZero_Value2);
    switch (gCalibCount) {

    case Calib_Point0:
        sprintf(sCali_Title,"%s",sScale_Title);
		DrawGbText("零位标定", 112, 88);
		DrawGbText("按确定键开始标定，否则请返回", 42, 110);
        break;

    case Calib_Point1:
    case Calib_Point2:
    case Calib_Point3:
    case Calib_Point4:
    case Calib_Point5:
        switch(gCalibStat) {
            
        case Calib_Step1:
            DrawRect(90,86,100,40);
            DrawRect(91,87,98,38);
    		FillRect(120, 106, 40, 20);
    		InvertColor();
    		DrawGbText("砝码", 126, 109);
    		InvertColor();
            sprintf(sCali_Title,"%s——标定点%d-1(%dkg)",sScale_Title,gCalibCount,lWeights_Value);
            break;
            
        case Calib_Step2:
            DrawRect(90,86,100,40);
            DrawRect(91,87,98,38);
    		FillRect(120, 86, 40, 20);
    		InvertColor();
    		DrawGbText("砝码", 126, 89);
    		InvertColor();
            sprintf(sCali_Title,"%s——标定点%d-2(%dkg)",sScale_Title,gCalibCount,lWeights_Value);
            break;
            
        case Calib_Step3:
    		DrawGbText("标定成功", 112, 88);
    		DrawGbText("按确定键保存数据，否则请返回", 42, 110);
            sprintf(sCali_Title,"%s——标定点%d(%dkg)",sScale_Title,gCalibCount,lWeights_Value);
            break;
            
        case Calib_Step4:
    		DrawGbText("标定完成", 112, 88);
    		DrawGbText("按确定键继续标定，否则请返回", 42, 110);
            sprintf(sCali_Title,"%s——标定点%d(%dkg)",sScale_Title,gCalibCount,lWeights_Value);
            break;
            
        default:
            break;
        }
        break;

    case Calib_End:
		DrawGbText("标定完成", 112, 88);
		//DrawGbText("请返回", 42, 110);
        break;

    default:
        break;
    }
    DrawGbText("AD码",5,44);
    DrawGbText("零点",5,66);
    DrawGbText("砝码",5,91);
    DrawGbText("位置",5,107);
	DrawGbText(sCali_Title, 5, 21);
    DrawHoriLine(0, 40, 240);
    DrawHoriLine(0, 62, 240);
    DrawHoriLine(0, 84, 240);
    DrawHoriLine(0, 127, 240);
    DrawVertLine(40, 40, 88);
    DrawVertLine(140, 40, 44);

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
static char CalibrateProc(void)
{
    int i,j;
    u32 mwet;
    SFitPoint mTemp1;
    SFitPoint mTemp2;
    SCalibrationParam *pCalib;
    
    if(!mCalib.ucFitConut) {
        return 1;
    }
    for (i = 0; i < mCalib.ucFitConut; i ++) {
        for (j = i+1; j <= mCalib.ucFitConut; j ++) {
            if(mCalib.Wet[i] > mCalib.Wet[j]) {
                mwet   =  mCalib.Wet[j];
                mTemp1.AD1=  mCalib.FitPoint1[j].AD1;
                mTemp1.AD2=  mCalib.FitPoint1[j].AD2;
                mTemp2.AD1=  mCalib.FitPoint2[j].AD1;
                mTemp2.AD2=  mCalib.FitPoint2[j].AD2;
                mCalib.Wet[j]    = mCalib.Wet[i];
                mCalib.FitPoint1[j].AD1=  mCalib.FitPoint1[i].AD1;
                mCalib.FitPoint1[j].AD2=  mCalib.FitPoint1[i].AD2;
                mCalib.FitPoint2[j].AD1=  mCalib.FitPoint2[i].AD1;
                mCalib.FitPoint2[j].AD2=  mCalib.FitPoint2[i].AD2;
                
                mCalib.Wet[i]= mwet;
                mCalib.FitPoint1[i].AD1= mTemp1.AD1;
                mCalib.FitPoint1[i].AD2= mTemp1.AD2;
                mCalib.FitPoint2[i].AD1= mTemp2.AD1;
                mCalib.FitPoint2[i].AD2= mTemp2.AD2;
            }
        }
    }

    memcpy((char *)gpCalibrationData,(char *)&mCalib,sizeof(SCalibrationData));
    pCalib = CalibCalc(gpCalibrationData);
    memcpy((char *)gpCalibrationParam,(char *)pCalib,sizeof(SCalibrationParam));
	CalibrationParamStore();//存储
    return 0;
}

void Form_Calibrate_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
    char   cBuff[20]="";
    char   cTemp[10]="";

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
            memset((char *)&mCalib,0,sizeof(SCalibrationData));
            if(pGuiMsgInfo->wParam ==1) {
                pADValue1 = &gWeightMeas.AD1;
                pADValue2 = &gWeightMeas.AD2;
                gpCalibrationData = &gCalibrationDataA;
                gpCalibrationParam= &gCalibrationParamA;
                sprintf(sScale_Title,"零点标定");
            } else {
                pADValue1 = &gWeightMeas.AD3;
                pADValue2 = &gWeightMeas.AD4;
                gpCalibrationData = &gCalibrationDataB;
                gpCalibrationParam= &gCalibrationParamB;
                sprintf(sScale_Title,"重量标定");
            }
			ClearScreen();
			SysTimeDly(3);
			CreateWindowTimer(&gWD_Calibrate);
			gCalibCount = Calib_Point0;
		case WM_SHOW:
			StartWindowTimer(&gWD_Calibrate);
			pWindow->DrawFunc(pWindow);
			break;

		case WM_UPDATECTRL:
			pControl = (CControl*)(pGuiMsgInfo->wParam);
			if(pControl != NULL)
			{
                if (pControl->state & CTRL_VISABLE)
                    pControl->DrawFunc(pControl);
			} 			
			break;

		case WM_TIMEUPDATE:
			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
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
                    switch (gCalibCount) {

                    case Calib_Point0:
                        //if (GetWetSteadySta()) {	 //取消对稳定的检测
                            fZero_Value1 = *pADValue1;
                            fZero_Value2 = *pADValue2;
                            sprintf(cBuff,"请输入砝码重量 (%s)",GetCurUnitStr());
        					if(LoadEditDlg(&g_TextEditWindow, cBuff, cTemp, cTemp, 10)) 
							{
                                if (atoi(cTemp)) 
								{
                                    mCalib.ucFitConut = 0;
                                    mCalib.Wet[mCalib.ucFitConut]= 0;
                                    mCalib.FitPoint1[mCalib.ucFitConut].AD1= fZero_Value1;
                                    mCalib.FitPoint1[mCalib.ucFitConut].AD2= fZero_Value2;
                                    mCalib.FitPoint2[mCalib.ucFitConut].AD1= fZero_Value1;
                                    mCalib.FitPoint2[mCalib.ucFitConut].AD2= fZero_Value2;
                                    lWeights_Value = atoi(cTemp);
                                    gCalibCount = Calib_Point1;
                                    gCalibStat  = Calib_Step1;
                                }
								else
								{
									MsgBoxDlg(&g_MsgBoxDlg, "错误", "输入值非法，请重新输入");
								}
        					}
                       // }
                        break;

                    case Calib_Point1:
                    case Calib_Point2:
                    case Calib_Point3:
                    case Calib_Point4:
                        switch(gCalibStat) {

                        case Calib_Step1:
                            //if (GetWetSteadySta()) {
                                mCalib.Wet[mCalib.ucFitConut+1] = lWeights_Value;
                                mCalib.FitPoint1[mCalib.ucFitConut+1].AD1= *pADValue1;
                                mCalib.FitPoint1[mCalib.ucFitConut+1].AD2= *pADValue2;
                                gCalibStat++;
                            //}
                            break;

                        case Calib_Step2:
                            //if (GetWetSteadySta()) {
                                mCalib.FitPoint2[mCalib.ucFitConut+1].AD1= *pADValue1;
                                mCalib.FitPoint2[mCalib.ucFitConut+1].AD2= *pADValue2;
                                gCalibStat++;
                            //}
                            break;

                        case Calib_Step3:
                            mCalib.ucFitConut++;
                            gCalibStat++;
                            /*
                             *  只标一次，强制结束
                             */
                            {
                            gCalibStat = Calib_Step1;
                            gCalibCount = Calib_End;
                            }
                            break;

                        case Calib_Step4:
                            sprintf(cBuff,"请输入砝码重量 (%s)",GetCurUnitStr());
        					if( LoadEditDlg(&g_TextEditWindow, cBuff, cTemp, cTemp, 5)) {
								if (atoi(cTemp)) {	
                                    lWeights_Value = atoi(cTemp);
                                    gCalibCount++;
                                    gCalibStat = Calib_Step1;
                                }
        					}
                            break;

                        default:
                            break;
                        }
                        break;

                    case Calib_Point5:
                        switch(gCalibStat) {

                        case Calib_Step1:
                            //if (GetWetSteadySta()) {
                                mCalib.Wet[mCalib.ucFitConut+1]          = lWeights_Value;
                                mCalib.FitPoint1[mCalib.ucFitConut+1].AD1= *pADValue1;
                                mCalib.FitPoint1[mCalib.ucFitConut+1].AD2= *pADValue2;
                                gCalibStat++;
                            //}
                            break;

                        case Calib_Step2:
                            //if (GetWetSteadySta()) {
                                mCalib.FitPoint2[mCalib.ucFitConut+1].AD1= *pADValue1;
                                mCalib.FitPoint2[mCalib.ucFitConut+1].AD2= *pADValue2;
                                mCalib.ucFitConut++;
                                gCalibStat++;
                            //}
                            break;

                        case Calib_Step3:
                            gCalibStat = Calib_Step1;
                            gCalibCount = Calib_End;
                            break;

                        default:
                            break;
                        }
                        break;

                    case Calib_End:
                        break;

                    default:
                        break;
                    }
					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);					
					break;

				case KEY_RIGHT: 
					break;

				case KEY_BACK:
                    gCalibCount = Calib_Point0;
                    gCalibStat  = Calib_Step1;
        			StopWindowTimer(&gWD_Calibrate);
                    CalibrateProc();
                    CalibParamSync();
					if(pWindow->pParentWindow != NULL)
					{
						g_pCurWindow = pWindow->pParentWindow;
					}
					else
					{
						g_pCurWindow = &gWD_Main;
					}
                    PostWindowMsg(WM_LOAD,0,0);
					break;

				case KEY_1:
				case KEY_2:
				case KEY_3:
				case KEY_4:
				case KEY_5:
				case KEY_6:
				case KEY_7:
				case KEY_8:
				case KEY_9:
					break;
				default:
					break;
			}
			break;
		
		default:
			break;
	}	
}



