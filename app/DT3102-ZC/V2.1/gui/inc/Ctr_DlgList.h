#ifndef __DLG_LIST_H_
#define __DLG_LIST_H_
#include "../Inc/GuiWindows.h"
#include "Ctr_ListSelect.h"


//加载列表框
int16 LoadListDlg(CWindow* pDlg);

//设置列表框当前选项
int16 ListDlgSetCurSel(CWindow* pDlg, int16 nSel);





//内部调用函数=================================================================================
void Dlg_List_Draw(LPWindow pWindow);
void Dlg_List_Proc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

#define DEF_LIST_DLG(Name, x, y, cx, cy, titel, str, total, focus, select)			\
CListSelect CTRL_CONTENT(Name) = {(titel),(str),(total),(focus),(select)};					 	\
CControl CTRL_NAME(Name) = {(NULL), 1, 2, ((cx)-2), ((cy)-3), (CTRL_VISABLE), (0), (&CTRL_CONTENT(Name)), Form_ListSelect_Draw, NULL};	\
CWindow Name = {(LPControl*)(&CTRL_NAME(Name)), 1, 0, (x), (y), (cx), (cy), WS_VISABLE | WS_CHILD, (NULL), 0, NULL, NULL, Dlg_List_Draw, Dlg_List_Proc}	


#endif


