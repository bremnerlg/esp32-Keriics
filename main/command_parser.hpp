#include <iostream>
#include <string_view>
#include <cstdint>
#include <array>
#include <algorithm>
#include <ranges>
#include <vector>

/* ad-hoc convention: on error return '-' then error number macro (e.g. return -ENULL) */
namespace lds {


enum class kCmdError {
	kSuccess, kNull, kCmd, kArg, kOutOfRange
};

const std::uint8_t kTokMax = 64;
const std::uint8_t kArgMax = 2; 
const std::uint8_t kEspCmdSz = 128; // allocated space for command names... probably won't ever exceed this many.
std::string_view kTokDelim = " \t\r\n\a";

using CmdIdx = std::int16_t;
using CmdError = std::int16_t;

/* The esp commands you can directly execute from the shell */


constexpr std::array<const char *, kEspCmdSz> kCmdStrings = {
	"gpio_set_mode",
	"gpio_set_level",
	"gpio_reset",
	"exit"
};

// These are to serve as indexes for the accepted functions. A command "code" is a value that is return which refers to the index of
// the command in the cmd string array
enum class kCmdCode {
	kGpioSetMode,
	kGpioSetLevel,
	kGpioReset,
	kExit
};






class HeapCmdExpression {
public:
	HeapCmdExpression(std::string_view line)
	{
		rawCmd = line;
		code = ParseLdsCmd(line);
		args = CollectLdsArgs(rawCmd);
		sz = line.size();
	}
private:
	std::string_view rawCmd;
	std::size_t sz;
	kCmdCode code; // turn string command into a code

	// went with string_view vec instead of const char* because they know their own size.
	// slight performance hit from this but better safety.
	// going dynamic on this to avoid overflows from excessive arg input.
	std::vector<std::string_view> args; 
	kCmdError error;

	kCmdCode ParseLdsCmd(const char *userin) // returns an int from the command enum found in command_parser.h
	{

	}
	TokenArray ParseLdsArgs(CmdIdx cmd, const char *arg);

	// stole some code from ashvardanian.com/posts. Thank you!
	std::vector<std::string_view> SplitTokensFromRaw(std::string_view rawCmd, std::string_view delims) // I'll just leave the delims subject to future change.
	{
		std::size_t pos = 0;
		while (pos < rawCmd.size()) {
			const std::iterator next_pos = std::find_first_of(delims, pos);
			args.push_back(rawCmd.substr(pos, next_pos - pos));
			pos = next_pos == std::string_view::npos ? str.size() : next_pos + 1;
	}
};


enum class kArgTypes {
	NUMERIC,
	FLAG
};



// parsing functions
const char* lds_read_line(const char *prompt); // wrapper for the esp32 linefeed
struct lds_expression *init_lds_expression(char *line);


// shell builtin commands
CmdError lds_execute(struct lds_expression *lds_cmd);
CmdError lds_gpio_set_level(struct token_array *args);
CmdError lds_gpio_reset(struct token_array *args);
CmdError lds_gpio_set_mode(struct token_array *args);
// cmd_error_t lds_gpio_set_mode(struct token_array * args);
CmdError lds_exit(void);

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

} // namespace lds