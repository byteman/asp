#include "dog_init.h"
#include "driver.h"
#include "dt3102_io.h"
//DOG设备控制结构体
typedef struct
{
	GPIO_TypeDef*		Port;							//引脚端口号
	uint16_t				Pin;								//引脚号
}DOGCTL_STRUCT;

//static DOGCTL_STRUCT Dog_Real = {GPIOD,GPIO_Pin_15};			//操作引脚实体				必须static
static u8 DOG_pDisc[] = "LuoHuaiXiang_DOG\r\n";						//设备描述字符	
#define WATCHDOG_PIN	GPIO_Pin_15
#define WATCHDOG_PORT	GPIOD
/****************************************************************************
* 名	称：void FeedDog(void)
* 功	能：喂狗动作
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void FeedDog(void)
{
	IoHi(WATCHDOG);
	IoLo(WATCHDOG);
	//GPIO_SetBits(Dog_Real.Port, Dog_Real.Pin);				//引脚置1
	//GPIO_ResetBits(Dog_Real.Port, Dog_Real.Pin);			//引脚置0
}

/****************************************************************************
* 名	称：static s8 Dog_Open(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 Dog_Open(u32 lParam)
{
	return 0;																							//打开成功
}

/****************************************************************************
* 名	称：static s8 Dog_Close(u32 lParam)
* 功	能：统一接口
* 入口参数：u32 lParam		参数无用
* 出口参数：u8						打开操作错误值	查HVL_ERR_CODE表
* 说	明：无
****************************************************************************/
static s8 Dog_Close(u32 lParam)
{
	return 0;
}

/****************************************************************************
* 名	称：static s32 Dog_Read(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：读
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：无
****************************************************************************/
static s32 Dog_Read(u8* buffer,u32 len) 
{
	return 0;						
}

/****************************************************************************
* 名	称：static s32 Dog_Write(u8* buffer,u32 len,u32* Reallen,u32 lParam) 
* 功	能：立即发音
* 入口参数：u8* buffer		接收缓冲区			无用
						u32 len				希望接收的长度	无用
						u32* Reallen	实际接收的长度	无用
						u32 lParam		参数						无用
* 出口参数：s32						操作是否成功		0成功	1失败
* 说	明：
****************************************************************************/
static s32 Dog_Write(u8* buffer,u32 len) 
{
	return 0;
}

/****************************************************************************
* 名	称：static s32 Dog_Ioctl(u32 cmd, u32 lParam)
* 功	能：串口控制
* 入口参数：u32 cmd				命令
						u32 lParam		参数
* 出口参数：s32						依据命令的不同返回的值含义不同
* 说	明：	无
****************************************************************************/
static s32 Dog_Ioctl(u32 cmd, u32 lParam)
{
	switch(cmd)
	{
		case DOGCMD_FEED:
		{
			FeedDog();			//喂狗
			return 1;
		}
		default:
			return 0;
	}
}

/****************************************************************************
* 名	称：u8 Dog_Init(void)
* 功	能：看门狗初始化
* 入口参数：无
* 出口参数：u8	初始化是否成功	1成功 0失败
* 说	明：无
****************************************************************************/
u8 Dog_Init(void)
{
//	GPIO_InitTypeDef GPIO_InitStructure;		//串口引脚结构
	
		//注册用变量
	DEV_REG dog = 				//设备注册信息表						不必static		
	{
		CHAR_DOG,						//设备ID号
		0,  								//是否共享			0:不可共享使用, 1:可共享使用
		0, 									//对于共享设备最大打开次数
		1,									//最大读次数
		1,									//最大写次数
		DOG_pDisc,					//驱动描述			自定义
		20120001,						//驱动版本			自定义
		(u32*)Dog_Open,		//设备打开函数指针
		(u32*)Dog_Close, 	//设备关闭函数指针
		(u32*)Dog_Read,		//字符读函数
		(u32*)Dog_Write,	//字符写函数
		(u32*)Dog_Ioctl		//控制函数
	};
	

	
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
#if 0
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_15;				//控制引脚
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			//设置引脚为输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//复用推挽输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;		//频率50MHz
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP; 			//不作内部上下拉
	GPIO_Init(Dog_Real.Port, &GPIO_InitStructure);
#endif
	IoSetMode(WATCHDOG,IO_MODE_OUTPUT);
	IoSetSpeed(WATCHDOG,IO_SPEED_50M);
	IoPushPull(WATCHDOG);
	if(DeviceInstall(&dog) != HVL_NO_ERR)								//注册设备
		return 0;	
	
	return 1;
}
