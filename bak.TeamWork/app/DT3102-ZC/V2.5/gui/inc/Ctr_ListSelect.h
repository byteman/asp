#ifndef  __CTR_LISTSELECT_H_
#define  __CTR_LISTSELECT_H_
#include "../Inc/GuiWindows.h"



typedef struct _CListSelect
{
    char*   titel;
	char**	str;
    int16   total;
    int16   focus;
    int16   select;
}CListSelect;

void Form_ListSelect_Draw(CControl* pControl);

#define DEF_LIST_SELECT(Name, pParent, x, y, cx, cy, titel, str, total, focus, select, state)			\
CListSelect CTRL_CONTENT(Name) = {(titel),(str),(total),(focus),(select)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), Form_ListSelect_Draw, NULL}







#endif

