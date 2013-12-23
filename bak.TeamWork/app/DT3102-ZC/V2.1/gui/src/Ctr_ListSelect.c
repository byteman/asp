#include "Ctr_ListSelect.h"


void Form_ListSelect_Draw(CControl* pControl)
{
    CListSelect *pListSelet = (CListSelect *)(pControl->pContent);
    char s[20];
    char **pstr;
    u16  i = 0;
    u8  pagenum = 0;
    u8  startpage = 0;
    u8  endpage = 0;

	EnableScreenFlush(FALSE);
	SetColor(1);
	FillRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
	SetColor(0);
    
    if(pListSelet->titel != NULL) 
	{
        DrawGbText(pListSelet->titel, pControl->x, pControl->y);
    }

    DrawHoriLine(pControl->x, pControl->y+16, pControl->sx);
    DrawHoriLine(pControl->x, pControl->y+18, pControl->sx);
    
	pagenum = (pControl->sy - 19) / 15;
    if(pListSelet->total < pagenum) 
	{
        pagenum = pListSelet->total;
    }

    if(pListSelet->focus >= pListSelet->total) 
	{
        pListSelet->focus = pListSelet->total - 1;
    }

    if(pListSelet->select >= pListSelet->total) 
	{
        pListSelet->select = pListSelet->total - 1;
    }

    startpage = (pListSelet->focus / pagenum) * pagenum;
    if((startpage + pagenum) >= pListSelet->total) 
	{
        endpage = pListSelet->total - 1;
    } 
	else 
	{
        endpage = startpage + pagenum - 1;
    }
    
    pstr = pListSelet->str + startpage;
    for(i = startpage; i <= endpage; i++) 
	{
        if(pListSelet->select == i) 
		{
            DrawGbText("¡Ì", pControl->x + 3, pControl->y+20+15*(i-startpage));
        }
        if(pListSelet->focus == i) 
		{
            sprintf(s,"%c%s%c",'[',*pstr,']');
        } 
		else 
		{
            sprintf(s,"%c%s%c",' ',*pstr,' ');
        }
        
		DrawGbText(s, pControl->x + 20, pControl->y+20+15*(i-startpage));
        sprintf(s,"-%d-",i+1);
        DrawGbText(s, pControl->sx - 25, pControl->y+20+15*(i-startpage));
        
		pstr++;
    }

	EnableScreenFlush(TRUE);
	FlushRect(pControl->x, pControl->y, pControl->sx, pControl->sy);
}


