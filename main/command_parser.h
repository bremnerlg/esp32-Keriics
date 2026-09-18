#include <string.h>

#define ENULL		1

/* ad-hoc convention: on error return '-' then error number macro (e.g. return -ENULL) */

enum LDS_COMMAND_CODES {
	SET_GPIO_OFF = 0,
	SET_GPIO_ON, 
	RESET_GPIO, 
	GET_STATE,
};	

struct lds_expression {
	lds_command_code_t cmd; // turn string command into a code
	char *args[];
	size_t cmdlen;
	size_t arg_count;
};	


enum LDS_ARG_CODES {
	NUMERIC,
	FLAG
};

typedef lds_command_code_t uint8_t;


int lloydos_shell();
lds_command_code_t parse_lloydos_cmd(char *userin); // returns an int from the command enum found in command_parser.h
lds_command_code_t parse_lloydos_cmd_arg(lds_command_code_t cmd, char *arg); // checks for valid args based on command.
