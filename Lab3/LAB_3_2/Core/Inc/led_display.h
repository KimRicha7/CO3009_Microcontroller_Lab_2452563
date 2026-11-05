#ifndef INC_LED_DISPLAY_H_
#define INC_LED_DISPLAY_H_

#define ROAD_1 0
#define ROAD_2 1

/* Moi digit sang SCAN_PERIOD ms (4 digit -> 80 ms/vong). 20 ms giam tai cho Proteus. */
#define SCAN_PERIOD 20

enum Light { LIGHT_OFF, LIGHT_RED, LIGHT_AMBER, LIGHT_GREEN };

void display_init(void);
void led_set_road(int road, enum Light light);   /* chi sua output buffer */
void led_set_all(enum Light light);
void seg_set_numbers(int left, int right);        /* left -> digit 0-1, right -> digit 2-3 */
void seg_show_error(int visible);                 /* hien "Er Er" (visible = 1) hoac tat het */
void display_run(void);                           /* output driver, goi trong while(1) */

#endif /* INC_LED_DISPLAY_H_ */
