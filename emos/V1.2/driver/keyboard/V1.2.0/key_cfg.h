
#ifndef _KEY_CFG_H_	
#define _KEY_CFG_H_

#ifdef __cplusplus		   //定义对CPP进行C处理 //开始部分
extern "C" {
#endif

//行配置	
#define		KEY_H1_RCC			RCC_AHB1Periph_GPIOE	
#define		KEY_H1_PORT		GPIOE
#define		KEY_H1_PIN			GPIO_Pin_10
	
#define		KEY_H2_RCC			RCC_AHB1Periph_GPIOE	
#define		KEY_H2_PORT		GPIOE
#define		KEY_H2_PIN			GPIO_Pin_11

#define		KEY_H3_RCC			RCC_AHB1Periph_GPIOE	
#define		KEY_H3_PORT		GPIOE
#define		KEY_H3_PIN			GPIO_Pin_12
	
#define		KEY_H4_RCC			RCC_AHB1Periph_GPIOE	
#define		KEY_H4_PORT		GPIOE
#define		KEY_H4_PIN			GPIO_Pin_13
	
	
///////////////////////////////
// #define		KEY_H5_RCC			RCC_AHB1Periph_GPIOE	
// #define		KEY_H5_PORT		GPIOE
// #define		KEY_H5_PIN			GPIO_Pin_14

// #define		KEY_H6_RCC			RCC_AHB1Periph_GPIOE
// #define		KEY_H6_PORT		GPIOE
// #define		KEY_H6_PIN			GPIO_Pin_15

//列配置
#define		KEY_L1_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L1_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L1_EXTI_PIN	EXTI_PinSource5	
#define 		KEY_L1_PORT		GPIOE
#define		KEY_L1_PIN			GPIO_Pin_5

#define		KEY_L2_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L2_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L2_EXTI_PIN	EXTI_PinSource6	
#define 		KEY_L2_PORT		GPIOE
#define		KEY_L2_PIN			GPIO_Pin_6

#define		KEY_L3_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L3_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L3_EXTI_PIN	EXTI_PinSource7	
#define 		KEY_L3_PORT		GPIOE
#define		KEY_L3_PIN			GPIO_Pin_7

#define		KEY_L4_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L4_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L4_EXTI_PIN	EXTI_PinSource8	
#define 		KEY_L4_PORT		GPIOE
#define		KEY_L4_PIN			GPIO_Pin_8

#define		KEY_L5_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L5_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L5_EXTI_PIN	EXTI_PinSource9	
#define 		KEY_L5_PORT		GPIOE
#define		KEY_L5_PIN			GPIO_Pin_9


#define		KEY_L6_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L6_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L6_EXTI_PIN	EXTI_PinSource14	
#define 	KEY_L6_PORT		GPIOE
#define		KEY_L6_PIN			GPIO_Pin_14


#define		KEY_L7_RCC			RCC_AHB1Periph_GPIOE
#define		KEY_L7_EXTI_PORT	EXTI_PortSourceGPIOE
#define		KEY_L7_EXTI_PIN	EXTI_PinSource15	
#define 	KEY_L7_PORT		GPIOE
#define		KEY_L7_PIN			GPIO_Pin_15



#ifdef __cplusplus		   //定义对CPP进行C处理 //结束部分
}
#endif

#endif







