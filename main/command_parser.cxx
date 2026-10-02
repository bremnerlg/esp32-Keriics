#include "command_parser.h"
#include "esp_log.h"
#include "ctype.h"
#include "linenoise/linenoise.h"
#include "stdlib.h"
#include "driver/gpio.h"
#include "stdio.h"
#include <string>
#include <iostream>

// string goes into parse_lloydos_command -> command code comes out to determine how to interpret the next expression.
// Return an error if the command isn't valid.
std::string lds_read_line(const std::string& prompt)
{
	const char* input = linenoise(prompt);
	return std::string(linenoise(prompt) != nullptr) ? ;
}

cmd_idx_t parse_lds_cmd(const std::string& line)
{

	for (int i = 0; i < LDS_CMD_SIZE; ++i) { // compare it against the builtins defined in command_parser.h
		valid_cmd_len = strlen(&LDS_BUILTIN[0][i]);
		if (strncmp(line, &LDS_BUILTIN[0][i], 
			cmd_len < valid_cmd_len ? cmd_len : valid_cmd_len) == 0)
			return i; // produce a number to index the supported command to be interpretted
	}
	return -EINVALCMD; // if not, then just return an error value and tell the user they goofed.
}

struct token_array *lds_split_line(char *line) // stolen from the brennan.io tutorial
{
	std::uint16_t bufsize = LDS_TOK_BUFSZ, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens) {
		fprintf(stderr, "lds: allocation error\n");
		exit(EXIT_FAILURE);
	}

	token = strtok(NULL, LDS_TOK_DELIM);
	while(token != NULL) {
		tokens[position] = token;
		position++;

		if (position >= bufsize) {
			bufsize += LDS_TOK_BUFSZ;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens) {
				fprintf(stderr, "lds: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}

	tokens[position] = NULL;

	struct token_array *new_array = malloc(sizeof(struct token_array));
	new_array->tokens = tokens;
	new_array->len = position;
	return new_array;
}



struct lds_expression *init_lds_expression(char *line)
{
	struct lds_expression *new_lds = malloc(sizeof(struct lds_expression));

	if (line == NULL) {
		new_lds->error = -ENULL;
		return new_lds;
	}
	cmd_idx_t cmd_idx = parse_lds_cmd(line);
	struct token_array *args = lds_split_line(line);
	uint16_t cmd_len = strlen(line);

	
	new_lds->cmd = cmd_idx;
	new_lds->args = args;
	new_lds->cmdlen = cmd_len;
	return new_lds;
}

uint8_t is_digit_arg(char *s, size_t len)
{
	for (int i = 0; i < len; ++i) {
		if (!isdigit(s[i]))
			return 0;
	}
	return 1;
}



cmd_error_t lds_execute(struct lds_expression *lds_cmd)
{
	/* errors ultimately handled in the execution function. */
	if (lds_cmd->error != 0) {
		return lds_cmd->error;
	} 

	switch(lds_cmd->cmd) {
		case LDS_GPIO_SET_MODE:
			ESP_LOGI(__func__, "lds_gpio_set_mode(): called\n");
			return lds_gpio_reset(lds_cmd->args);
		case LDS_GPIO_SET_LEVEL:
			ESP_LOGI(__func__, "lds_gpio_set_mode(): called\n");
			return lds_gpio_set_mode(lds_cmd->args);
		case LDS_RESET_GPIO:
			ESP_LOGI(__func__, "lds_gpio_reset(): called\n");
			return lds_gpio_reset(lds_cmd->args);
		case LDS_GPIO_GET_LEVEL:
			ESP_LOGI(__func__, "lds_gpio_get_mode(): called\n");
			return lds_gpio_get_mode(lds_cmd->args);
		default:
			ESP_LOGE(__func__, "CRITICAL: INVALID COMMAND CODE \"%d\" exiting...\n");
			exit(EXIT_FAILURE);
	}
}

cmd_error_t lds_gpio_set_mode(struct token_array *args)
{
	long pin;
	char *mode;
	char *endptr;

	pin = strtol(args->tokens[0], &endptr, 10); // expect the user to enter base10....
	if (endptr == args->tokens[0])
		return -EINVALARG;
	
	strcpy(mode, args->tokens[1]);
	if (strcmp(mode, "disable") == 0)
		gpio_set_direction(pin, GPIO_MODE_DISABLE);
	else if (strcmp(mode, "input") == 0)
		gpio_set_direction(pin, GPIO_MODE_INPUT);
	else if (strcmp(mode, "output") == 0)
		gpio_set_direction(pin, GPIO_MODE_OUTPUT);
	else
		return -EINVALARG;

	return 0;
}

cmd_error_t lds_gpio_set_level(struct token_array *args)
{
	long pin, level;
	char *endptr;

	pin = strtol(args->tokens[0], &endptr, 10);
	if (endptr == args->tokens[0])
		return -EINVALARG;

	level = strtol(args->tokens[1], &endptr, 10);
	if (endptr == args->tokens[1])
		return -EINVALARG;

	gpio_set_level(pin, level);	
	return 0;
}

void lds_loop(void)
{
	char *line;
	cmd_error_t status;
	struct lds_expression *lds_eval;

	printf("Current Command Library\n");
	printf("gpio_set_mode [PIN] [MODE]\n");
	printf("gpio_set_mode [PIN] [LEVEL]\n");
	printf("gpio_reset [PIN]\n");
	printf("exit\n"); 
	do {
		line = lds_read_line("> ");
		lds_eval = init_lds_expression(line); // groups all of the parsing functions into one main expression parser

		status = lds_execute(lds_eval); // for now, all critical errors are handled inside of this function.

		if (status == -EINVALCMD)

		switch(status) {
			case -EINVALCMD:
				fprintf(stderr,"COMMAND NOT RECOGNIZED: %s\n", line);
				break;
			case -ENULL:
				fprintf(stderr,"CRITICAL: INPUT STRING EVALUATED TO NULL, CHECK ESP LOG. EXITING SHELL...\n");
				exit(EXIT_FAILURE);
			default: // hopefully this branch never executes...
				fprintf(stderr,"CRITICAL: UNKNOWN ERROR, CHECK ESP LOG. EXITING SHELL...\n"); 
				exit(EXIT_FAILURE);
		}

		free(lds_eval);
	} while (status > 0);
}
