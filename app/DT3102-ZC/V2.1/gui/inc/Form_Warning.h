#ifndef __FORM_WARNING_H_
#define __FORM_WARNING_H_
#include "../Inc/GuiWindows.h"

typedef struct
{
    uint8  uAD;		//传感器
    uint8  uPT;		//打印机
    uint8  uIC;		//射频模块
	uint8 uCS5532;	//转换芯片
} SWarningFlag;

extern	CWindow		gWD_Warning;		//警告查询界面

#endif


