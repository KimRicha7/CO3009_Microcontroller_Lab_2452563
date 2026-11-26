#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include <stdint.h>

#define TIMER_TICK_MS     10      /* TIM2 interrupt period */
#define NUM_OF_TIMERS     2

#define TIMER_TIMEOUT     0       /* 3 s timeout waiting for !OK# */
#define TIMER_LED         1       /* 500 ms LED blinky */

void    setTimer(uint8_t index, uint32_t duration_ms);
uint8_t isTimerExpired(uint8_t index);
void    timerRun(void);           /* call every 10 ms from the TIM2 ISR */

#endif /* INC_SOFTWARE_TIMER_H_ */
