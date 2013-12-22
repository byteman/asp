#include "Keyboard.h"
#include "Key_cfg.h"
#include "stm32f4xx.h"
#include "c2.h"
#include "driver.h"
#include "dt3102_IO.h"
#include "IO.h"

#define MATRIX_KEY_ROW1SET		IoHi(KEY_H1)//KEY_H1_PORT->BSRRL = KEY_H1_PIN;
#define MATRIX_KEY_ROW2SET		IoHi(KEY_H2)//KEY_H2_PORT->BSRRL = KEY_H2_PIN;
#define MATRIX_KEY_ROW3SET		IoHi(KEY_H3)//KEY_H3_PORT->BSRRL = KEY_H3_PIN;
#define MATRIX_KEY_ROW4SET		IoHi(KEY_H4)//KEY_H4_PORT->BSRRL = KEY_H4_PIN;
// #define MATRIX_KEY_ROW5SET		IoHi(KEY_H5)//KEY_H5_PORT->BSRRL = KEY_H5_PIN;
// #define MATRIX_KEY_ROW6SET		IoHi(KEY_H6)
#define	MATRIX_KEY_ROW1RESET	IoLo(KEY_H1)//KEY_H1_PORT->BSRRH = KEY_H1_PIN;
#define	MATRIX_KEY_ROW2RESET	IoLo(KEY_H2)//KEY_H2_PORT->BSRRH = KEY_H2_PIN;
#define	MATRIX_KEY_ROW3RESET	IoLo(KEY_H3)//KEY_H3_PORT->BSRRH = KEY_H3_PIN;
#define	MATRIX_KEY_ROW4RESET	IoLo(KEY_H4)//KEY_H4_PORT->BSRRH = KEY_H4_PIN;
// #define	MATRIX_KEY_ROW5RESET	IoLo(KEY_H5)//KEY_H5_PORT->BSRRH = KEY_H5_PIN;
// #define	MATRIX_KEY_ROW6RESET 	IoLo(KEY_H6)


#define	KEY_GPIO_STATE_GET(port,pin)	( ((port->IDR & pin)>0) ? 1:0)
#define	EXTI_GET_IT_STATUS(x)			( ((EXTI->PR & x) != (u32)RESET)? SET : RESET)
static const u8 Key_pDisc[] = "Key\r\n";				//设备描述字符
static SYS_EVENT* KeyEvent = (SYS_EVENT*)0;
extern u16 KeyPin;	
extern GPIO_TypeDef* KeyPort;

static u32 Matrix_Key_IntEnable(BOOL bEn);
static const char KeyTable[MATRIX_KEY_LINE][MATRIX_KEY_ROW] = {
//		A				B			C			D				E			
/*1*/	KEY_1,			KEY_2,			KEY_3,		KEY_LEFT,		KEY_BACK,				KEY_F1,			KEY_MENU,				
/*2*/	KEY_4,			KEY_5,			KEY_6,		KEY_UP,			KEY_DIAGNOSIS,	KEY_F2,			KEY_OK,
/*3*/	KEY_7,			KEY_8,			KEY_9,		KEY_RIGHT,	KEY_ZERO,	    	KEY_F3,			KEY_ERR,
/*4*/	KEY_Del,   	KEY_0,			KEY_DOT,	KEY_DOWN,		KEY_PRINT,			KEY_F4,			KEY_ERR	
/*6*/				
};
//矩阵键盘中断注册信息表
static SDEV_IRQ_REGS Key_IRQ = {
	0,								//中断线号
	0,								//中断线开启标志
	(pIRQHandle)0,					//中断句柄
};

/****************************************************************************
* 名	称：void KEY_EXTI15_IRQHandler(void)
* 功	能：中断执行函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static void KEY_IRQHandler(void)
{
	if(EXTI_GET_IT_STATUS(KEY_L1_PIN) != RESET){			    //1列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L1_PIN;						//中断得到列值
			KeyPort = KEY_L1_PORT;				
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L1_PIN;							//清中断标志
	}
	else if(EXTI_GET_IT_STATUS(KEY_L2_PIN) != RESET){			//2列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L2_PIN;						//中断得到列值
			KeyPort = KEY_L2_PORT;
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L2_PIN;							//清中断标志
	}
	else if(EXTI_GET_IT_STATUS(KEY_L3_PIN) != RESET){			//3列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L3_PIN;						//中断得到列值
			KeyPort = KEY_L3_PORT;
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L3_PIN;							//清中断标志
	}
	else if(EXTI_GET_IT_STATUS(KEY_L4_PIN) != RESET){			//4列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L4_PIN;						//中断得到列值
			KeyPort = KEY_L4_PORT;
			if(KeyEvent != 0){							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量				
			}
		}
		EXTI->PR |= KEY_L4_PIN;							//清中断标志
	}
	else if(EXTI_GET_IT_STATUS(KEY_L5_PIN) != RESET){			//5列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L5_PIN;						//中断得到列值
			KeyPort = KEY_L5_PORT;
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L5_PIN;							//清中断标志
	}
		else if(EXTI_GET_IT_STATUS(KEY_L6_PIN) != RESET){			//6列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L6_PIN;						//中断得到列值
			KeyPort = KEY_L6_PORT;
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L6_PIN;							//清中断标志
	}
		else if(EXTI_GET_IT_STATUS(KEY_L7_PIN) != RESET){			//7列中断线触发
		if(KeyPin == 0){ 								//如果前期没有获取按键列值
			KeyPin = KEY_L7_PIN;						//中断得到列值
			KeyPort = KEY_L7_PORT;
			if(KeyEvent != 0)							//信号量已创建
				SysSemPost(KeyEvent);					//发送信号量
		}
		EXTI->PR |= KEY_L7_PIN;							//清中断标志
	}
}


/*
*********************************************************************************************************
*	函 数 名: bsp_InitKeyHard
*	功能说明: 配置按键对应的GPIO
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/

void bsp_InitKeyHard(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE );
	
	
	//IO配置
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//5列
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;						//输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	 					//上拉输入 
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | 
									GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_14 | GPIO_Pin_15;		
	GPIO_Init(GPIOE, &GPIO_InitStructure);	
	
	
	//6行
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;	 					//输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;						//推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					//50MHZS 
	
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | 
									GPIO_Pin_13 ;	
	GPIO_Init(GPIOE, &GPIO_InitStructure);		
}


/****************************************************************************
* 名	称：Init_Matrix_Key_Pin()
* 功	能：初始化矩阵键盘管脚
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
static void Init_Matrix_Key_Pin(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//时钟使能
	RCC_AHB1PeriphClockCmd(KEY_H1_RCC, ENABLE );		
// 	RCC_AHB1PeriphClockCmd(KEY_H2_RCC, ENABLE );		
// 	RCC_AHB1PeriphClockCmd(KEY_H3_RCC, ENABLE );		
// 	RCC_AHB1PeriphClockCmd(KEY_H4_RCC, ENABLE );		
// 	RCC_AHB1PeriphClockCmd(KEY_H5_RCC, ENABLE );		
// 	RCC_AHB1PeriphClockCmd(KEY_L1_RCC, ENABLE );	
// 	RCC_AHB1PeriphClockCmd(KEY_L2_RCC, ENABLE );
// 	RCC_AHB1PeriphClockCmd(KEY_L3_RCC, ENABLE );	
// 	RCC_AHB1PeriphClockCmd(KEY_L4_RCC, ENABLE );
// 	RCC_AHB1PeriphClockCmd(KEY_L5_RCC, ENABLE );	
// 	RCC_AHB1PeriphClockCmd(KEY_L6_RCC, ENABLE );
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);		
	
	//IO配置
  	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	//5列
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;						//输入模式
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;	 					//上拉输入 
	
	GPIO_InitStructure.GPIO_Pin   = KEY_L1_PIN;		
	GPIO_Init(KEY_L1_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_L2_PIN;		
	GPIO_Init(KEY_L2_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_L3_PIN;		
	GPIO_Init(KEY_L3_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_L4_PIN;		
	GPIO_Init(KEY_L4_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_L5_PIN;		
	GPIO_Init(KEY_L5_PORT, &GPIO_InitStructure);		
	GPIO_InitStructure.GPIO_Pin   = KEY_L6_PIN;		
	GPIO_Init(KEY_L6_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_L7_PIN;		
	GPIO_Init(KEY_L7_PORT, &GPIO_InitStructure);			
	//6行
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;	 					//输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;						//推挽
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;					//50MHZS 
	
	GPIO_InitStructure.GPIO_Pin   = KEY_H1_PIN;		
	GPIO_Init(KEY_H1_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_H2_PIN;		
	GPIO_Init(KEY_H2_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_H3_PIN;		
	GPIO_Init(KEY_H3_PORT, &GPIO_InitStructure);	
	GPIO_InitStructure.GPIO_Pin   = KEY_H4_PIN;		
	GPIO_Init(KEY_H4_PORT, &GPIO_InitStructure);	
// 	GPIO_InitStructure.GPIO_Pin   = KEY_H5_PIN;		
// 	GPIO_Init(KEY_H5_PORT, &GPIO_InitStructure);	
// 	GPIO_InitStructure.GPIO_Pin   = KEY_H6_PIN;		
// 	GPIO_Init(KEY_H6_PORT, &GPIO_InitStructure);	
}
/****************************************************************************
* 名	称：Matrix_Key_IntEnable()
* 功	能：中断线配置
* 入口参数：BOOL	TRUE-使能	;	FALSE-失能
* 出口参数：
* 说	明：无
****************************************************************************/
static u32 Matrix_Key_IntEnable(BOOL bEn)
{
	EXTI_InitTypeDef EXTI_InitStructure; 
	//外部中断配置
    EXTI_InitStructure.EXTI_Line = KEY_L1_PIN | KEY_L2_PIN | KEY_L3_PIN | KEY_L4_PIN | KEY_L5_PIN | KEY_L6_PIN | KEY_L7_PIN ;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
    if (TRUE==bEn) {
    	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
    } else {
    	EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
    }
	EXTI_Init(&EXTI_InitStructure);	
	
	return EXTI_InitStructure.EXTI_Line;
}
/****************************************************************************
* 名	称：Init_Matrix_Key_IRQ()
* 功	能：初始化矩阵键盘中断
* 入口参数：
* 出口参数：
* 说	明：无
****************************************************************************/
static BOOL Init_Matrix_Key_IRQ(void)
{
//	EXTI_InitTypeDef EXTI_InitStructure; 
	NVIC_InitTypeDef NVIC_InitStructure;
	//中断管脚映射
	SYSCFG_EXTILineConfig(KEY_L1_EXTI_PORT, KEY_L1_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L2_EXTI_PORT, KEY_L2_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L3_EXTI_PORT, KEY_L3_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L4_EXTI_PORT, KEY_L4_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L5_EXTI_PORT, KEY_L5_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L6_EXTI_PORT, KEY_L6_EXTI_PIN);	
	SYSCFG_EXTILineConfig(KEY_L7_EXTI_PORT, KEY_L7_EXTI_PIN);	

	//SYSCFG_EXTILineConfig(KEY_L6_EXTI_PORT, KEY_L6_EXTI_PIN);
	//中断线配置
/*    EXTI_InitStructure.EXTI_Line = */Matrix_Key_IntEnable(TRUE);
	//中断服务函数配置
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	Key_IRQ.EXTI_Flag = 1;													//中断标志	
	{
		NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
		NVIC_Init(&NVIC_InitStructure);
		Key_IRQ.EXTI_No = 5;												//中断线号??
		Key_IRQ.IRQHandler = KEY_IRQHandler;								//中断函数		
		if(EXTIIRQHandlerInstall(&Key_IRQ) != HVL_NO_ERR)					//中断注册
			return FALSE;		//失败
	}	
	Key_IRQ.EXTI_Flag = 1;													//中断标志	
	{
		NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
		NVIC_Init(&NVIC_InitStructure);
		Key_IRQ.EXTI_No = 10;												//中断线号??
		Key_IRQ.IRQHandler = KEY_IRQHandler;								//中断函数		
		if(EXTIIRQHandlerInstall(&Key_IRQ) != HVL_NO_ERR)					//中断注册
			return FALSE;		//失败
	}		
	return TRUE;
}


 u32 Key_swap(void)
{
	u32 RowKey;
	u32 key ;
	
	key = GPIO_ReadInputData(GPIOE);
	SysTimeDly(20);
	
	GPIO_SetBits(GPIOE, GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 );
	GPIO_ResetBits(GPIOE, GPIO_Pin_10);
	RowKey = GPIO_ReadInputData(GPIOE)&0xc3E0;
	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 );
	
	if (RowKey != 0xc3E0) {
		
		return (RowKey>>5) + 0x3E00;
	}
	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_12 | GPIO_Pin_13 );
	GPIO_ResetBits(GPIOE, GPIO_Pin_11);	
	RowKey = GPIO_ReadInputData(GPIOE)&0xc3E0;
	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 );
	if (RowKey != 0xc3E0) {
		
		return (RowKey>>5) + 0x3D00;
	}	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_13 );
	GPIO_ResetBits(GPIOE, GPIO_Pin_12);		
	RowKey = GPIO_ReadInputData(GPIOE)&0xc3E0;
	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 );
	if (RowKey != 0xc3E0) {
		
		return (RowKey>>5) + 0x3B00;
	}	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12  );
	GPIO_ResetBits(GPIOE, GPIO_Pin_13);	
	RowKey = GPIO_ReadInputData(GPIOE)&0xc3E0;
	
	GPIO_SetBits(GPIOE, GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 );
	if (RowKey != 0xc3E0) {
		
		return (RowKey>>5) + 0x3700;
	}	
	
	
	return 0x3F00 + 0x1F;
}


/****************************************************************************
* 名	称：u8 KeyScanCheckL(void)
* 功	能：矩阵键盘列检查	查询模式用
* 入口参数：无
* 出口参数：u8 引脚列号			非255找到				255没找到
* 说	明：扫描电平方式，第一次按键获得按键值，第二次为确认按键值
****************************************************************************/
static u8 KeyScanCheckL(void)
{
	if(0 == KEY_GPIO_STATE_GET(KeyPort, KeyPin)){
		switch(KeyPin){
			
			case KEY_L6_PIN:return 0;
			case KEY_L7_PIN:return 1;			
			case KEY_L1_PIN:return 2;
			case KEY_L2_PIN:return 3;
			case KEY_L3_PIN:return 4;
			case KEY_L4_PIN:return 5;
			case KEY_L5_PIN:return 6;

			//case KEY_L6_PIN:return 5;
			default :break;
		}
	}
	return  255;												//没找到引脚值
}
/****************************************************************************
* 名	称：u8 KeyCheckH(void)
* 功	能：矩阵键盘行检查
* 入口参数：无
* 出口参数：行号
* 说	明：扫描得到行号
****************************************************************************/	
static u8 KeyCheckH(void)	
{
	int a = 0;
	//逐行将引脚拉高
	MATRIX_KEY_ROW1SET;			//1行拉高
	SysTimeDly(6);
	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
		a = 0;
		return 0;
	}	
	MATRIX_KEY_ROW2SET;			//2行拉高
	SysTimeDly(6);
	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
				a = 1;
		return 1;
	}	
	MATRIX_KEY_ROW3SET;			//3行拉高
	SysTimeDly(6);
	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
				a = 2;
		return 2;
	}	
	MATRIX_KEY_ROW4SET;			//4行拉高
	SysTimeDly(6);
	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
				a = 3;
		return 3;
	}	
// 	MATRIX_KEY_ROW5SET;			//5行拉高
// 	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
// 		return 4;
// 	}
// 	MATRIX_KEY_ROW6SET;			//6行拉高
// 	if(KEY_GPIO_STATE_GET(KeyPort, KeyPin) == 1){//检测前期的列号引脚是否为低 如果这次也被拉低了 则按键存在于该行
// 		return 5;
// 	}	
	return 255;
}
/****************************************************************************
* 名	称：u8 KeyCheck(u8* K_H,u8* K_L)
* 功	能：矩阵键盘行列检查
* 入口参数：u8* K_H	得到的行值
			u8* K_L	得到的列值
* 出口参数：BOOL 是否取得按键值	false没有取得	true取得
* 说	明：得到矩阵键盘的行列值
			未使用中断
****************************************************************************/	
BOOL KeyCheck(u8* K_H,u8* K_L)
{

	if( (*K_L = KeyScanCheckL()) >= MATRIX_KEY_ROW){		//找列号 如果列值不正确
		return FALSE;
	}	
	SysTimeDly(4);											//20毫秒延时

 	if(*K_L != KeyScanCheckL()){	 						//如果没有检测到列值，按键按下的持续性不够
 		return FALSE;
	}
	*K_H = KeyCheckH(); 									//得到行值
	//行复位
	
	SysTimeDly(4);
	MATRIX_KEY_ROW1RESET;			
	MATRIX_KEY_ROW2RESET;			
	MATRIX_KEY_ROW3RESET;			
	MATRIX_KEY_ROW4RESET;		
	SysTimeDly(4);
// 	MATRIX_KEY_ROW5RESET;	
// 	MATRIX_KEY_ROW6RESET;
	
	if( *K_H >= MATRIX_KEY_LINE ){							//行值检测
		return FALSE;
	}
	return TRUE;											//行列值不在范围内或检测到按键按下
}
/****************************************************************************
* 名	称：static s8 KEY_Open(u32 lParam)
* 功	能：设备打开
* 入口参数：u32 lParam		设备配置参数
* 出口参数：s8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 KEY_Open(INT32U lParam)
{
	Matrix_Key_IntEnable(TRUE);
	return HVL_NO_ERR;					//打开设备成功
}

/****************************************************************************
* 名	称：static s8 KEY_Close(u32 lParam)
* 功	能：设备关闭
* 入口参数：u32 lParam		设备配置参数
* 出口参数：s8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：
****************************************************************************/
static s8 KEY_Close(u32 lParam)
{
	Matrix_Key_IntEnable(FALSE);
	return HVL_NO_ERR;					//关闭设备成功
}
/****************************************************************************
* 名	称：static s32 BEEP_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		无用
						u32 len				无用
						u32* Reallen	无用
						u32 lParam		参数
* 出口参数：s32						按键值	0没有	非0即按键值
* 说	明：无
****************************************************************************/
static s32 KEY_Read(u8* buffer,u32 len) 
{
	s8	s8KeyValue;
	u8	u8KeyLine,u8KeyRow,err;
	
	SysSemPend(KeyEvent,0,&err);			//等待中断服务函数发送的信号量
 	if(err != SYS_NO_ERR)					
 		return -1;		
	Matrix_Key_IntEnable(FALSE);				//关闭中断
	if(KeyCheck(&u8KeyLine,&u8KeyRow) == FALSE)	//检测到按键按下 
		{s8KeyValue = -2;}						//错误按键值
		else{s8KeyValue = (VKEY_VAL)KeyTable[u8KeyLine][u8KeyRow];}
	KeyPin = 0;	
	Matrix_Key_IntEnable(TRUE);					//打开中断
	return s8KeyValue;
}

/****************************************************************************
* 名	称：static s32 KEY_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：写
* 入口参数：u8* buffer		数据源缓冲区					无用
						u32 len				希望写入的长度				无用
						u32* Reallen	实际写入的长度				无用
						u32 lParam		参数									无用
* 出口参数：s32						操作是否成功		0成功	1失败			无用
* 说	明：无
****************************************************************************/
static s32 KEY_Write(u8* buffer,u32 len) 
{
	return 0;
}
/****************************************************************************
* 名	称：static INT32S KEY_CHARIoctl(INT32U cmd, INT32U lParam)
* 功	能：设备控制
* 入口参数：INT32U cmd				命令
						INT32U lParam			参数
* 出口参数：INT32S						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 KEY_CHARIoctl(s32 cmd, s32 lParam)
{
	switch(cmd){
		case KEYCMD_SETONOFF:{											//设置开关
			Matrix_Key_IntEnable((BOOL)lParam);		
			return 1;								
		}
		case KEYCMD_LookONOFF:											//查询开关
			return ( *((volatile uint32_t*)EXTI_BASE)&&KEY_L1_PIN );	//返回第一列中断的开关状态
		case (u32)KEYCMD_LookLCount:									//查询矩阵列数
			return MATRIX_KEY_ROW;	
		case (u32)KEYCMD_LookHCount:									//查询矩阵行数
			return MATRIX_KEY_LINE;	
		case (u32)KEYCMD_LookDevMode:									//查询设备工作模式
			return 1;	
		case (u32)KEYCMD_LookEventFlag:									//查询中断模式下信号量是否启用
			return 1;	
		default:break;
	}
	return 1;	
}
/****************************************************************************
* 名	称：Key_Config()
* 功	能：初始化矩阵键盘
* 入口参数：
* 出口参数：BOOL	TRUE:	成功		
					FALSE:	失败
* 说	明：无
****************************************************************************/
BOOL Key_Config(void)
{
	DEV_REG Dev_KEY ={
		CHAR_KEY,						//字符设备号		自DEV_CHAR_ID取匹配设备ID
		0,								//是否共享			0:不可共享使用, 1:可共享使用
		0,								//对于共享设备最大打开次数
		1,								//最大读次数
		1,								//最大写次数
		(u8*)Key_pDisc,					//驱动描述			自定义
		20130001,						//驱动版本			自定义
		(u32*)KEY_Open,					//设备打开函数指针
		(u32*)KEY_Close,				//设备关闭函数指针
		(u32*)KEY_Read,					//设备读函数映射指针
		(u32*)KEY_Write,				//设备写函数映射指针
		(u32*)KEY_CHARIoctl				//设备控制函数映射指针
	};
	//注册
	if(DeviceInstall(&Dev_KEY) != HVL_NO_ERR){
		return FALSE;	
	}
	Init_Matrix_Key_Pin();				//管脚初始化
	if(FALSE==Init_Matrix_Key_IRQ()){	//中断初始化
		return FALSE;
	}		
	if(KeyEvent == (SYS_EVENT*)0){	
		KeyEvent = SysSemCreate(0); 		//创建按键信号量
	}
	return TRUE;
}

void Key_Inif_KeyHard(void)
{
		Init_Matrix_Key_Pin();				//管脚初始化
}

VKEY_VAL Key_Get_KeyValue(u8 line, u8 row)
{
		s8	s8KeyValue;
		s8KeyValue = (VKEY_VAL)KeyTable[line][row];
	return s8KeyValue;
}

