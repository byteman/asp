#ifndef __FORM_MSGBOX_H_
#define __FORM_MSGBOX_H_
#include "../Inc/GuiWindows.h"



//打开编辑框设置窗口
void MsgBoxDlg(CWindow* pDlg, char* pTitle, char* pMsg);
extern CWindow g_MsgBoxDlg;

#endif 
