#ifndef  __CTR_ATTR_H_
#define  __CTR_ATTR_H_
#include "../Inc/GuiWindows.h"



typedef struct _CAttrItem
{
    char*   titel;
	char*   str;
    uint8   cmd;
}CAttrItem;

typedef struct _CAttr
{
    char*       titel;
	CAttrItem*	item;
    uint8       total;
    uint8       focus;
}CAttr;

void Form_Attr_Draw(CControl* pControl);

#define DEF_ATTR_CTRL(Name, pParent, x, y, cx, cy, titel, item, total, focus, state)			\
CAttr   CTRL_CONTENT(Name) = {(titel),(item),(total),(focus)};					 	\
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), Form_Attr_Draw, NULL}







#endif

