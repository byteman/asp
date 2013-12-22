#ifndef __TEXT_EDIT_H_
#define __TEXT_EDIT_H_
#include "includes.h"
#include "GDI.h"
#include "guiwindows.h"



//打开编辑框设置窗口
/****************************************************************************
* 名	称：BOOL LoadEditDlg(CWindow* pDlg, char* pDlgName, char* pTextIn, char* pTextOut, uint16 nMaxLen)
* 功	能：打开编辑框设置窗口
* 入口参数：CWindow*	pDlg				父窗口指针
						char*			pDlgName		要显示的编辑框名
						char*			pTextIn			要显示的可编辑文本
						char*			pTextOut		修改后的文本的输出地址
						uint16		mMaxLen			文本的最大长度	
* 出口参数：
* 说	明：无
****************************************************************************/
BOOL LoadEditDlg(CWindow* pDlg, char* pDlgName, char* pTextIn, char* pTextOut, uint16 nMaxLen);
extern CWindow g_TextEditWindow;


//内部调用===========================================================================================
#define TEXT_EDIT_MAX_CHAR		36

//------------------TextEdit--------------
typedef struct _CTextEdit
{
	int16	nMaxAllowed;					   	//最多允许的字符编辑数
	int16	nCurLen;							//当前写入的字符数目
	int16	nCurPos;							//当前正编辑的字符索引
	char*	pMaskBuff;							//掩码缓冲区
	char*	pTextIn;							//初始化字符串的指针
	char	arrDigit[TEXT_EDIT_MAX_CHAR];		//字符组
}CTextEdit;

void LoadTextEdit(CControl* pControl);
void ApplyTextEdit(CControl* pControl);
void DrawTextEdit(CControl* pControl);
uint16 ProcTextEdit(CControl* pControl, struct _GuiMsgInfo* pGuiMsgInfo);

//定义浮点数控件
//小字号
#define DEF_TEXT_EDIT(Name, pParent, x, y, cx, cy, state, pTextIn, nMaxAllowed, pMaskBuff, Align)	 		\
CTextEdit CTRL_CONTENT(Name) = {(nMaxAllowed), 0, 0, pMaskBuff, pTextIn};					 	 \
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (Align), (&CTRL_CONTENT(Name)), DrawTextEdit, ProcTextEdit} \

extern void DefaultTextEditKeyProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);


#endif 
