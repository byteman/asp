#ifndef __BOOTKEY_INIT_H
#define __BOOTKEY_INIT_H

#include "stm32f4xx.h"

#define BootKey_SDA_I 	GPIO_ReadInputDataBit(GPIOE, GPIO_Pin_0)	//Òýµ¼¼ü¼ì²â

void BootKey_init(void);

#endif
