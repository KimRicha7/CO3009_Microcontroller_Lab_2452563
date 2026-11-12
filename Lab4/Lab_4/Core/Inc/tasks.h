#ifndef INC_TASKS_H_
#define INC_TASKS_H_

void tasks_setup(void);   /* add all demo tasks to the scheduler (call before while(1)) */
void uart_print(const char *fmt, ...);

#endif /* INC_TASKS_H_ */
