#include "beep_init.h"
#include "includes.h"					//INT16U
#include "driver.h"

#define USER_BEEP_Port		GPIOE								//蜂鸣器引脚端口
#define USER_BEEP_Pin			GPIO_Pin_4					//蜂鸣器引脚

//BEEP设备控制结构体
typedef struct
{
	GPIO_TypeDef*		BEEP_Port;							//引脚端口号
	uint16_t				BEEP_Pin;								//引脚号
} BEEPCTL_STRUCT;

static BEEPCTL_STRUCT BEEP_Real = {(GPIO_TypeDef*)0,0};			//蜂鸣器操作引脚实体				必须static
static u8 BeepFlag = 0;										//开启or关闭Beep标志 0关闭		1开启		必须static
static u32 Bcount = 0;										//鸣叫次数
static u32 Btime = 0;											//鸣叫音长

/****************************************************************************
* 名	称：void SetBeepFlag(u8 on)
* 功	能：设置beep发声与否的开关
* 入口参数：u8 on		0 关闭	1 开启
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetBeepFlag(u8 on)
{
	if(on > 1) 
		on = 1;
	BeepFlag = on;		//蜂鸣器开关
}

/****************************************************************************
* 名	称：u8 LookBeepFlag(void)
* 功	能：查看蜂鸣器开关
* 入口参数：无
* 出口参数：u8		0 关闭	1 开启
* 说	明：无
****************************************************************************/
u8 LookBeepFlag(void)
{
	return BeepFlag;		//蜂鸣器开关
}

/****************************************************************************
* 名	称：void Beep(u32 count,u16 time)
* 功	能：蜂鸣器发声
* 入口参数：u16 count 	发声次数		16位计数
						u16 time 		声长10MS		16位计数，最长响65535毫秒，即655.3秒
* 出口参数：无
* 说	明：无
****************************************************************************/
void Beep(u32 count,u16 time)
{
	u32 i;							//累加器
	
	if(BeepFlag != 1)		//不允许鸣叫
	{
			GPIO_SetBits(BEEP_Real.BEEP_Port, BEEP_Real.BEEP_Pin);				//引脚置1
			SysTimeDly(Btime);			//10毫秒级延时
			return;
	}
	else								//鸣叫
	{
		Bcount = count;			//保存次数
		Btime = time;				//保存音长
		
		for(i = 0;i < Bcount;i++)
		{
			GPIO_ResetBits(BEEP_Real.BEEP_Port, BEEP_Real.BEEP_Pin);			//引脚置0
			SysTimeDly(Btime);			//10毫秒级延时
			
			GPIO_SetBits(BEEP_Real.BEEP_Port, BEEP_Real.BEEP_Pin);				//引脚置1
			SysTimeDly(Btime);			//10毫秒级延时
			
		}
	}
}

/****************************************************************************
* 名	称：void Beep_Init(BEEPCTL_STRUCT Beep_t)
* 功	能：Beep模块驱动初始化
* 入口参数：BEEPCTL_STRUCT Beep_t		引脚设置
* 出口参数：无
* 说	明：无
****************************************************************************/
void Beep_Init(BEEPCTL_STRUCT Beep_t)
{
	GPIO_InitTypeDef GPIO_InitStructure;		//串口引脚结构

	BEEP_Real.BEEP_Port = Beep_t.BEEP_Port;		//获取端口号
	BEEP_Real.BEEP_Pin = Beep_t.BEEP_Pin;			//获取引脚号
	
	//根据引脚端口号打开相应的端口时钟
	if(BEEP_Real.BEEP_Port == GPIOA)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOB)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOC)
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOD)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOE)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOF)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOG)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOH)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
	else if(BEEP_Real.BEEP_Port == GPIOI)
			RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI, ENABLE);

	GPIO_InitStructure.GPIO_Pin   = BEEP_Real.BEEP_Pin;	//蜂鸣器控制引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			//设置引脚为输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//频率50MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	//不作内部上下拉
	GPIO_Init(BEEP_Real.BEEP_Port, &GPIO_InitStructure);
}

/****************************************************************************
* 名	称：void Beep_Config(void)
* 功	能：只是为了来封装Beep_init,把引脚固定设置
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Beep_Config(void)
{
	BEEPCTL_STRUCT Beep_t =
	{
		USER_BEEP_Port,		//引脚端口号
		USER_BEEP_Pin			//引脚号
	};

	Beep_Init(Beep_t);
}
