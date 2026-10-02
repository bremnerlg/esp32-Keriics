
#include <array>
#include <string>
#include <string_view>
#include <vector>
#include "stdint.h"

#define 	ENULL				1
#define		EINVALCMD			2
#define 	EINVALARG			3

#define 	LDS_TOK_BUFSZ		64
#define 	LDS_TOK_DELIM		" \t\r\n\a"
#define		LDS_CMD_SIZE		6
#define 	ARG_MAX				2
/* ad-hoc convention: on error return '-' then error number macro (e.g. return -ENULL) */


typedef int16_t cmd_idx_t;
typedef int16_t cmd_error_t;

using token_array = std::vector<std::string>;
enum LDS_COMMAND_CODES {
	LDS_GPIO_SET_MODE,
	LDS_GPIO_SET_LEVEL,
	LDS_RESET_GPIO, 
	LDS_GPIO_GET_LEVEL,
};	

constexpr std::array<std::string_view, 4> LDS_BUILTIN_CMDS = {
	"gpio_set_mode",
	"gpio_set_level",
	"gpio_reset",
	"exit"
	//"print",
	//"jump",
	//"write"
};


struct lds_expression {
	cmd_idx_t cmd; // turn string command into a code
	token_array args;
	cmd_error_t error; // 0>= for fine, negative for bad
};	


enum LDS_ARG_CODES {
	NUMERIC,
	FLAG
};



// parsing functions
char* lds_read_line(char *prompt); // wrapper for the esp32 linefeed
struct lds_expression *init_lds_expression(char *line);

cmd_idx_t parse_lds_cmd(char *userin); // returns an int from the command enum found in command_parser.h
cmd_idx_t collect_lds_args(cmd_idx_t cmd, char *arg);
uint8_t is_digit_arg(char *s, size_t len);

// shell builtin commands
cmd_error_t lds_execute(struct lds_expression *lds_cmd);
cmd_error_t lds_gpio_set_level(struct token_array *args);
cmd_error_t lds_gpio_reset(struct token_array *args);
cmd_error_t lds_gpio_set_mode(struct token_array *args);
// cmd_error_t lds_gpio_set_mode(struct token_array * args);
cmd_error_t lds_exit(void);

/*
For later use... Right now haven't enough functions to justify this array.
cmd_error_t (*builtin_func[]) (struct token_array *) = {
	&lds_gpio_set_mode,
	&lds_gpio_set_level,
	&lds_gpio_reset,
	&lds_exit
	// &lds_print,
	// &lds_jump,
	// &lds_write
};
*/
void lds_loop(void);
