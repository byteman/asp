#ifndef __WINDOWS_H
#define __WINDOWS_H
#include "includes.h"
#include "GDI.h"
#include "key_init.h"

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

// typedef enum
// {
// 	VK_NONE,					/*无效*/
// 	
// 	VK_DELETE,				/*删除*/
// 	VK_UNITS,					/*单位*/
// 	VK_SEEK,					/*查询*/
// 	VK_PRINT,					//打印		
// 	VK_UP,						/*上*/
// 	VK_DOWN,					/*下*/
// 	VK_LEFT,					/*左*/
// 	VK_RIGHT,					/*右*/
// 	VK_OK,						//确认
// 	VK_MENU,					//菜单
// 	VK_RETURN,				//返回	
// 	VK_WHOLE_CAR,			/*整车*/
// 	VK_AXLE_WEIGHT,		/*轴重*/
// 	VK_DIAGNOSE,			/*诊断*/
// 	VK_SET_ZERO,			/*置零*/
// 	VK_0,
// 	VK_1,
// 	VK_2,
// 	VK_3,
// 	VK_4,
// 	VK_5,
// 	VK_6,
// 	VK_7,
// 	VK_8,
// 	VK_9,
// 	VK_PLUS_MINUS,		/*加减*/
// 	VK_POINT,					/*小数点*/
// 	VK_F1,						/*扩展用*/
// 	VK_F2,
// 	VK_F3,
// 	VK_F4,
// 	VK_F5,
// 	VK_F6,
// 	VK_F7,
// 	VK_F8,
//} VK_VAL;	 													//键值用于矩阵

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

/****************************************************************************
* 名	称：BOOL CreateWindowTimer(LPWindow lpWindow)
* 功	能：注册窗口定时处理函数1
* 入口参数：LPWindow lpWindow-需要注册的窗口结构体
* 出口参数：TRUE	-	注册成功
						FALSE	-	注册失败
* 说	明：定时处理函数指针，定时时间长度均有窗口结构体传入
****************************************************************************/
BOOL CreateWindowTimer(LPWindow lpWindow);

/****************************************************************************
* 名	称：BOOL CreateWindowTimerEx(LPWindow lpWindow, uint32 delay)
* 功	能：注册窗口定时处理函数2
* 入口参数：LPWindow lpWindow-需要注册的窗口结构体
						uint32 delay	首次进入定时处理函数的时间长度
* 出口参数：TRUE	-	注册成功
						FALSE	-	注册失败
* 说	明：定时处理函数指针，定时时间长度均有窗口结构体传入
****************************************************************************/
BOOL CreateWindowTimerEx(LPWindow lpWindow, uint32 delay);

/****************************************************************************
* 名	称：BOOL DestoryWindowTimer(LPWindow lpWindow)
* 功	能：注销窗口定时处理函数
* 入口参数：LPWindow lpWindow-需要注册的窗口结构体
* 出口参数：TRUE	-	注销成功
						FALSE	-	注销失败
* 说	明：定时处理函数指针，定时时间长度均有窗口结构体传入
****************************************************************************/
BOOL DestoryWindowTimer(LPWindow lpWindow);

/****************************************************************************
* 名	称：BOOL StartWindowTimer(LPWindow lpWindow)
* 功	能：启动窗口定时
* 入口参数：LPWindow lpWindow-需要启动的定时器的窗口结构体
* 出口参数：TRUE	-	启动成功
						FALSE	-	启动失败
* 说	明：
****************************************************************************/
BOOL StartWindowTimer(LPWindow lpWindow);

/****************************************************************************
* 名	称：BOOL StopWindowTimer(LPWindow lpWindow)
* 功	能：关闭窗口定时
* 入口参数：LPWindow lpWindow-需要关闭的定时器的窗口结构体
* 出口参数：TRUE	-	关闭成功
						FALSE	-	关闭失败
* 说	明：
****************************************************************************/
BOOL StopWindowTimer(LPWindow lpWindow);


//------------------TextLabel--------------
typedef struct _CTextLabel
{
	char*	s;
}CTextLabel;

void DrawTextLabel(CControl* pControl);

//定义文本控件
/****************************************************************************
* 名	称： DEF_TEXT_LABEL
* 功	能：	定义文本控件
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state			控件状态		（CTRL_VISABLE-控件可视TRL_FOCUS-控件具有输入焦点）
						char*				str				要显示的字符
* 出口参数：无
* 说	明：无			
****************************************************************************/
#define DEF_TEXT_LABEL(Name, pParent, x, y, cx, cy, state, str)							\
CTextLabel CTRL_CONTENT(Name) = {(str)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawTextLabel, NULL}

//------------------FloatLabel--------------
typedef struct _CFloatLabel
{
	float*	pF;									//要显示的浮点数的存放地址
	uint8	nFixPoint;						//显示几位小数
	float	min;									//允许的最小值
	float	max;									//允许的最大值
	char	unit[5];							//单位字符串  t
}CFloatLabel;

void DrawSmallFloatLabel(CControl* pControl);
void DrawBigFloatLabel(CControl* pControl);

//定义浮点数控件
//小字号
/****************************************************************************
* 名	称： DEF_SMALL_FLOAT_LABLE
* 功	能：	定义浮点数控件(小字号)
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state			控件状态		（CTRL_VISABLE-控件可视TRL_FOCUS-控件具有输入焦点）
						float*			pFloat		要显示的浮点数地址
						uint8				nFixPoint	显示几位小数
						float				fMin			允许的最小值
						float				fMax			允许的最大值
						char				strUnit		单位字符串
						int16				Align			对齐方式										
* 出口参数：无
* 说	明：无			
****************************************************************************/
#define DEF_SMALL_FLOAT_LABLE(Name, pParent, x, y, cx, cy, state, pFloat, nFixPoint, fMin, fMax, strUnit, Align)							\
CFloatLabel CTRL_CONTENT(Name) = {(pFloat), (nFixPoint), (fMin), (fMax), (strUnit)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (Align), (&CTRL_CONTENT(Name)), DrawSmallFloatLabel, NULL}

//大字号
/****************************************************************************
* 名	称： DEF_BIG_FLOAT_LABLE
* 功	能：	定义浮点数控件(大字号)
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state			控件状态		（CTRL_VISABLE-控件可视TRL_FOCUS-控件具有输入焦点）
						float*			pFloat		要显示的浮点数地址
						uint8				nFixPoint	显示几位小数
						float				fMin			允许的最小值
						float				fMax			允许的最大值
						char				strUnit		单位字符串
						int16				Align			对齐方式
* 出口参数：无
* 说	明：无			
****************************************************************************/
#define DEF_BIG_FLOAT_LABLE(Name, pParent, x, y, cx, cy, state, pFloat, nFixPoint, fMin, fMax, strUnit, Align)							\
CFloatLabel CTRL_CONTENT(Name) = {(pFloat), (nFixPoint), (fMin), (fMax), (strUnit)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (Align), (&CTRL_CONTENT(Name)), DrawBigFloatLabel, NULL}


//-------------------Status Image-------------
typedef struct _CStatusImage
{
	uint8*	pNormalImage;
	uint8*	pHotImage;
	BOOL	bHot;
	int16	sx;
	int16	sy;
}CStatusImage;

void DrawStatusImage(CControl* pControl);

//定义状态图形控件
/****************************************************************************
* 名	称： DEF_STATUS_IMAGE_CTRL
* 功	能：	定义状态图形控件
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state						控件状态		（CTRL_VISABLE-控件可视TRL_FOCUS-控件具有输入焦点）
						uint8*			pImageNormal		要显示的图像1的数组首地址
						uint8*			pImageHot				要显示的图像2的数组首地址
						BOOL				Status					选择要显示的图像（TRUE-图像1；FALSE-图像2）
* 出口参数：无
* 说	明：无			
****************************************************************************/
#define DEF_STATUS_IMAGE_CTRL(Name, pParent, x, y, cx, cy, state, pImageNormal, pImageHot, Status)							\
CStatusImage CTRL_CONTENT(Name) = {(pImageNormal), (pImageHot), (Status), (cx), (cy)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawStatusImage, NULL}

//-------------------Image Control------------
typedef struct _CImageCtrl
{
	uint8*	pImage;
	int16	sx;
	int16	sy;
}CImageCtrl;

void DrawImageCtrl(CControl* pControl);

//定义图形控件
/****************************************************************************
* 名	称： DEF_IMAGE_CTRL
* 功	能：	定义图形控件
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state			控件状态		
						uint8*			pImage		要显示图像的起始地址(建议定义为static const uint8 类型)
* 出口参数：无
* 说	明：无			
****************************************************************************/
#define DEF_IMAGE_CTRL(Name, pParent, x, y, cx, cy, state, pImage)							\
CImageCtrl CTRL_CONTENT(Name) = {(pImage), (cx), (cy)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawImageCtrl, NULL}

//-------------------Custom Text Control--------
typedef struct _CCustomTextCtrl
{
	uint8*	pArr;
	int16	sx;					//每个字的宽度(单位：像素)
	int16	sy;					//每个字的高度(单位：像素)
	uint16  nWordSize;			//每个字的字模生成的字节数
	uint16	nCount;				//自定义文本控件包含多少个字
}CCustomTextCtrl;

void DrawCustomTextCtrl(CControl* pControl);

//定义自定制文本控件
/****************************************************************************
* 名	称： DEF_CUSTOM_TEXT_CTRL
* 功	能：	定义自定制文本控件
* 入口参数：自定义字符	Name			控件名
						CWindow*		pParent		该控件要显示的所在窗口
						int16				x					x轴起始位置
						int16				y					y轴起始位置
						int16				cx				x轴显示长度
						int16				cy				y轴显示长度
						int16				state			控件状态		
						uint8*			pMatrix		要显示的文本起始地址(建议定义为static const uint8 类型)
						int16				wcx				每个字的宽度(单位:像素)
						int16				wcy				每个字的高度(单位:像素)
						uint16			wSize			每个字的字模生成的字节数
						uint16			wCount		自定义文本控件包含多少个字
* 出口参数：无
* 说	明：	无			
****************************************************************************/
#define DEF_CUSTOM_TEXT_CTRL(Name, pParent, x, y, cx, cy, state, pMatrix, wcx, wcy, wSize, wCount)							\
CCustomTextCtrl CTRL_CONTENT(Name) = {(pMatrix), (wcx), (wcy), (wSize), (wCount)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawCustomTextCtrl, NULL}

//-------------------Rect Control--------
typedef struct _CRectCtrl
{
	int16	x;
	int16	y;		
	int16	cx;
	int16	cy;
}CRectCtrl;

void DrawRectCtrl(CControl* pControl);

//定义矩形控件
#define DEF_RECT_CTRL(Name, pParent, x, y, cx, cy, state)							\
CRectCtrl CTRL_CONTENT(Name) = {(x), (y), (cx), (cy)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawRectCtrl, NULL}

//------------------Cartoon Control-------------
#define CARTOON_STATE_NORMAL	0		//第一种状态
#define CARTOON_STATE_HOT		1		//第二种状态
#define CARTOON_STATE_ALTER		2		//交替状态，动画

typedef struct _CCartoonCtrl
{
	LPControl			pControl;				   //宿主指针
	uint8*				pNormalImage;
	uint8*				pHotImage;
	uint16				cartoonState;
	int16				sx;
	int16				sy;

	uint16				nFrame;						//动画时处于哪一帧
	uint16				period;						//定时器周期
	SYS_TMR*				pTimer;
	SYS_TMR_CALLBACK		tmrCallBack;
	
}CCartoonCtrl, *LPCartoonCtrl;

BOOL CreateCartoonCtrl(LPCartoonCtrl lpCartoonCtrl);

BOOL DestoryCartoonCtrl(LPCartoonCtrl lpCartoonCtrl);

BOOL StartCartoonCtrl(LPCartoonCtrl lpCartoonCtrl);

BOOL StopCartoonCtrl(LPCartoonCtrl lpCartoonCtrl);

void CartoonTmrCallBack(void *ptmr, void *parg);

void DrawCartoonCtrl(CControl* pControl);

//定义动画图形控件
#define DEF_CARTOON_CTRL(Name, pParent, x, y, cx, cy, state, pImageNormal, pImageHot, Status, period)								\
extern CControl Name;\
CCartoonCtrl CTRL_CONTENT(Name) = {(&Name), (pImageNormal), (pImageHot), (Status), (cx), (cy), 0, period, NULL, CartoonTmrCallBack};		\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), DrawCartoonCtrl, NULL}



#define DEF_STATUS_BAR(Name, pParent, x, y, cx, cy, sta_gm, sta_dg, sta_lz, voice, error, unit, time, state)  \
CStatusBar CTRL_CONTENT(Name) = {(sta_gm),(sta_dg),(sta_lz),(voice),(error),(unit),(time)};                     \
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), Form_StatusBar_Draw, NULL}



extern CWindow*	g_pCurWindow;	//当前窗体

//-------------GUI消息队列-------------------
#define GUIMSGSIZE		16		//GUI消息队列大小

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

} GuiMsgInfo, *LPGuiMsgInfo;

extern GuiMsgInfo			m_GuiMsgInfoArr[GUIMSGSIZE];	   //GUI消息信息存储区

extern void*				m_GuiMsgPointArr[GUIMSGSIZE];	   //GUI消息信息指针数组

extern SYS_EVENT 			*m_pEvtGuiMsg;					   //GUI消息队列事件

/****************************************************************************
* 名	称：void  GuiMsgQueueCreate(void)
* 功	能：创建GUI消息队列
* 入口参数：无
* 出口参数：无
* 说	明：
****************************************************************************/
void  GuiMsgQueueCreate(void);

/****************************************************************************
* 名	称int8 GuiMsgQueuePost(GuiMsgInfo* pGuiMsgInfo)
* 功	能：发送GUI消息队列
* 入口参数：GuiMsgInfo* pGuiMsgInfo	要发送的消息
* 出口参数：SYS_ERR_NONE						没有错误
						SYS_ERR_Q_FULL					消息队列已满
						SYS_ERR_EVENT_TYPE			pevent 不是指向消息邮箱的指针
						SYS_ERR_PEVENT_NULL			如果pevent是一个空指针
* 说	明：
****************************************************************************/
uint8 GuiMsgQueuePost(GuiMsgInfo* pGuiMsgInfo);

/****************************************************************************
* 名	称void* GuiMsgQueuePend(GuiMsgInfo* pGuiMsgInfo, uint8* err)
* 功	能：等待GUI消息队列
* 入口参数：GuiMsgInfo* pGuiMsgInfo	要等待的消息
* 出口参数：uint8* err							错误类型的存放地址
* 说	明：
****************************************************************************/
void* GuiMsgQueuePend(GuiMsgInfo* pGuiMsgInfo, uint8* err);


/****************************************************************************
* 名	称uint8 PostWindowMsg(uint16 nID, uint32 wParam, uint32 lParam)
* 功	能：发送窗口消息
* 入口参数int16 nID			窗体下一步要执行的步骤
* 出口参数：uint32 wParam	消息发送的参数之1
						uint32 lParam	消息发送的参数之2
* 说	明：
****************************************************************************/
uint8 PostWindowMsg(uint16 nID, uint32 wParam, uint32 lParam);	//发送窗口消息







#endif
