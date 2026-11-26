#ifndef INC_COMMAND_PARSER_H_
#define INC_COMMAND_PARSER_H_

#include <stdint.h>

#define MAX_BUFFER_SIZE   30      /* receive buffer filled by the UART ISR */
#define MAX_CMD_SIZE      10      /* longest text between '!' and '#' */

/* command_flag values */
#define CMD_NONE          0
#define CMD_RST           1       /* received !RST# */
#define CMD_OK            2       /* received !OK#  */

/* Shared with HAL_UART_RxCpltCallback in main.c */
extern uint8_t          temp;
extern uint8_t          buffer[MAX_BUFFER_SIZE];
extern volatile uint8_t index_buffer;
extern volatile uint8_t buffer_flag;

/* Outputs of the parser, read by uart_communication_fsm() */
extern uint8_t command_flag;
extern uint8_t command_data[MAX_CMD_SIZE];

void command_parser_fsm(void);

#endif /* INC_COMMAND_PARSER_H_ */
