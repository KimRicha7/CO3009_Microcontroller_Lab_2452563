#include "main.h"
#include "led_display.h"
#include "timer.h"

/* ---------- output buffer ---------- */
static enum Light roadLight[2] = { LIGHT_OFF, LIGHT_OFF };
static int lastWritten[2] = { -1, -1 };   /* chi ghi chan LED khi mau thay doi */
static uint8_t digitBuffer[4] = { 0, 0, 0, 0 };
static int scanIndex = 0;

/* ---------- bang chan ---------- */
static GPIO_TypeDef *const redPort[2]   = { RED1_GPIO_Port,   RED2_GPIO_Port };
static const uint16_t      redPin[2]    = { RED1_Pin,         RED2_Pin };
static GPIO_TypeDef *const amberPort[2] = { AMBER1_GPIO_Port, AMBER2_GPIO_Port };
static const uint16_t      amberPin[2]  = { AMBER1_Pin,       AMBER2_Pin };
static GPIO_TypeDef *const greenPort[2] = { GREEN1_GPIO_Port, GREEN2_GPIO_Port };
static const uint16_t      greenPin[2]  = { GREEN1_Pin,       GREEN2_Pin };

static GPIO_TypeDef *const enPort[4] = { EN0_GPIO_Port, EN1_GPIO_Port, EN2_GPIO_Port, EN3_GPIO_Port };
static const uint16_t      enPin[4]  = { EN0_Pin, EN1_Pin, EN2_Pin, EN3_Pin };
static GPIO_TypeDef *const segPort[7] = { SEG0_GPIO_Port, SEG1_GPIO_Port, SEG2_GPIO_Port, SEG3_GPIO_Port,
                                          SEG4_GPIO_Port, SEG5_GPIO_Port, SEG6_GPIO_Port };
static const uint16_t      segPin[7]  = { SEG0_Pin, SEG1_Pin, SEG2_Pin, SEG3_Pin, SEG4_Pin, SEG5_Pin, SEG6_Pin };

/* bit0 = a (SEG0) ... bit6 = g (SEG6), 1 = doan sang
 * 0-9: chu so, 10: tat, 11: 'E', 12: 'r' */
#define DIGIT_BLANK 10
#define DIGIT_E     11
#define DIGIT_R     12
static const uint8_t digitPattern[13] = { 0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D, 0x7D, 0x07, 0x7F, 0x6F,
                                          0x00, 0x79, 0x50 };

void led_set_road(int road, enum Light light)
{
    if (road == ROAD_1 || road == ROAD_2) roadLight[road] = light;
}

void led_set_all(enum Light light)
{
    roadLight[ROAD_1] = light;
    roadLight[ROAD_2] = light;
}

static int clamp99(int v)
{
    if (v < 0) return 0;
    if (v > 99) return 99;
    return v;
}

void seg_set_numbers(int left, int right)
{
    left = clamp99(left);
    right = clamp99(right);
    digitBuffer[0] = left / 10;
    digitBuffer[1] = left % 10;
    digitBuffer[2] = right / 10;
    digitBuffer[3] = right % 10;
}

void seg_show_error(int visible)
{
    digitBuffer[0] = visible ? DIGIT_E : DIGIT_BLANK;
    digitBuffer[1] = visible ? DIGIT_R : DIGIT_BLANK;
    digitBuffer[2] = visible ? DIGIT_E : DIGIT_BLANK;
    digitBuffer[3] = visible ? DIGIT_R : DIGIT_BLANK;
}

/* ---------- output driver ---------- */
static void write_traffic_leds(void)
{
    for (int r = 0; r < 2; r++) {   /* LED active-high */
        if ((int)roadLight[r] == lastWritten[r]) continue;   /* khong doi -> khong ghi */
        lastWritten[r] = (int)roadLight[r];
        HAL_GPIO_WritePin(redPort[r],   redPin[r],   roadLight[r] == LIGHT_RED   ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(amberPort[r], amberPin[r], roadLight[r] == LIGHT_AMBER ? GPIO_PIN_SET : GPIO_PIN_RESET);
        HAL_GPIO_WritePin(greenPort[r], greenPin[r], roadLight[r] == LIGHT_GREEN ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

static void write_segments(uint8_t pattern)
{
    for (int k = 0; k < 7; k++) {   /* common anode: 0 = doan sang */
        HAL_GPIO_WritePin(segPort[k], segPin[k], ((pattern >> k) & 0x01) ? GPIO_PIN_RESET : GPIO_PIN_SET);
    }
}

static void scan_next_digit(void)
{
    for (int e = 0; e < 4; e++) {   /* PNP: EN = 1 la tat */
        HAL_GPIO_WritePin(enPort[e], enPin[e], GPIO_PIN_SET);
    }
    write_segments(digitPattern[digitBuffer[scanIndex]]);
    HAL_GPIO_WritePin(enPort[scanIndex], enPin[scanIndex], GPIO_PIN_RESET);
    scanIndex = (scanIndex + 1) % 4;
}

void display_init(void)
{
    for (int e = 0; e < 4; e++) HAL_GPIO_WritePin(enPort[e], enPin[e], GPIO_PIN_SET);
    write_segments(0x00);
    lastWritten[0] = lastWritten[1] = -1;   /* ep ghi lan dau */
    write_traffic_leds();
    setTimer(TIMER_SCAN, SCAN_PERIOD);
}

void display_run(void)
{
    write_traffic_leds();
    if (isTimerExpired(TIMER_SCAN)) {
        setTimer(TIMER_SCAN, SCAN_PERIOD);
        scan_next_digit();
    }
}
