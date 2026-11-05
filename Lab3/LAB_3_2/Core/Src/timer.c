#include "main.h"
#include "timer.h"
#include "input_reading.h"

static volatile int timer_counter[NUM_TIMERS];
static volatile int timer_flag[NUM_TIMERS];

/* Tham so tinh bang ms, doi ra so tick theo TIMER_CYCLE (Exercise 4) */
void setTimer(int index, int duration_ms)
{
    if (index < 0 || index >= NUM_TIMERS) return;
    int ticks = duration_ms / TIMER_CYCLE;
    if (ticks < 1) ticks = 1;
    timer_flag[index] = 0;
    timer_counter[index] = ticks;
}

int isTimerExpired(int index)
{
    if (index < 0 || index >= NUM_TIMERS) return 0;
    if (timer_flag[index]) {
        timer_flag[index] = 0;
        return 1;
    }
    return 0;
}

void timerRun(void)
{
    for (int i = 0; i < NUM_TIMERS; i++) {
        if (timer_counter[i] > 0) {
            timer_counter[i]--;
            if (timer_counter[i] == 0) timer_flag[i] = 1;
        }
    }
}

/* Goi moi TIMER_CYCLE ms boi TIM2 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        button_reading();
        timerRun();
    }
}
