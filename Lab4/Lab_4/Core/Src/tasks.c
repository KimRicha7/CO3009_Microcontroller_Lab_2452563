#include "main.h"
#include "tasks.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdarg.h>

extern UART_HandleTypeDef huart1;

/* Fallback if PA4 has no label in the .ioc */
#ifndef LED_2500MS_Pin
#define LED_2500MS_Pin       GPIO_PIN_4
#define LED_2500MS_GPIO_Port GPIOA
#endif

#define TICK_PRINT_DURATION 1000   /* the 10 ms task prints for the first 1 s, then deletes itself */

static uint32_t id10ms = SCH_NO_TASK_ID;

void uart_print(const char *fmt, ...)
{
    char buf[64];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    if (n < 0) return;
    if (n > (int)sizeof(buf) - 1) n = sizeof(buf) - 1;
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100);
}

/* ---------- 5 periodic tasks (lab requirement) ---------- */
static void Task_LED_500ms(void)
{
    HAL_GPIO_TogglePin(LED_500MS_GPIO_Port, LED_500MS_Pin);
    uart_print("%7lu ms: LED 0.5s\r\n", get_time());
}

static void Task_LED_1s(void)
{
    HAL_GPIO_TogglePin(LED_1S_GPIO_Port, LED_1S_Pin);
    uart_print("%7lu ms: LED 1s\r\n", get_time());
}

static void Task_LED_1500ms(void)
{
    HAL_GPIO_TogglePin(LED_1500MS_GPIO_Port, LED_1500MS_Pin);
    uart_print("%7lu ms: LED 1.5s\r\n", get_time());
}

static void Task_LED_2s(void)
{
    HAL_GPIO_TogglePin(LED_2S_GPIO_Port, LED_2S_Pin);
    uart_print("%7lu ms: LED 2s\r\n", get_time());
}

static void Task_LED_2500ms(void)
{
    HAL_GPIO_TogglePin(LED_2500MS_GPIO_Port, LED_2500MS_Pin);
    uart_print("%7lu ms: LED 2.5s\r\n", get_time());
}

/* ---------- demo: callback every 10 ms printing get_time() ---------- */
static void Task_10ms(void)
{
    uint32_t t = get_time();
    uart_print("%7lu ms: tick 10ms\r\n", t);
    if (t >= TICK_PRINT_DURATION) {
        SCH_Delete_Task(id10ms);            /* demonstrates SCH_Delete_Task, keeps the terminal readable */
        uart_print("%7lu ms: 10ms task deleted\r\n", t);
    }
}

/* ---------- demo: one-shot task (PERIOD = 0) ---------- */
static void Task_OneShot(void)
{
    uart_print("%7lu ms: one-shot fired\r\n", get_time());
}

void tasks_setup(void)
{
    SCH_Init();

    id10ms = SCH_Add_Task(Task_10ms, 10, 10);

    uint32_t id1 = SCH_Add_Task(Task_LED_500ms,  500,  500);
    uint32_t id2 = SCH_Add_Task(Task_LED_1s,     1000, 1000);
    uint32_t id3 = SCH_Add_Task(Task_LED_1500ms, 1500, 1500);
    uint32_t id4 = SCH_Add_Task(Task_LED_2s,     2000, 2000);
    uint32_t id5 = SCH_Add_Task(Task_LED_2500ms, 2500, 2500);

    /* One-shot tasks: delays long enough that while(1) is entered before they run */
    SCH_Add_Task(Task_OneShot, 1230, 0);
    SCH_Add_Task(Task_OneShot, 3330, 0);
    SCH_Add_Task(Task_OneShot, 5550, 0);

    uart_print("\r\nLab 4 scheduler start\r\n");
    uart_print("IDs: 10ms=%lu 0.5s=%lu 1s=%lu 1.5s=%lu 2s=%lu 2.5s=%lu\r\n",
               id10ms, id1, id2, id3, id4, id5);
}
