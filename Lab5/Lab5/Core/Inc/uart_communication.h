#ifndef INC_UART_COMMUNICATION_H_
#define INC_UART_COMMUNICATION_H_

#define RESPONSE_TIMEOUT_MS   3000    /* wait for !OK# before resending */

void uart_communication_fsm(void);

#endif /* INC_UART_COMMUNICATION_H_ */
