#include "main.h"
#include "uart_communication.h"
#include "command_parser.h"
#include "software_timer.h"
#include <stdio.h>

extern ADC_HandleTypeDef  hadc1;
extern UART_HandleTypeDef huart2;

/* Communication states */
typedef enum {
	COMM_WAIT_RST,        /* idle, waiting for !RST#                  */
	COMM_WAIT_OK          /* packet sent, waiting for !OK# (ACK)      */
} CommState;

static CommState comm_state = COMM_WAIT_RST;
static uint32_t  ADC_value  = 0;
static char      packet[20];
static uint8_t   packet_len = 0;

static uint32_t read_adc(void) {
	uint32_t value = 0;
	HAL_ADC_Start(&hadc1);
	if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK) {
		value = HAL_ADC_GetValue(&hadc1);
	}
	HAL_ADC_Stop(&hadc1);
	return value;
}

static void send_packet(void) {
	HAL_UART_Transmit(&huart2, (uint8_t *)packet, packet_len, 100);
}

/* read a new ADC value, build "!ADC=xxxx#", send it, start the timeout */
static void new_response(void) {
	ADC_value  = read_adc();
	packet_len = (uint8_t)sprintf(packet, "!ADC=%lu#\r\n", (unsigned long)ADC_value);
	send_packet();
	setTimer(TIMER_TIMEOUT, RESPONSE_TIMEOUT_MS);
}

void uart_communication_fsm(void) {
	switch (comm_state) {
	case COMM_WAIT_RST:
		if (command_flag == CMD_RST) {
			command_flag = CMD_NONE;
			new_response();
			comm_state = COMM_WAIT_OK;
		} else if (command_flag == CMD_OK) {
			command_flag = CMD_NONE;          /* stray !OK#: ignore */
		}
		break;

	case COMM_WAIT_OK:
		if (command_flag == CMD_OK) {         /* ACK received: done */
			command_flag = CMD_NONE;
			comm_state = COMM_WAIT_RST;
		} else if (command_flag == CMD_RST) { /* new request: new value */
			command_flag = CMD_NONE;
			new_response();
		} else if (isTimerExpired(TIMER_TIMEOUT)) {
			send_packet();                    /* resend the SAME packet */
			setTimer(TIMER_TIMEOUT, RESPONSE_TIMEOUT_MS);
		}
		break;

	default:
		comm_state = COMM_WAIT_RST;
		break;
	}
}
