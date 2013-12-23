#include "power_init.h"			
#include "static_init.h"		//Delay_MS

#define DELAYCNT  50			//检测时长，1个延时1MS

void Power_init(void)
{
	GPIO_InitTypeDef P; 
  int i;						 //关机按键检测次数
    
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE);

	P.GPIO_Pin = GPIO_Pin_3;			   	//开关线
  P.GPIO_Mode  = GPIO_Mode_OUT;			//输出
  P.GPIO_Speed = GPIO_Speed_25MHz;	//引脚频率
  P.GPIO_OType = GPIO_OType_PP;			//推挽输出
  P.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	//不作上下拉
	GPIO_Init(GPIOE, &P);
	
  Power_Close;						   				//初始化为关机

	P.GPIO_Pin = GPIO_Pin_4;			  	//检测线
  P.GPIO_Mode  = GPIO_Mode_IN;			//输入
  P.GPIO_Speed = GPIO_Speed_25MHz;	//引脚频率
  P.GPIO_OType = GPIO_OType_OD;			//开漏输出
  P.GPIO_PuPd  = GPIO_PuPd_NOPULL; 	//不作上下拉
	GPIO_Init(GPIOE, &P);

  for(i = 0;i < DELAYCNT;i++) 			  //按键检测50次
	{
		if(PowerLine_Read != 0) 		  //按键按下，检测次数增加
			break;						  //按键松开，检测次数停止，退出次数检测
		Delay_MS(1);					  //检测间隔时间
	}

	if(i >= DELAYCNT) 					  //按键检测次数达到检测条件
		Power_Open;		//电源开
	else 								  //按键检测次数没达到检测条件
	{
		Power_Close;	//依旧关机
		while(1);
	}
}


