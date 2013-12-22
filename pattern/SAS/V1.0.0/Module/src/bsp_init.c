#include "includes.h"
#include "bsp_init.h"			

/****************************************************************************
* 名	称：void Bsp_Init(void)
* 功	能：驱动初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Bsp_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//配置4个输出管脚
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOD|RCC_AHB1Periph_GPIOE,ENABLE);	  										//输入输出引脚时钟
	
	
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;  
	GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;
	GPIO_Init(GPIOE, &GPIO_InitStructure);	

	//配置10个输入
	GPIOF->MODER &= 0xffc00000;
	//配置标定PD10
	GPIOD->MODER &= 0xffcfffff;
}

