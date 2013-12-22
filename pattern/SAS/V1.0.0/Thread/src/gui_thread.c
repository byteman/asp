#include "includes.h"
#include "Form_Logo.h"
#include "Form_Main.h"
#include "Keyboard.h"
#include "guiwindows.h"

void GUI_Thread(void *arg)
{

	uint8 err;
	GuiMsgInfo guiMsgInfo;		//GUI消息结构实体
	
	//初始化GUI
	BeginPaint();
	
	//设置默认颜色
	SetColor(0);
	
	//清屏
	EraseScreen();	 

	//显示启动画面
    guiMsgInfo.ID = WM_LOAD;
    gWD_Logo.ProcFunc(&gWD_Logo, &guiMsgInfo);
    SysTimeDly(300);		  					
	
	//创建GUI消息队列接收
	GuiMsgQueueCreate();

	//设置主窗口
	g_pCurWindow = &gWD_Main;// &gWD_Main_Axle;				//&g_FlowWindow;	
	
	//向主当前窗口发送加载消息
	guiMsgInfo.pWindow = g_pCurWindow;
	guiMsgInfo.ID = WM_LOAD;
	GuiMsgQueuePost(&guiMsgInfo);

	//消息循环
	while(TRUE)
	{
		GuiMsgQueuePend(&guiMsgInfo, &err);		//等待GUI队列消息
		
		if(err == SYS_NO_ERR) 
		{
			if((g_pCurWindow != NULL) && (g_pCurWindow->ProcFunc != NULL))
			{
				g_pCurWindow->ProcFunc(g_pCurWindow, &guiMsgInfo);
			}
		}  
	}
}
