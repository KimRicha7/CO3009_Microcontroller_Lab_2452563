#include "main.h"
#include "input_processing.h"
#include "input_reading.h"
#include "global.h"
#include "fsm_traffic.h"
#include "led_display.h"
#include "timer.h"

#define BLINK_PERIOD       250    /* doi trang thai moi 250 ms = 2 Hz */
#define AUTO_REPEAT_PERIOD 200    /* giu nut 2 > 1 s: +1 moi 200 ms */
#define ERROR_SHOW_TIME    2000   /* hien "Er" trong 2 giay */

static int blinkOn = 0;
static int autoRepeat = 0;

/* Bo gia tri hop le gan nhat (red = amber + green), dung de khoi phuc khi sai */
static int backupRed, backupAmber, backupGreen;

static enum Light mode_color(void)
{
    if (mode == MODE_RED)   return LIGHT_RED;
    if (mode == MODE_AMBER) return LIGHT_AMBER;
    return LIGHT_GREEN;
}

static void enter_mode(int newMode)
{
    autoRepeat = 0;

    /* Roi mode 1 de chinh: luu lai bo gia tri dang hop le */
    if (mode == MODE_NORMAL && newMode == MODE_RED) {
        backupRed   = durationRed;
        backupAmber = durationAmber;
        backupGreen = durationGreen;
    }

    /* Roi mode 4 ve mode 1: kiem tra quy tac red = amber + green */
    if (mode == MODE_GREEN && newMode == MODE_NORMAL &&
        durationRed != durationAmber + durationGreen) {
        mode = MODE_ERROR;                  /* hien canh bao truoc */
        led_set_all(LIGHT_OFF);
        blinkOn = 1;
        seg_show_error(1);
        setTimer(TIMER_BLINK, BLINK_PERIOD);
        setTimer(TIMER_ERROR, ERROR_SHOW_TIME);
        return;
    }

    mode = newMode;
    if (mode == MODE_NORMAL) {
        traffic_reset();                    /* chay lai voi thoi gian (moi hoac da khoi phuc) */
        return;
    }
    if (mode == MODE_RED)   tempValue = durationRed;
    if (mode == MODE_AMBER) tempValue = durationAmber;
    if (mode == MODE_GREEN) tempValue = durationGreen;
    blinkOn = 1;
    led_set_all(mode_color());
    setTimer(TIMER_BLINK, BLINK_PERIOD);
}

static void increase_temp(void)
{
    tempValue++;
    if (tempValue > MAX_DURATION) tempValue = MIN_DURATION;   /* 99 -> 1 */
}

/* Luu gia tri: moi mau luu doc lap; quy tac duoc kiem tra khi quay ve mode 1 */
static void save_value(void)
{
    if (mode == MODE_RED)   durationRed   = tempValue;
    if (mode == MODE_AMBER) durationAmber = tempValue;
    if (mode == MODE_GREEN) durationGreen = tempValue;
}

void fsm_mode_init(void)
{
    backupRed   = durationRed;
    backupAmber = durationAmber;
    backupGreen = durationGreen;
    mode = MODE_NORMAL;
    enter_mode(MODE_NORMAL);
}

void fsm_for_input_processing(void)
{
    /* Dang hien "Er": bo qua nut nhan, nhap nhay 2 Hz, het 2 s thi khoi phuc */
    if (mode == MODE_ERROR) {
        (void)is_button_pressed_once(0);
        (void)is_button_pressed_once(1);
        (void)is_button_pressed_once(2);
        if (isTimerExpired(TIMER_BLINK)) {
            setTimer(TIMER_BLINK, BLINK_PERIOD);
            blinkOn = !blinkOn;
            seg_show_error(blinkOn);
        }
        if (isTimerExpired(TIMER_ERROR)) {
            durationRed   = backupRed;          /* khoi phuc bo gia tri hop le */
            durationAmber = backupAmber;
            durationGreen = backupGreen;
            mode = MODE_NORMAL;
            traffic_reset();
        }
        return;
    }

    /* BUTTON_1: doi mode 1 -> 2 -> 3 -> 4 -> 1 */
    if (is_button_pressed_once(0)) {
        enter_mode(mode == MODE_GREEN ? MODE_NORMAL : mode + 1);
        if (mode == MODE_ERROR) return;
    }

    if (mode == MODE_NORMAL) {
        (void)is_button_pressed_once(1);   /* bo qua nut 2, 3 trong mode 1 */
        (void)is_button_pressed_once(2);
        traffic_run();
        seg_set_numbers(traffic_time1(), traffic_time2());
        return;
    }

    /* Mode 2-4: nhap nhay 2 Hz cac LED cung mau */
    if (isTimerExpired(TIMER_BLINK)) {
        setTimer(TIMER_BLINK, BLINK_PERIOD);
        blinkOn = !blinkOn;
        led_set_all(blinkOn ? mode_color() : LIGHT_OFF);
    }

    /* BUTTON_2: tang gia tri; giu hon 1 s thi tu tang */
    if (is_button_pressed_once(1)) {
        increase_temp();
    }
    if (is_button_pressed_1s(1)) {
        if (!autoRepeat) {
            autoRepeat = 1;
            increase_temp();
            setTimer(TIMER_AUTO, AUTO_REPEAT_PERIOD);
        } else if (isTimerExpired(TIMER_AUTO)) {
            increase_temp();
            setTimer(TIMER_AUTO, AUTO_REPEAT_PERIOD);
        }
    } else {
        autoRepeat = 0;
    }

    /* BUTTON_3: luu gia tri */
    if (is_button_pressed_once(2)) {
        save_value();
    }

    seg_set_numbers(tempValue, mode);   /* digit 0-1: gia tri, digit 2-3: so mode */
}
