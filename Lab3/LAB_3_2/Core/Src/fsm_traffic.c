#include "main.h"
#include "fsm_traffic.h"
#include "global.h"
#include "led_display.h"
#include "timer.h"

/* Moi road co FSM rieng: RED -> GREEN -> AMBER -> RED ...
 * Road 2 lech pha so voi road 1 de hai road khong bao gio cung xanh/vang.
 * Gia tri red, amber, green duoc luu doc lap:
 *  - red = green + amber : chay binh thuong
 *  - red > green + amber : phan du thanh khoang ALL-RED (ca hai road deu do),
 *                          chia doi cho hai lan chuyen pha
 *  - red < green + amber : chu ky nay dung red = green + amber de an toan
 *                          (gia tri red da luu khong bi thay doi) */

enum RoadState { ROAD_RED, ROAD_GREEN, ROAD_AMBER };

static enum RoadState roadState[2];
static int roadTime[2];
static int effectiveRed = 0;   /* thoi gian do thuc te dung cho chu ky hien tai */

static void show_lights(void)
{
    for (int r = 0; r < 2; r++) {
        if (roadState[r] == ROAD_RED)        led_set_road(r, LIGHT_RED);
        else if (roadState[r] == ROAD_GREEN) led_set_road(r, LIGHT_GREEN);
        else                                 led_set_road(r, LIGHT_AMBER);
    }
}

void traffic_reset(void)
{
    effectiveRed = durationRed;
    if (effectiveRed < durationGreen + durationAmber) {
        effectiveRed = durationGreen + durationAmber;
    }
    int spare = effectiveRed - durationGreen - durationAmber;   /* tong thoi gian all-red */
    int clearanceBeforeRoad2 = spare / 2;                        /* phan con lai nam truoc road 1 xanh */

    roadState[ROAD_1] = ROAD_RED;
    roadTime[ROAD_1]  = effectiveRed;

    if (clearanceBeforeRoad2 > 0) {
        roadState[ROAD_2] = ROAD_RED;
        roadTime[ROAD_2]  = clearanceBeforeRoad2;
    } else {
        roadState[ROAD_2] = ROAD_GREEN;
        roadTime[ROAD_2]  = durationGreen;
    }
    show_lights();
    setTimer(TIMER_SECOND, 1000);
}

static void next_state(int r)
{
    switch (roadState[r]) {
    case ROAD_RED:   roadState[r] = ROAD_GREEN; roadTime[r] = durationGreen; break;
    case ROAD_GREEN: roadState[r] = ROAD_AMBER; roadTime[r] = durationAmber; break;
    case ROAD_AMBER: roadState[r] = ROAD_RED;   roadTime[r] = effectiveRed;  break;
    }
}

void traffic_run(void)
{
    if (!isTimerExpired(TIMER_SECOND)) return;
    setTimer(TIMER_SECOND, 1000);

    for (int r = 0; r < 2; r++) {
        roadTime[r]--;
        if (roadTime[r] <= 0) next_state(r);
    }
    show_lights();
}

int traffic_time1(void) { return roadTime[ROAD_1]; }
int traffic_time2(void) { return roadTime[ROAD_2]; }
