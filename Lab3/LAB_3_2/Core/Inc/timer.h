#ifndef INC_TIMER_H_
#define INC_TIMER_H_

/* Chu ky ngat TIM2 (ms). Doi tick timer: chi sua so nay + Prescaler/Period trong .ioc */
#define TIMER_CYCLE 10

/* Danh sach software timer */
enum {
    TIMER_SECOND = 0,   /* nhip 1 giay cho dem nguoc den giao thong */
    TIMER_BLINK,        /* nhap nhay 2 Hz trong mode 2-4 */
    TIMER_SCAN,         /* quet 4 LED 7 doan */
    TIMER_AUTO,         /* tu tang gia tri khi giu nut 2 */
    TIMER_ERROR,        /* thoi gian hien canh bao Er */
    NUM_TIMERS
};

void setTimer(int index, int duration_ms);
int  isTimerExpired(int index);   /* tra 1 dung mot lan khi het gio */
void timerRun(void);

#endif /* INC_TIMER_H_ */
