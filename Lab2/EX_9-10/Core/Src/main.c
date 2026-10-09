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
#define SWITCH_TIME   250  /* 250ms per digit -> 4 digits = 1s = 1Hz scanning */
#define MATRIX_TIME   10   /* 10ms per matrix column (Exercise 9)            */
#define SHIFT_TIME    200  /* 500ms between shifts (Exercise 10)             */

#define ENABLE_SHIFT  1    /* 0 = Exercise 9 (static "A"), 1 = Exercise 10 (shift) */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* 7-segment clock (from Exercise 8) */
const int MAX_LED = 4;
int index_led = 0;
int led_buffer[4] = {0, 0, 0, 0};   /* filled by updateClockBuffer() */
int hour = 15, minute = 8, second = 50;


/* Exercise 10: scrolling text. The "A" followed by 8 blank columns, so the
   character leaves the screen completely before entering again from the right. */
#define SCROLL_LEN 16
const uint8_t scroll_buffer[SCROLL_LEN] = {
  0x00, 0x7C, 0x12, 0x11, 0x11, 0x12, 0x7C, 0x00,   /* "A"           */
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00    /* blank gap     */
};
int scroll_pos = 0;   /* first column of scroll_buffer shown on the matrix */

/* LED matrix (Exercise 9) */
const int MAX_LED_MATRIX = 8;
int index_led_matrix = 0;

/* Column patterns for the letter "A". Each byte: bit r = ROWr (1 = LED on). */
uint8_t matrix_buffer[8] = {0x00, 0x7C, 0x12, 0x11, 0x11, 0x12, 0x7C, 0x00};

uint16_t ROW_PINS[8] = {ROW0_Pin, ROW1_Pin, ROW2_Pin, ROW3_Pin,
                         ROW4_Pin, ROW5_Pin, ROW6_Pin, ROW7_Pin};
uint16_t ENM_PINS[8] = {ENM0_Pin, ENM1_Pin, ENM2_Pin, ENM3_Pin,
                         ENM4_Pin, ENM5_Pin, ENM6_Pin, ENM7_Pin};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* Software timers, counted down every timer interrupt (10ms)
   timer0: 1s tick for the clock and the DOT
   timer1: 250ms tick for 7-segment scanning
   timer2: 10ms tick for LED matrix column scanning (Exercise 9)
   timer3: 500ms tick for shifting the matrix (Exercise 10)      */
int timer0_counter = 0;
volatile int timer0_flag = 0;   /* volatile: set in the interrupt, read in main */
int timer1_counter = 0;
volatile int timer1_flag = 0;
int timer2_counter = 0;
volatile int timer2_flag = 0;
int timer3_counter = 0;
volatile int timer3_flag = 0;
int TIMER_CYCLE = 10;           /* timer interrupt period in ms */

void setTimer0(int duration)
{
  timer0_counter = duration / TIMER_CYCLE;
  timer0_flag = 0;
}

void setTimer1(int duration)
{
  timer1_counter = duration / TIMER_CYCLE;
  timer1_flag = 0;
}

void setTimer2(int duration)
{
  timer2_counter = duration / TIMER_CYCLE;
  timer2_flag = 0;
}

void setTimer3(int duration)
{
  timer3_counter = duration / TIMER_CYCLE;
  timer3_flag = 0;
}

void timer_run(void)
{
  if (timer0_counter > 0)
  {
    timer0_counter--;
    if (timer0_counter == 0) timer0_flag = 1;
  }
  if (timer1_counter > 0)
  {
    timer1_counter--;
    if (timer1_counter == 0) timer1_flag = 1;
  }
  if (timer2_counter > 0)
  {
    timer2_counter--;
    if (timer2_counter == 0) timer2_flag = 1;
  }
  if (timer3_counter > 0)
  {
    timer3_counter--;
    if (timer3_counter == 0) timer3_flag = 1;
  }
}

/* Common-anode 7-segment: a segment is ON when its pin is LOW.
   SEG0..SEG6 = a..g. Bit i of the table = segment i (1 = ON).            */
void display7SEG(int num)
{
  static const uint8_t SEG_TABLE[10] = {
    0x3F, /* 0 */ 0x06, /* 1 */ 0x5B, /* 2 */ 0x4F, /* 3 */ 0x66, /* 4 */
    0x6D, /* 5 */ 0x7D, /* 6 */ 0x07, /* 7 */ 0x7F, /* 8 */ 0x6F  /* 9 */
  };
  GPIO_TypeDef* const port[7] = {SEG0_GPIO_Port, SEG1_GPIO_Port, SEG2_GPIO_Port,
                                 SEG3_GPIO_Port, SEG4_GPIO_Port, SEG5_GPIO_Port,
                                 SEG6_GPIO_Port};
  const uint16_t pin[7] = {SEG0_Pin, SEG1_Pin, SEG2_Pin, SEG3_Pin,
                           SEG4_Pin, SEG5_Pin, SEG6_Pin};

  uint8_t code = (num >= 0 && num <= 9) ? SEG_TABLE[num] : 0x00; /* invalid -> blank */
  for (int i = 0; i < 7; i++)
  {
    HAL_GPIO_WritePin(port[i], pin[i], ((code >> i) & 1) ? GPIO_PIN_RESET : GPIO_PIN_SET);
  }
}

/* Show led_buffer[index] on 7-segment number 'index'.
   PNP transistors: a digit is ON when its EN pin is LOW.              */
void update7SEG(int index)
{
  /* All digits OFF first so the new number doesn't ghost on the old digit */
  HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_SET);

  switch (index)
  {
    case 0:
      display7SEG(led_buffer[0]);
      HAL_GPIO_WritePin(EN0_GPIO_Port, EN0_Pin, GPIO_PIN_RESET);
      break;
    case 1:
      display7SEG(led_buffer[1]);
      HAL_GPIO_WritePin(EN1_GPIO_Port, EN1_Pin, GPIO_PIN_RESET);
      break;
    case 2:
      display7SEG(led_buffer[2]);
      HAL_GPIO_WritePin(EN2_GPIO_Port, EN2_Pin, GPIO_PIN_RESET);
      break;
    case 3:
      display7SEG(led_buffer[3]);
      HAL_GPIO_WritePin(EN3_GPIO_Port, EN3_Pin, GPIO_PIN_RESET);
      break;
    default:
      break;
  }
}

/* Copy hour and minute into led_buffer as HH MM. */
void updateClockBuffer(void)
{
  led_buffer[0] = hour / 10;
  led_buffer[1] = hour % 10;
  led_buffer[2] = minute / 10;
  led_buffer[3] = minute % 10;
}

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
	scroll_pos++;
	  if (scroll_pos >= SCROLL_LEN) scroll_pos = 0;

	  for (int i = 0; i < MAX_LED_MATRIX; i++)
	  {
	    matrix_buffer[i] = scroll_buffer[(scroll_pos + i) % SCROLL_LEN];
	  }
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
  /* Start with DOT and LED_RED off (HIGH = off) */
  HAL_GPIO_WritePin(DOT_GPIO_Port, DOT_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(LED_RED_GPIO_Port, LED_RED_Pin, GPIO_PIN_SET);

  /* Start with every matrix column OFF and every row OFF */
  HAL_GPIO_WritePin(GPIOA, ENM0_Pin|ENM1_Pin|ENM2_Pin|ENM3_Pin
                          |ENM4_Pin|ENM5_Pin|ENM6_Pin|ENM7_Pin, GPIO_PIN_SET);
  HAL_GPIO_WritePin(GPIOB, ROW0_Pin|ROW1_Pin|ROW2_Pin|ROW3_Pin
                          |ROW4_Pin|ROW5_Pin|ROW6_Pin|ROW7_Pin, GPIO_PIN_SET);

  /* Load the starting time and show the first digit */
  updateClockBuffer();
  update7SEG(index_led++);

  HAL_TIM_Base_Start_IT(&htim2);

  setTimer0(1000);          /* clock + DOT        */
  setTimer1(SWITCH_TIME);   /* 7-segment scanning */
  setTimer2(MATRIX_TIME);   /* matrix scanning    */
  setTimer3(SHIFT_TIME);    /* matrix shifting    */
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* Clock + DOT: every 1s (software timer 0) */
    if (timer0_flag == 1)
    {
      setTimer0(1000);
      HAL_GPIO_TogglePin(DOT_GPIO_Port, DOT_Pin);

      second++;
      if (second >= 60) { second = 0; minute++; }
      if (minute >= 60) { minute = 0; hour++; }
      if (hour >= 24)   { hour = 0; }
      updateClockBuffer();
    }

    /* 7-segment scanning: next digit every 250ms (software timer 1) */
    if (timer1_flag == 1)
    {
      setTimer1(SWITCH_TIME);
      if (index_led >= MAX_LED) index_led = 0;
      update7SEG(index_led++);
    }

    /* Exercise 9: LED matrix scanning, next column every 10ms (software timer 2) */
    if (timer2_flag == 1)
    {
      setTimer2(MATRIX_TIME);
      updateLEDMatrix(index_led_matrix++);
      if (index_led_matrix >= MAX_LED_MATRIX) index_led_matrix = 0;
    }

#if ENABLE_SHIFT
    /* Exercise 10: shift the character left every 500ms (software timer 3) */
    if (timer3_flag == 1)
    {
      setTimer3(SHIFT_TIME);
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
/* The interrupt (every 10ms) only runs the software timers.
   All processing is done in the main loop (Exercise 8 rule). */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance != TIM2) return;

  timer_run();
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
