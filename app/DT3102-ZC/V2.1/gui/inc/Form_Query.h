#ifndef __FORM_QUERY_H_
#define __FORM_QUERY_H_
#include "../Inc/GuiWindows.h"
#include "overtime_queue.h"

extern	CWindow		gWD_NormalQuery;		//正常数据查询

extern	CWindow		gWD_DataMenu;   //查询标题

extern	CWindow		gWD_OverloadQuery;		//超载数据查询 

void Get_VehShow(OvertimeData *pData);

#endif


