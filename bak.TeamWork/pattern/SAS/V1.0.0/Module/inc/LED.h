#ifndef _LED_H_	
#define _LED_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

#include "includes.h"
	
//LED管脚定义	
#define LED1_PORT		GPIOF
#define LED1_PIN		GPIO_Pin_11
#define LED2_PORT		GPIOF
#define LED2_PIN		GPIO_Pin_12
#define LED3_PORT		GPIOF
#define LED3_PIN		GPIO_Pin_13
#define LED4_PORT		GPIOC
#define LED4_PIN		GPIO_Pin_13	
#define LED5_PORT		GPIOD
#define LED5_PIN		GPIO_Pin_7
#define LED6_PORT		GPIOD
#define LED6_PIN		GPIO_Pin_11
#define LED7_PORT		GPIOD
#define LED7_PIN		GPIO_Pin_13

	
	

//引脚上下拉
#define LED1_OFF						(LED1_PORT -> BSRRL = LED1_PIN)
#define LED1_ON							(LED1_PORT -> BSRRH = LED1_PIN)	
#define LED2_OFF						(LED2_PORT -> BSRRL = LED2_PIN)
#define LED2_ON							(LED2_PORT -> BSRRH = LED2_PIN)	
#define LED3_OFF						(LED3_PORT -> BSRRL = LED3_PIN)
#define LED3_ON							(LED3_PORT -> BSRRH = LED3_PIN)	
#define LED5_OFF						(LED5_PORT -> BSRRL = LED5_PIN)
#define LED5_ON							(LED5_PORT -> BSRRH = LED5_PIN)	
#define LED6_OFF						(LED6_PORT -> BSRRL = LED6_PIN)
#define LED6_ON							(LED6_PORT -> BSRRH = LED6_PIN)	
#define LED7_OFF						(LED7_PORT -> BSRRL = LED7_PIN)
#define LED7_ON							(LED7_PORT -> BSRRH = LED7_PIN)	
	
#define LED4_ON 						GPIO_ResetBits(LED4_PORT, LED4_PIN)			
#define LED4_OFF 						GPIO_SetBits(LED4_PORT, LED4_PIN)	
#define	ReadLED4State()			(GPIO_ReadOutputDataBit(LED4_PORT,LED4_PIN))
	
	
	
	
//初始化
extern void fLED_IO_Init(void);												//设置

	
	
	
	

#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif
