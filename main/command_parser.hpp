#include <iostream>
#include <string_view>
#include <array>
#include <algorithm>
#include <ranges>
#include <vector>
#include <cstdlib>
#include "linenoise.h"
#include "esp_err.h"
#include "esp_log.h"

/* The set standard for this project is C++26 */
/* std::string_view is used to deal with the reality of stack-only C-style strings in a responsible way. */

namespace lds {

/*
template <typename key_t, typename value_t, std::size_t N>
struct StaticMap {
	std::array<key_t, N> keys;
	std::array<value_t, N> values;
	std::size_t size = N;
}
*/
void main_loop()
{
	Shell LDS("> ");
	while(1) {
		LDS.prompt()
	}
}

class Shell {
private:
	constexpr uint8_t kArgLenMax = 4; // arg should never be more than 4 chars
	constexpr uint8_t kArgCntMax = 16;
	constexpr uint8_t kCmdSizeMax = 128;
	enum class cmd_code {
		kSetGpioMode, kSetGpioLevel, kResetGpioPin, kPoke
	};
	using return_codes = std::tuple<cmd_code cmd, esp_error_t err>;
	

	// The execution will go 
	// 1. read into cmdBuffer, 2. Parse into expression, 3. Check if expression is valid, 4. Execute.
	/* The structure of the commands for this are going to be of form
		C_FUNCTION(ARG1, ARG2, ARG3)
	*/
	template <std::size_t arglen, std::size_t argcnt>
	struct expression {
		return_codes codes;
		std::array<const char[arglen], argcnt> args;
	};

	std::array<const char, kCmdSizeMax> rawCmd; // parse raw buffer into expression structure
	expression<kArgLenMax, kArgCntMax> ex;


	// wrapper over raw esp function
	const char read_line(const std::array<const char, N>& pr)
	{
		return linenoise(prompt);
	}

	void parse_expression(expression& e)
	{
		e.return_codes = DecodeCmd(rawCmd);

	}

	template<std::size_t N>
	return_codes DecodeCmd(const std::array<const char, N>& raw)
	{
		uint8_t ws_cnt = 0;
		for (int i = 0; i < raw.size(); ++i) {
			switch(raw[i]) {

			}
		}
	}


public:
	Shell(std::string_view prompt)
	prompt(std::string_view prompt)
	{
		rawCmd = read_line(prompt);
		ex = 

	}
}
}; // namespace lds