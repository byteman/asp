#include "BootKey_init.h"
#include "ConnectCfg.h"				//BootKeyPin

#ifdef BootKey								//有引导按键
/****************************************************************************
* 名	称：void BootKey_init(void)
* 功	能：引导按键初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void BootKey_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;

  RCC_AHB1PeriphClockCmd(BootKeyRCC, ENABLE);					//打开端口时钟

	//检测线 Key一引脚线
	GPIO_InitStructure.GPIO_Pin = 	BootKeyPin;					//检测线
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN;				//输入
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;		//频率
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;			//开漏
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  	//不作上下拉
	GPIO_Init(BootKeyPort, &GPIO_InitStructure);
    
	//拉低电平	Key另一引脚线
	GPIO_InitStructure.GPIO_Pin = 	GPIO_Pin_9;					//电平线
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;			//输出
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;		//频率
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;			//推挽
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  	//不作上下拉
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	BootKeyDownVal == Bit_RESET ? GPIO_SetBits(GPIOE,GPIO_Pin_9) : GPIO_ResetBits(GPIOE,GPIO_Pin_9);	//按下为低电平
}
#endif
