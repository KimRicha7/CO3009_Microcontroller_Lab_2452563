#ifndef INC_SCHEDULER_H_
#define INC_SCHEDULER_H_

#include <stdint.h>

#define SCH_TICK_MS      10          /* TIM2 interrupt period (ms) */
#define SCH_MAX_TASKS    16          /* maximum number of tasks at the same time */
#define SCH_NO_TASK_ID   0           /* invalid task ID */

/* Return codes of SCH_Delete_Task */
#define SCH_OK           1
#define SCH_ERROR        0

void     SCH_Init(void);
void     SCH_Update(void);                      /* called in the timer interrupt, O(1) */
void     SCH_Dispatch_Tasks(void);              /* called in while(1) */
uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD);
uint8_t  SCH_Delete_Task(uint32_t taskID);
uint32_t get_time(void);                        /* time since start-up (ms) */

#endif /* INC_SCHEDULER_H_ */
