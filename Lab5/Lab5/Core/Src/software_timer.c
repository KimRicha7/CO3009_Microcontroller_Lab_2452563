#include "software_timer.h"

static volatile uint32_t timer_counter[NUM_OF_TIMERS] = {0};
static volatile uint8_t  timer_flag[NUM_OF_TIMERS]    = {0};

void setTimer(uint8_t index, uint32_t duration_ms) {
	if (index >= NUM_OF_TIMERS) return;
	timer_flag[index]    = 0;
	timer_counter[index] = duration_ms / TIMER_TICK_MS;
	if (timer_counter[index] == 0) timer_flag[index] = 1;
}

uint8_t isTimerExpired(uint8_t index) {
	if (index >= NUM_OF_TIMERS) return 0;
	if (timer_flag[index] == 1) {
		timer_flag[index] = 0;    /* consume the event once */
		return 1;
	}
	return 0;
}

void timerRun(void) {
	for (uint8_t i = 0; i < NUM_OF_TIMERS; i++) {
		if (timer_counter[i] > 0) {
			timer_counter[i]--;
			if (timer_counter[i] == 0) timer_flag[i] = 1;
		}
	}
}
