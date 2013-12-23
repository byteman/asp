#include "Menu.h"
#include "guiwindows.h"

#include "stdio.h"
#include "string.h"
#define OSTimeDly		SysTimeDly



void DefaultMenuWindowDraw(LPWindow pWindow)
{
	uint16 i;
	int16 nStrSize, nStrStart;
	uint8 nPos;//nParantID, 
	char strTemp[36];
	LPMenu	pMenu;
	LPMenuItem pMenuItem;

	pMenu = (LPMenu)(pWindow->pLPControls);

	if(pMenu == NULL) return;

	EnableScreenFlush(FALSE);

	//擦除屏幕，包括缓存
	EraseBuffer();

	//禁止绘图
	SetRedraw(FALSE);

	//设置视图
	SetGdiView(pWindow->nViewPosX, pWindow->nViewPosY, pWindow->nViewSizeX, pWindow->nViewSizeY);
	EnableGdiView(TRUE);

	if(pMenu->strName != NULL)
	{
		DrawGbText(pMenu->strName, 0, 22);	//显示菜单名
	}

	//显示菜单级别
	if(pMenu->pParentMenu != NULL)			
	{
		//nParantID = pMenu->pParentMenu->nCurItem;
	}
	else
	{
		//nParantID = 0;
	}

	//snprintf(strTemp, 50, "%d/%d", pMenu->nCurItem, nParantID);
	snprintf(strTemp, 50, "%d", pMenu->nCurItem);
	DrawGbText(strTemp, 211, 22);	//显示菜单级别
	
	//绘制名称分割线
	DrawHoriLine(0, 38, 240);
	DrawHoriLine(0, 40, 240);

	for(i = 0; (i < pMenu->nNbItems) && (i < MAX_SHOW_MENU_ITEM); i++)
	{
		if(i + pMenu->nFirstItem >= pMenu->nNbItems) break;

		pMenuItem = *(pMenu->pLPMenuItems + i + pMenu->nFirstItem);


		nStrSize = strlen(pMenuItem->strName);

		if(nStrSize == 0) continue;	//菜单无名称，不显示

		if(nStrSize > MAX_MENU_NAME_CHARACTER)
		{
			nStrSize = MAX_MENU_NAME_CHARACTER;
		}

		//计算居中位置
		nStrStart = (MAX_LINE_CHARACTER - nStrSize) / 2;

		memset(strTemp, 0x20, MAX_LINE_CHARACTER);
		memcpy(&strTemp[nStrStart], pMenuItem->strName, nStrSize);
		strTemp[MAX_LINE_CHARACTER] = 0;
		if((i + pMenu->nFirstItem) == pMenu->nCurItem) 
		{
			strTemp[nStrStart-1] = '[';	//【
			strTemp[nStrStart+nStrSize] = ']';	//】
		}
		//显示菜单名
		nPos = i & 1 ? 120 : 0;
		DrawGbText(strTemp, nPos, (i/2+1) * 23+20);
	}

	//开始刷屏
	EnableScreenFlush(TRUE);

	//刷屏
	FlushScreen();

	//开启绘图
	SetRedraw(TRUE);  
}

extern CControl  gStatusBar;
void DefMenuWindowProc(LPWindow pWindow, LPGuiMsgInfo pGuiMsgInfo)
{
	CControl* pControl;	
	GuiMsgInfo guiMsgInfo;
	LPMenu pMenu;
	LPMenuItem pCurItem;

	pMenu = (LPMenu)(pWindow->pLPControls);
	pCurItem = pMenu->pLPMenuItems[pMenu->nCurItem];

	switch(pGuiMsgInfo->ID)
	{
		case WM_LOAD:
			ClearScreen();
			OSTimeDly(15);
            if(pGuiMsgInfo->wParam == 1) {
    			pMenu->nCurItem = 0;
    			pMenu->nCurPos = 0;
    			pMenu->nFirstItem = 0;
            }
			pWindow->DrawFunc(pWindow);
            gStatusBar.DrawFunc(&gStatusBar);
			break;
		
		case WM_SHOW:
			pWindow->DrawFunc(pWindow);
			gStatusBar.DrawFunc(&gStatusBar);
			break;

		case WM_UPDATECTRL:
			pControl = (CControl*)(pGuiMsgInfo->wParam);
			if(pControl != NULL)
			{
				pControl->DrawFunc(pControl);
			} 			
			break;
		case WM_TIMEUPDATE:
			PostWindowMsg(WM_UPDATECTRL, (uint32)&gStatusBar, 0);
			break;

		case WM_KEYDOWN:
			switch(pGuiMsgInfo->wParam)
			{
				case VK_UP:	
					if(pMenu->nNbItems == 0) return;

					if(pMenu->nCurItem - 2 <= 0)
					{
						pMenu->nCurItem = 0;
						pMenu->nCurPos = 0;
						pMenu->nFirstItem = 0;
					}
					else
					{
						pMenu->nCurItem -= 2;
						if(pMenu->nCurPos > 1)
						{
							pMenu->nCurPos -= 2;	
						}
						else
						{
							if(pMenu->nFirstItem > 1)
							{
								pMenu->nFirstItem -= 2;
							}
						}
					}

					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);
					break;
				case VK_DOWN:							
					if(pMenu->nNbItems == 0) return;

					if(pMenu->nCurItem + 2 > pMenu->nNbItems - 1)
					{
						pMenu->nCurItem = pMenu->nNbItems - 1;	//最后一条菜单
						if(pMenu->nNbItems % 2 == 0)
						{
							if(pMenu->nNbItems >= MAX_SHOW_MENU_ITEM)
							{
								pMenu->nFirstItem = pMenu->nNbItems - MAX_SHOW_MENU_ITEM;
							}
							else
							{
								pMenu->nFirstItem = 0;
							}
						}
						else
						{
							if(pMenu->nNbItems >= MAX_SHOW_MENU_ITEM)
							{
								pMenu->nFirstItem = pMenu->nNbItems - MAX_SHOW_MENU_ITEM + 1;
							}
							else
							{
								pMenu->nFirstItem = 0;
							}
						}
						pMenu->nCurPos = pMenu->nNbItems - 1 - pMenu->nFirstItem;
					}
					else
					{
						pMenu->nCurItem += 2;
						if(pMenu->nCurPos < (MAX_SHOW_MENU_ITEM - 2))
						{
							pMenu->nCurPos += 2;	
						}
						else
						{
							pMenu->nFirstItem += 2;
						}
					}

					guiMsgInfo.ID = WM_SHOW;
					GuiMsgQueuePost(&guiMsgInfo);
					break;
				case VK_LEFT:
					if(pMenu->nNbItems == 0) return;

					if(pMenu->nCurItem > 0)
					{
						pMenu->nCurItem--;
						if(pMenu->nCurPos > 0)
						{
							pMenu->nCurPos--;	
						}
						else
						{
							if(pMenu->nFirstItem > 1)
							{
								pMenu->nFirstItem -= 2;
								pMenu->nCurPos = 1;
							}
						}

						guiMsgInfo.ID = WM_SHOW;
						GuiMsgQueuePost(&guiMsgInfo);
					}
					break;
				case VK_RIGHT:
					if(pMenu->nNbItems == 0) return;

					if(pMenu->nCurItem < pMenu->nNbItems - 1)
					{
						pMenu->nCurItem++;
						if(pMenu->nCurPos < 7)
						{
							pMenu->nCurPos++;	
						}
						else
						{
							pMenu->nFirstItem += 2;
							pMenu->nCurPos = 6;
						}

						guiMsgInfo.ID = WM_SHOW;
						GuiMsgQueuePost(&guiMsgInfo);
					}
					break;
				case VK_OK:
					if(pCurItem->pSubMenu != NULL)
					{
						pCurItem->pSubMenu->pParentMenu = pMenu;
						pWindow->pLPControls = (LPControl*)(pCurItem->pSubMenu);
						pCurItem->pSubMenu->nCurItem = 0;
						pCurItem->pSubMenu->nCurPos = 0;
						pCurItem->pSubMenu->nFirstItem = 0;
						pCurItem->pSubMenu->strName = pMenu->pLPMenuItems[pMenu->nCurItem]->strName;
						guiMsgInfo.ID = WM_SHOW;
						GuiMsgQueuePost(&guiMsgInfo);	
					}
					else
					{
						if(pMenu->ProcFunc != NULL)
						{
							pMenu->ProcFunc(pWindow, pCurItem->nCmd);
						}
					}	
					break;
				case VK_RETURN:
					if(pMenu->pParentMenu != NULL)
					{
						pWindow->pLPControls = (LPControl*)(pMenu->pParentMenu);
						guiMsgInfo.ID = WM_LOAD;
						GuiMsgQueuePost(&guiMsgInfo);
					}
					else
					{					
						ClearScreen();						//清屏
						g_pCurWindow = pWindow->pParentWindow;
						guiMsgInfo.ID = WM_LOAD;
						GuiMsgQueuePost(&guiMsgInfo);
					} 
					break;
				default:
					break;
			}
			break;

		case WM_SUBMENU:
			if(pMenu->nNbItems == 0) return;
			if(pGuiMsgInfo->wParam < pMenu->nNbItems)
			{
				pMenu->nCurItem = pGuiMsgInfo->wParam;
				if(pMenu->nCurItem >= MAX_SHOW_MENU_ITEM)
				{
					pMenu->nFirstItem = pMenu->nCurItem - 3;
					pMenu->nCurPos = 3;
				}
				else
				{
					pMenu->nFirstItem = 0;
					pMenu->nCurPos = pMenu->nCurItem;
				}

				guiMsgInfo.ID = WM_KEYDOWN;
				guiMsgInfo.wParam = VK_OK;
				GuiMsgQueuePost(&guiMsgInfo);
			}
			break;

		default:
			break;
	}	
}


