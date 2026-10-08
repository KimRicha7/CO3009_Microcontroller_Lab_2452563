/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
/* USER CODE BEGIN PFP */
void clearAllClock(void);
void setNumberOnClock(int num);
void clearNumberOnClock(int num);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* One entry per LED, in the same order as your labels: RED_1 through RED_12 */
const uint16_t RED_PINS[12] = {
    RED_1_Pin, RED_2_Pin, RED_3_Pin, RED_4_Pin,
    RED_5_Pin, RED_6_Pin, RED_7_Pin, RED_8_Pin,
    RED_9_Pin, RED_10_Pin, RED_11_Pin, RED_12_Pin
};

/* Indexed by TRUE clock position: [0]=12 o'clock, [1]=1 o'clock, ... [11]=11 o'clock */
const uint16_t CLOCK_PINS[12] = {
    RED_2_Pin,   /* 12 */
    RED_3_Pin,   /* 1  */
    RED_10_Pin,  /* 2  */
    RED_11_Pin,  /* 3  */
    RED_12_Pin,  /* 4  */
    RED_9_Pin,   /* 5  */
    RED_8_Pin,   /* 6  */
    RED_7_Pin,   /* 7  */
    RED_6_Pin,   /* 8  */
    RED_5_Pin,   /* 9  */
    RED_4_Pin,   /* 10 */
    RED_1_Pin    /* 11 */
};

/* Exercise 7: turn every LED off */
void clearAllClock(void)
{
    for (int j = 0; j < 12; j++)
    {
        HAL_GPIO_WritePin(GPIOA, RED_PINS[j], GPIO_PIN_RESET);
    }
}

/* Exercise 8: turn on the LED for clock position num (0 to 11) */
void setNumberOnClock(int num)
{
    if (num < 0 || num > 11) return;
    HAL_GPIO_WritePin(GPIOA, CLOCK_PINS[num], GPIO_PIN_SET);
}

/* Exercise 9: turn off the LED for clock position num (0 to 11) */
void clearNumberOnClock(int num)
{
    if (num < 0 || num > 11) return;
    HAL_GPIO_WritePin(GPIOA, CLOCK_PINS[num], GPIO_PIN_RESET);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  /* USER CODE BEGIN 2 */
  int hours = 2;
  int minutes = 0;
  int seconds = 0;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    int hourPos = hours % 12;   /* hour 0/12 -> LED at 12 o'clock */
    int minPos  = minutes / 5;  /* 60 minutes / 12 LEDs = 5 min per LED */
    int secPos  = seconds / 5;  /* 60 seconds / 12 LEDs = 5 sec per LED */

    clearAllClock();
    setNumberOnClock(hourPos);
    setNumberOnClock(minPos);
    setNumberOnClock(secPos);

    HAL_Delay(100);

    seconds ++ ;
    if (seconds >= 60)
    {
        seconds = 0;
        minutes++;
        if (minutes >= 60)
        {
            minutes = 0;
            hours++;
            if (hours >= 24) hours = 0;
        }
    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, RED_1_Pin|RED_2_Pin|RED_3_Pin|RED_4_Pin
                          |RED_5_Pin|RED_6_Pin|RED_7_Pin|RED_8_Pin
                          |RED_9_Pin|RED_10_Pin|RED_11_Pin|RED_12_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : RED_1_Pin RED_2_Pin RED_3_Pin RED_4_Pin
                           RED_5_Pin RED_6_Pin RED_7_Pin RED_8_Pin
                           RED_9_Pin RED_10_Pin RED_11_Pin RED_12_Pin */
  GPIO_InitStruct.Pin = RED_1_Pin|RED_2_Pin|RED_3_Pin|RED_4_Pin
                          |RED_5_Pin|RED_6_Pin|RED_7_Pin|RED_8_Pin
                          |RED_9_Pin|RED_10_Pin|RED_11_Pin|RED_12_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
