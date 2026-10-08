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
#define SCAN_TIME      1   /* 1 x 10ms = 10ms per column -> steady image          */
#define SHIFT_TIME     50  /* 50 x 10ms = 500ms between shifts (Exercise 10)      */
#define HEARTBEAT_TIME 50  /* 50 x 10ms = 500ms -> D1 blinks at 1Hz (power check) */

#define ENABLE_SHIFT   0   /* 0 = Exercise 9 (static "A"), 1 = Exercise 10 (shift) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

/* Column patterns for the letter "A". Each byte: bit r = ROWr (1 = LED on). */
uint8_t matrix_buffer[8] = {0x00, 0x7C, 0x12, 0x11, 0x11, 0x12, 0x7C, 0x00};

uint16_t ROW_PINS[8] = {ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
                         ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin};
uint16_t ENM_PINS[8] = {ENM0_Pin, ENM1_Pin, ENM2_Pin, ENM3_Pin,
                         ENM4_Pin, ENM5_Pin, ENM6_Pin, ENM7_Pin};

/* Software timers: counted down in the interrupt, flags handled in main() */
int scan_counter = SCAN_TIME;
volatile int scan_flag = 0;

int shift_counter = SHIFT_TIME;
volatile int shift_flag = 0;

int heartbeat_counter = HEARTBEAT_TIME;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Exercise 9: display one column of matrix_buffer at a time (column scanning).
   Circuit logic (ULN2803 inverts, 100R pull-ups on the columns):
     ENMx = 1 -> ULN2803 pulls COLx LOW  -> column OFF
     ENMx = 0 -> pull-up keeps COLx HIGH -> column ON
     ROWr = 0 -> dot ON,  ROWr = 1 -> dot OFF                                  */
void updateLEDMatrix(int index)
{
  if (index < 0 || index >= MAX_LED_MATRIX) return;

  /* 1. Turn every column OFF so the previous column's data doesn't ghost */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin
                          |ENM4_Pin|ENM5_Pin|ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);

  /* 2. Load this column's dots onto the rows (active LOW) */
  uint8_t pattern = matrix_buffer[index];
  for (int row = 0; row < MAX_LED_MATRIX; row++)
  {
    HAL_GPIO_WritePin(GPIOB, ROW_PINS[row],
                      (pattern & (1 << row)) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  }

  /* 3. Turn only the current column ON */
  HAL_GPIO_WritePin(GPIOA, ENM_PINS[index], GPIO_PIN_RESET);
}

/* Exercise 10: shift the displayed character left, wrapping around for a looping animation. */
void shiftLeftMatrix(void)
{
  uint8_t first = matrix_buffer[0];
  for (int i = 0; i < MAX_LED_MATRIX - 1; i++)
  {
    matrix_buffer[i] = matrix_buffer[i + 1];
  }
  matrix_buffer[MAX_LED_MATRIX - 1] = first;
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
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /* Start with every matrix column OFF and every row OFF */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin
                          |ENM4_Pin|ENM5_Pin|ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, ROW0_Pin|ROW1_Pin|ROW2_Pin|ROW3_Pin
                          |ROW4_Pin|ROW5_Pin|ROW6_Pin|ROW7_Pin, GPIO_PIN_SET);

  HAL_TIM_Base_Start_IT(&htim2);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* Exercise 9: column scanning, called from main (not from the interrupt) */
    if (scan_flag == 1)
    {
      scan_flag = 0;
      updateLEDMatrix(index_led_matrix);
      index_led_matrix++;
      if (index_led_matrix >= MAX_LED_MATRIX)
      {
        index_led_matrix = 0;
      }
    }

#if ENABLE_SHIFT
    /* Exercise 10: shift the character left every SHIFT_TIME */
    if (shift_flag == 1)
    {
      shift_flag = 0;
      shiftLeftMatrix();
    }
#endif
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : ENM0_Pin ENM1_Pin DOT_Pin LED_RED_Pin
                           EN0_Pin EN1_Pin EN2_Pin EN3_Pin
                           ENM2_Pin ENM3_Pin ENM4_Pin ENM5_Pin
                           ENM6_Pin ENM7_Pin */
  GPIO_InitStruct.Pin = ENM0_Pin|ENM1_Pin|DOT_Pin|LED_RED_Pin
                          |EN0_Pin|EN1_Pin|EN2_Pin|EN3_Pin
                          |ENM2_Pin|ENM3_Pin|ENM4_Pin|ENM5_Pin
                          |ENM6_Pin|ENM7_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG0_Pin SEG1_Pin SEG2_Pin ROW2_Pin
                           ROW3_Pin ROW4_Pin ROW5_Pin ROW6_Pin
                           ROW7_Pin SEG3_Pin SEG4_Pin SEG5_Pin
                           SEG6_Pin ROW0_Pin ROW1_Pin */
  GPIO_InitStruct.Pin = SEG0_Pin|SEG1_Pin|SEG2_Pin|ROW2_Pin
                          |ROW3_Pin|ROW4_Pin|ROW5_Pin|ROW6_Pin
                          |ROW7_Pin|SEG3_Pin|SEG4_Pin|SEG5_Pin
                          |SEG6_Pin|ROW0_Pin|ROW1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
/* The interrupt (every 10ms) only runs the software timers and sets flags.
   All the real work is done in main() (Exercise 8 rule). */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM2) return;

  /* Exercise 9: column scan timer */
  if (scan_counter > 0)
  {
    scan_counter--;
    if (scan_counter == 0)
    {
      scan_flag = 1;
      scan_counter = SCAN_TIME;
    }
  }

  /* Exercise 10: shift timer */
  if (shift_counter > 0)
  {
    shift_counter--;
    if (shift_counter == 0)
    {
      shift_flag = 1;
      shift_counter = SHIFT_TIME;
    }
  }

  /* Power check: D1 (LED_RED) blinks at 1Hz.
     If PA5 toggles but D1 never lights, the +3.3V rail in Proteus is dead. */
  if (heartbeat_counter > 0)
  {
    heartbeat_counter--;
    if (heartbeat_counter == 0)
    {
      heartbeat_counter = HEARTBEAT_TIME;
      HAL_GPIO_TogglePin(GPIOA, LED_RED_Pin);
    }
  }
}
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
