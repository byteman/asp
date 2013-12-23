#ifndef	_KEY_INIT_H_
#define	_KEY_INIT_H_
#include <stdbool.h>
#include "sys_cpu.h"

#define 	KEYCMD_SETONOFF										0		//设备开关设置
#define 	KEYCMD_LookONOFF									1		//设备开关获取
#define		KEYCMD_LookLCount									2		//查询矩阵列数
#define		KEYCMD_LookHCount									3		//查询矩阵行数
#define		KEYCMD_LookDevMode									4		//查询设备工作模式	1中断	0一直扫描
#define		KEYCMD_LookEventFlag								5		//查询中断模式下是否启用信号量发送	1启用	0没启用

typedef enum
{
	KEY_ERR = 0,         							//错误按键 
	KEY_0,											//0
	KEY_1,											//1
	KEY_2,											//2
	KEY_3,											//3
	KEY_4,											//4
	KEY_5,											//5
	KEY_6,											//6
	KEY_7,											//7
	KEY_8,											//8
	KEY_9,											//9
	KEY_Del,										//删除
	KEY_UNIT,										//单位
	KEY_OK,											//确定
	KEY_QUERY,										//查询
	KEY_ADD,										//+-
	KEY_DOT,										//.
	KEY_PRINT,										//打印
	KEY_UP,											//上
	KEY_LEFT,										//左
	KEY_RIGHT,										//右
	KEY_MENU,										//菜单
	KEY_BACK,										//返回
	KEY_DOWN,										//下
	KEY_CAR,										//整车
	KEY_AXIS,										//轴重
	KEY_DIAGNOSIS,									//诊断
	KEY_ZERO,										//置零
}VKEY_VAL; 									//按键值

/****************************************************************************
* 名	称：Init_Matrix_Key()
* 功	能：初始化矩阵键盘
* 入口参数：
* 出口参数：bool	TRUE:	成功		
					FALSE:	失败
* 说	明：无
****************************************************************************/
extern BOOL Key_Config(void);


#endif


