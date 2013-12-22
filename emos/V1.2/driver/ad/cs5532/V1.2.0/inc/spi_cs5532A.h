/**
  ******************************************************************************
  * @file    spi_flash.h
  * @author  MCD Application Team
  * @version V1.0.1
  * @date    13-April-2012
  * @brief   This file contains all the functions prototypes for the spi_flash
  *          firmware driver.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2012 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_CS5532A_H
#define __SPI_CS5532A_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "C2.h"
#include "ADInterface.h"
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define CS5532A1    0x01
#define CS5532A2    0x02
	 
/* High layer functions  */
void CS5532A_Init(void);
	 	 
u8 CS5532A1_Flag(void);
u8 CS5532A2_Flag(void);
int32 CS5532Ax_GetValue(void);//为提高执行效率，该函数需要和	 CS5532B1_Flag或者CS5532B2_Flag函数配合使用

//新增
u8 fStepConfgAD_Ax( u8 u8Channel , u8 Step);

#ifdef __cplusplus
}
#endif

#endif /* __SPI_FLASH_H */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
