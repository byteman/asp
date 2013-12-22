#include "Ctr_Attr.h"
#include "Form_Resource.h"

void Form_Attr_Draw(CControl* pControl)
{
    CAttr *pAttr = (CAttr *)(pControl->pContent);
    char s[20];
    CAttrItem *pitem;
    u16  i = 0;
    u8  pagenum = 0;
    u8  startpage = 0;
    u8  endpage = 0;
    u8  titellen=0;
    u8  line_x=0;
    u8  str_x=0;

	EnableScreenFlush(FALSE);
	SetColor(1);
	FillRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	SetColor(0);
    
    if(pAttr->titel != NULL) 
	{
        DrawGbText(pAttr->titel, pControl->x, pControl->y);
    }

    DrawHoriLine(pControl->x, pControl->y+16, pControl->sx);
    DrawHoriLine(pControl->x, pControl->y+18, pControl->sx);
    
	pagenum = (pControl->sy - 22) / 17;
    if(pAttr->total < pagenum) 
	{
        pagenum = pAttr->total;
    }

    if(pAttr->focus >= pAttr->total) 
	{
        pAttr->focus = pAttr->total - 1;
    }

    startpage = (pAttr->focus / pagenum) * pagenum;
    if((startpage + pagenum) >= pAttr->total) 
	{
        endpage = pAttr->total - 1;
    } 
	else 
	{
        endpage = startpage + pagenum - 1;
    }
    
    pitem = pAttr->item + startpage;
    for(i = startpage; i <= endpage; i++) 
	{
        if(pAttr->focus == i) 
		{
            DrawImage(Img_Right, pControl->x + 3, pControl->y+23+17*(i-startpage), 12, 12);
            //DrawGbText("¡Ì", pControl->x + 3, pControl->y+22+17*(i-startpage));
        }        
        sprintf(s,"%s£º",pitem->titel);
		DrawGbText(s, pControl->x + 20, pControl->y+22+17*(i-startpage));
        titellen = strlen(s);
        line_x    = pControl->x + 20 + titellen * 7;
        if(line_x < (pControl->x + 80)) {
            line_x = pControl->x + 80;
        }
        DrawHoriLine(line_x, pControl->y+36+17*(i-startpage), pControl->sx - (line_x - pControl->x));
        str_x = line_x + 10;
        if (str_x > pControl->x + 90) {
            if (line_x < pControl->x + 90) {
                str_x = pControl->x + 90;
            } else {
                str_x = line_x + 3;
            }
        }
        DrawGbText(pitem->str, str_x, pControl->y+22+17*(i-startpage));
        
		pitem++;
    }

	EnableScreenFlush(TRUE);
	FlushRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
}


