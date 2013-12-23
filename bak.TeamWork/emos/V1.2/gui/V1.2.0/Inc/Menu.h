#ifndef __MENU_H
#define __MENU_H
#include "includes.h"
#include "GDI.h"
#include "guiwindows.h"

#define MAX_SHOW_MENU_ITEM		8		//最大显示条目数
#define MAX_MENU_NAME_CHARACTER	10		//最大菜单名长度
#define MAX_LINE_CHARACTER		20		//一行能显示的最大字符数

struct _CMenu;

//菜单处理函数
typedef void (*MenuProcTypeDef)(struct _CWindow* pParentWindow, uint32 nCmd);

//定义菜单项
typedef struct _CMenuItem
{
	char* 					strName;				 		//菜单项名称
	uint16 					nbs;					 			//菜单项名称的字符个数
	uint16 					nCmd;				 				//菜单项命令
	struct _CMenu* 	pSubMenu;						//子菜单
} CMenuItem, *LPMenuItem;

//定义菜单
typedef struct _CMenu
{
	char*					strName;					//菜单名
	LPMenuItem*		pLPMenuItems;			//菜单项指针数组
	uint16				nNbItems;					//菜单项个数
	uint16				nCurItem;					//当前被选中的菜单
	uint16				nCurPos;					//当前选中菜单在窗口中的位置
	uint16				nFirstItem;				//窗口中第一个显示出的菜单项

	struct _CMenu*		pParentMenu;		//父菜单
	MenuProcTypeDef		ProcFunc;
	
} CMenu, *LPMenu;

/****************************************************************************
* 名	称：DEF_MENU_ITEM(Name, strName, nCmd, pSubMenu)
* 功	能：定义菜单
* 入口参数：CMenuItem				Name			菜单结构体
						char*						strName		菜单名
						uint16					nCmd			菜单项命令
						struct _CMenu*	pSubMenu	子菜单
* 出口参数：
* 说	明：无
****************************************************************************/
#define DEF_MENU_ITEM(Name, strName, nCmd, pSubMenu)	\
CMenuItem Name = {(strName), 0, nCmd, pSubMenu}					 


#define DEF_MENU(Name, strName, Items, pParentMenu, ProcFunc)	\
CMenu Name = {strName, (Items), sizeof(Items)/sizeof(LPMenu), 0, 0, 0, (pParentMenu), (ProcFunc)}					 

#define DEF_MENU_WND(Name, pMenu, pParentWindow)	\
CWindow Name = {(LPControl*)(pMenu), 0, 0, 0, 0, 240, 128, WS_VISABLE, (pParentWindow), 0, NULL, NULL, DefaultMenuWindowDraw, DefMenuWindowProc}	

//窗口函数
void DefaultMenuWindowDraw(LPWindow pWindow);
void DefMenuWindowProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo);

#endif



