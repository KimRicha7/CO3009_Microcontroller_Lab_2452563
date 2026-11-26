#include "command_parser.h"
#include <string.h>

/* Buffer written by the UART interrupt */
uint8_t          temp = 0;
uint8_t          buffer[MAX_BUFFER_SIZE];
volatile uint8_t index_buffer = 0;      /* write position (ISR)     */
volatile uint8_t buffer_flag  = 0;

/* Parser outputs */
uint8_t command_flag = CMD_NONE;
uint8_t command_data[MAX_CMD_SIZE];

/* Parser states */
typedef enum {
	PARSER_IDLE,          /* waiting for '!'                         */
	PARSER_RECEIVING      /* collecting characters until '#'         */
} ParserState;

static ParserState parser_state = PARSER_IDLE;
static uint8_t     read_index   = 0;     /* read position (main loop) */
static uint8_t     cmd_len      = 0;

static void finish_command(void) {
	command_data[cmd_len] = '\0';
	if (strcmp((char *)command_data, "RST") == 0) {
		command_flag = CMD_RST;
	} else if (strcmp((char *)command_data, "OK") == 0) {
		command_flag = CMD_OK;
	}
	/* any other text between ! and # is ignored */
}

void command_parser_fsm(void) {
	/* process every character received since the last call */
	while (read_index != index_buffer) {
		uint8_t c = buffer[read_index++];
		if (read_index >= MAX_BUFFER_SIZE) read_index = 0;

		switch (parser_state) {
		case PARSER_IDLE:
			if (c == '!') {
				cmd_len = 0;
				parser_state = PARSER_RECEIVING;
			}
			break;

		case PARSER_RECEIVING:
			if (c == '!') {                  /* new start: drop the old part */
				cmd_len = 0;
			} else if (c == '#') {           /* end of command */
				finish_command();
				parser_state = PARSER_IDLE;
			} else if (c < 0x20 || c > 0x7E) {
				/* skip control / non-ASCII bytes (e.g. from an input method) */
			} else if (cmd_len < MAX_CMD_SIZE - 1) {
				command_data[cmd_len++] = c;
			} else {                         /* too long: not a valid command */
				parser_state = PARSER_IDLE;
			}
			break;

		default:
			parser_state = PARSER_IDLE;
			break;
		}
	}
}
