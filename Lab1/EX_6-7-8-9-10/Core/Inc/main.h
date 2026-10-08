/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define RED_1_Pin GPIO_PIN_4
#define RED_1_GPIO_Port GPIOA
#define RED_2_Pin GPIO_PIN_5
#define RED_2_GPIO_Port GPIOA
#define RED_3_Pin GPIO_PIN_6
#define RED_3_GPIO_Port GPIOA
#define RED_4_Pin GPIO_PIN_7
#define RED_4_GPIO_Port GPIOA
#define RED_5_Pin GPIO_PIN_8
#define RED_5_GPIO_Port GPIOA
#define RED_6_Pin GPIO_PIN_9
#define RED_6_GPIO_Port GPIOA
#define RED_7_Pin GPIO_PIN_10
#define RED_7_GPIO_Port GPIOA
#define RED_8_Pin GPIO_PIN_11
#define RED_8_GPIO_Port GPIOA
#define RED_9_Pin GPIO_PIN_12
#define RED_9_GPIO_Port GPIOA
#define RED_10_Pin GPIO_PIN_13
#define RED_10_GPIO_Port GPIOA
#define RED_11_Pin GPIO_PIN_14
#define RED_11_GPIO_Port GPIOA
#define RED_12_Pin GPIO_PIN_15
#define RED_12_GPIO_Port GPIOA
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
