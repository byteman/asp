#ifndef __GUI_WINDOWS_H
#define __GUI_WINDOWS_H
#include "includes.h"



struct _CControl;
struct _CWindow;
struct _GuiMsgInfo;

typedef void (*GuiDrawTypeDef)(struct _CControl* pCControl);
typedef uint16 (*ControlProcTypeDef)(struct _CControl* pCControl, struct _GuiMsgInfo* pGuiMsgInfo);

typedef void (*WindowDrawTypeDef)(struct _CWindow* pWindow);
typedef void (*WindowProcTypeDef)(struct _CWindow* pWindow, struct _GuiMsgInfo* pGuiMsgInfo);

//窗口风格
//window styles
#define WS_VISABLE		 0x00000001L		//窗口可视
#define WS_CHILD		 0x00000002L		//窗口是否为子窗口
#define WS_POPUP	   	 0x00000004L		//弹出窗口
#define WS_DISABLED     0x00000008L		//窗口是否被禁止


//控件状态
//control state
#define CTRL_VISABLE	0x0001		//控件可视
#define CTRL_FOCUS		0x0002		//控件具有输入焦点


//对齐方式
//test allign
#define TA_LEFT			0x0001
#define TA_CENTER		0x0002
#define TA_RIGHT		0x0004
#define TA_TOP			0x0010
#define TA_VCENTER	0x0020
#define TA_BOTTOM		0x0040

//提取控件内容
#define CTRL_CONTENT(Name)	 Name##CC
#define CTRL_NAME(Name)		 Name##CTRL

typedef struct _CControl
{
	//属性
	struct _CWindow*	pParent;			//父窗口
	int16	x;												//显示的x轴起始位置
	int16	y;												//显示的y轴起始位置
	int16	sx;												//显示的x轴长度
	int16	sy;												//显示的y轴长度
	int16	state;										//显示状态
	int16	align;										//对齐方式
	void*	pContent;					   			//控件内容

	//方法
	GuiDrawTypeDef			DrawFunc;		//控件绘图函数
	ControlProcTypeDef	ProcFunc;		//控件处理函数
}CControl, *LPControl;

#define SYS_TMR	int
#define SYS_TMR_CALLBACK int

typedef struct _CWindow
{
	LPControl*			pLPControls;//控件地址
	int16				nNbControls;		//控件个数
	int16				nCurControl;
	int16				nViewPosX;			//视图水平位置(相对于物理窗口，单位为像素)
	int16				nViewPosY;			//视图垂直位置(相对于物理窗口，单位为像素)
	int16				nViewSizeX;			//视图水平尺寸(单位为像素)
	int16				nViewSizeY;			//视图垂直尺寸(单位为像素)
	uint32				nStyle;				//窗口风格
	struct _CWindow*	pParentWindow;		//父窗口

	uint32				period;				//定时器周期
	SYS_TMR*				pTimer;
	SYS_TMR_CALLBACK		tmrCallBack;	//定时器回调函数

	WindowDrawTypeDef	DrawFunc;			//窗口绘图函数
	WindowProcTypeDef	ProcFunc;			//窗口处理函数
}CWindow, *LPWindow;

typedef enum
{
	WM_LOAD,
	WM_SHOW,
	WM_CLOSE,	   
	WM_RETURN,
	WM_KEYDOWN,
	WM_KEYUP,
	WM_TIMER,
	WM_TIMEUPDATE,		//时间更新
	WM_UPDATECTRL,		//刷新控件
	WM_SUBMENU,			
	WM_ICREAD,			//刷卡消息
	WM_INPUTUSERID,		//输入用户卡号消息
	WM_CARIN,           //车辆进入
	WM_CARBACK,         //车辆退出
	WM_AXELADD,         //增加轴数据
	WM_AXELSUB,         //减小轴数据
	WM_VEHWET,          //整车称重完成
	WM_VEHOVERSPEED,    //超速
	WM_VEHSTA,           //称重状态
	
	
	WM_LCDTest,					//屏幕测试
} GuiMsgID;				//窗体消息ID号


typedef struct _GuiMsgInfo		//GUI消息信息
{

	uint16		ID;				//GUI消息号
	uint32		wParam;
	uint32		lParam;
	LPWindow	pWindow;
} GuiMsgInfo, *LPGuiMsgInfo;





uint8 GuiMsgQueuePost(GuiMsgInfo* pGuiMsgInfo);


extern CWindow*	g_pCurWindow;	//当前窗体


#endif
