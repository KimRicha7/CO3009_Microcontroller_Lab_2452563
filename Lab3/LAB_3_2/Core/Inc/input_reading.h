#ifndef INC_INPUT_READING_H_
#define INC_INPUT_READING_H_

#include <stdint.h>

#define NO_OF_BUTTONS 3   /* 0 = BUTTON_1 (mode), 1 = BUTTON_2 (tang), 2 = BUTTON_3 (luu) */

void button_init(void);
void button_reading(void);
unsigned char is_button_pressed(uint8_t index);       /* dang duoc giu */
unsigned char is_button_pressed_1s(uint8_t index);    /* da giu hon 1 giay */
unsigned char is_button_pressed_once(uint8_t index);  /* vua nhan (1 lan moi lan nhan) */

#endif /* INC_INPUT_READING_H_ */
