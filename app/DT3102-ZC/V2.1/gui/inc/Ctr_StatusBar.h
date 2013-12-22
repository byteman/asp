#ifndef __CTR_STATUSBAR_H_
#define __CTR_STATUSBAR_H_


#include "../Inc/GuiWindows.h"

typedef struct _CStatusBar
{
    char    sta_gm;     //0x00:异常     0x01:无车       0x02:有车
	char    sta_dg;     //0x00:异常     0x01:无车       0x02:有车
	char    sta_lz;     //0x00:异常     0x01:单轮       0x02:双轮
    char    voice;      //0x00:close    0x01:open
    char    error;      //0x00:close    0x01:open
    char*   unit;
    char*   time;
}CStatusBar;



//外部调用函数
void EnableVoiceIcon(BOOL b);
void SetGmState(char sta);
void SetDgState(char sta);
void SetLzState(char sta);


void Form_StatusBar_Draw(CControl* pControl);


#define DEF_STATUS_BAR(Name, pParent, x, y, cx, cy, sta_gm, sta_dg, sta_lz, voice, error, unit, time, state)  \
CStatusBar CTRL_CONTENT(Name) = {(sta_gm),(sta_dg),(sta_lz),(voice),(error),(unit),(time)};                     \
CControl Name = {(pParent), (x), (y), (cx), (cy), (state), (0), (&CTRL_CONTENT(Name)), Form_StatusBar_Draw, NULL}


extern CControl gStatusBar;
extern CStatusBar CTRL_CONTENT(gStatusBar);








#endif

