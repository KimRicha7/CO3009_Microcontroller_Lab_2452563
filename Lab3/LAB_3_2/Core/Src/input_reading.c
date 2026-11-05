#include "main.h"
#include "input_reading.h"
#include "timer.h"

#define DURATION_FOR_AUTO_INCREASING (1000 / TIMER_CYCLE)   /* 1 giay */
#define BUTTON_IS_PRESSED  GPIO_PIN_RESET                   /* active-low */
#define BUTTON_IS_RELEASED GPIO_PIN_SET

static GPIO_TypeDef *const buttonPort[NO_OF_BUTTONS] = { BUTTON_1_GPIO_Port, BUTTON_2_GPIO_Port, BUTTON_3_GPIO_Port };
static const uint16_t      buttonPin[NO_OF_BUTTONS]  = { BUTTON_1_Pin, BUTTON_2_Pin, BUTTON_3_Pin };

static volatile GPIO_PinState buttonBuffer[NO_OF_BUTTONS];      /* ket qua sau debounce */
static GPIO_PinState debounceButtonBuffer1[NO_OF_BUTTONS];
static GPIO_PinState debounceButtonBuffer2[NO_OF_BUTTONS];
static volatile uint8_t flagForButtonPress1s[NO_OF_BUTTONS];
static uint16_t counterForButtonPress1s[NO_OF_BUTTONS];
static volatile uint8_t flagForButtonPressed[NO_OF_BUTTONS];    /* bat khi vua nhan */

void button_init(void)
{
    for (uint8_t i = 0; i < NO_OF_BUTTONS; i++) {
        buttonBuffer[i]          = BUTTON_IS_RELEASED;
        debounceButtonBuffer1[i] = BUTTON_IS_RELEASED;
        debounceButtonBuffer2[i] = BUTTON_IS_RELEASED;
        flagForButtonPress1s[i]    = 0;
        counterForButtonPress1s[i] = 0;
        flagForButtonPressed[i]    = 0;
    }
}

/* Goi moi 10 ms tu timer interrupt */
void button_reading(void)
{
    for (uint8_t i = 0; i < NO_OF_BUTTONS; i++) {
        debounceButtonBuffer2[i] = debounceButtonBuffer1[i];
        debounceButtonBuffer1[i] = HAL_GPIO_ReadPin(buttonPort[i], buttonPin[i]);

        if (debounceButtonBuffer1[i] == debounceButtonBuffer2[i]) {
            if (buttonBuffer[i] == BUTTON_IS_RELEASED && debounceButtonBuffer1[i] == BUTTON_IS_PRESSED) {
                flagForButtonPressed[i] = 1;          /* canh nhan: nha -> nhan */
            }
            buttonBuffer[i] = debounceButtonBuffer1[i];
        }

        if (buttonBuffer[i] == BUTTON_IS_PRESSED) {
            if (counterForButtonPress1s[i] < DURATION_FOR_AUTO_INCREASING) {
                counterForButtonPress1s[i]++;
            } else {
                flagForButtonPress1s[i] = 1;
            }
        } else {
            counterForButtonPress1s[i] = 0;
            flagForButtonPress1s[i] = 0;
        }
    }
}

unsigned char is_button_pressed(uint8_t index)
{
    if (index >= NO_OF_BUTTONS) return 0;
    return (buttonBuffer[index] == BUTTON_IS_PRESSED);
}

unsigned char is_button_pressed_1s(uint8_t index)
{
    if (index >= NO_OF_BUTTONS) return 0;
    return (flagForButtonPress1s[index] == 1);
}

unsigned char is_button_pressed_once(uint8_t index)
{
    if (index >= NO_OF_BUTTONS) return 0;
    if (flagForButtonPressed[index]) {
        flagForButtonPressed[index] = 0;
        return 1;
    }
    return 0;
}
