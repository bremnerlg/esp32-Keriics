#ifndef LINENOISE_H_
#define LINENOISE_H_
#include "linenoise.h"
#endif

#ifndef ESP_ERR_H_
#define ESP_ERR_H_
#include "esp_err.h"
#endif

#ifndef ESP_LOG_H_
#define ESP_LOG_H_
#include "esp_log.h"
#endif

#include <string_view>
#include <algorithm>
#include <ranges>
#include <vector>
#include <cstdlib>
#include <array>

/* The set standard for this project is C++26 */
/* std::string_view is used to deal with the reality of stack-only C-style strings in a responsible way. */

namespace lds {



class Shell {
private:
	static constexpr uint8_t kArgLenMax = 4; // arg should never be more than 4 chars
	static constexpr uint8_t kArgCntMax = 16;
	static constexpr uint8_t kCmdSizeMax = 128;
	static constexpr const char kCmdDelim = ';';

	enum class kCmdCode {
		kENoParen, 
		kENoTerm,
		kEInvalHandle,
		kEOverflow,
		kGpioSetDirection,
		kGpioGetDirection,
		kGpioGetLevel,
		kGpioSetLevel,
		kGpioResetPin,
		kPoke
	};

	struct cmd_handle_t {
		const char *cmd_lit;
		kCmdCode code;
	};
				
	
	static constexpr cmd_handle_t kCmdHandles[] = {
		{ "gpio_set_direction", kCmdCode::kGpioSetDirection },
		{ "gpio_get_direction", kCmdCode::kGpioGetDirection },
		{ "gpio_set_level", kCmdCode::kGpioSetLevel },
		{ "gpio_get_level", kCmdCode::KGpioGetLevel },
		{ "gpio_reset_pin", kCmdCode::kGpioResetPin },
		{ "poke", kCmdCode::kPoke }
	};

	using return_codes = std::tuple<kCmdCode, esp_err_t>;
	

	// The execution will go 
	// 1. read into cmdBuffer, 2. Parse into expression, 3. Check if expression is valid, 4. Execute.
	/* The structure of the commands for this are going to be of form
		C_FUNCTION(ARG1, ARG2, ARG3)
	*/
	template <std::size_t arglen, std::size_t argcnt>
	struct expression {
		kCmdCode code;
		std::array<const char[arglen], argcnt> args;
	};

	std::array<const char, kCmdSizeMax> rawCmd; // parse raw buffer into expression structure
	expression<kArgLenMax, kArgCntMax> ex;


	// wrapper over raw esp function
	const char[] read_line(std::string_view prompt)
	{
		return linenoise(prompt);
	}

	template<std::size_t arglen, std::size_t argcnt>
	void parse_expression(expression<arglen, argcnt>& e)
	{
		e.code = DecodeCmd(rawCmd);

	}

	kCmdCode DecodeCmd(std::string_view raw)
	{
		// all commands required cfunction(); formatting. If none of these are found the
		// command already is wrong. Instead of waiting on the correct characters like
		// psql, we just reset the command so the user isn't wrestling with his mistakes.

		if (raw.find("(") == std::string_view::npos)
			return kCmdCode::kENoParen;
		if (raw.find(")") == std::string_view::npos)
			return kCmdCode::kENoParen;
		if (raw.find(";") == std::string_view::npos)
			return kCmdCode::kENoTerm;

		size_t size = std::size(arr);
		for (const std::string_view *p = kCmdLiterals, *end = arr + size;
			p != end; ++p) {
			if (raw == p) {
				

		
	}
		




public:
	Shell(std::string_view prompt)
	prompt(st::string_view prompt)
	{
		rawCmd = read_line(prompt);
		ex = 

	}
}

void main_loop()
{
	Shell LDS("> ");
	while(1) {
		LDS.prompt();
	}
}


}; // namespace lds
