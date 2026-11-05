#ifndef INC_GLOBAL_H_
#define INC_GLOBAL_H_

/* Cac mode, doi bang BUTTON_1: 1 -> 2 -> 3 -> 4 -> 1 */
#define MODE_NORMAL 1
#define MODE_RED    2
#define MODE_AMBER  3
#define MODE_GREEN  4
#define MODE_ERROR  5   /* trang thai noi bo: dang hien canh bao Er */

/* Thoi gian mac dinh (giay), luon giu red = amber + green */
#define DEFAULT_RED   5
#define DEFAULT_AMBER 2
#define DEFAULT_GREEN 3

#define MIN_DURATION 1
#define MAX_DURATION 99

extern int mode;
extern int durationRed;
extern int durationAmber;
extern int durationGreen;
extern int tempValue;      /* gia tri dang chinh trong mode 2-4, chua luu */

#endif /* INC_GLOBAL_H_ */
