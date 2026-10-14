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


class Cmd {
private:
	enum class kCmdCode {
		kENoParen, 
		kENoTerm,
		kEInvalHandle,
		kGpioSetDirection,
		kGpioGetDirection,
		kGpioGetLevel,
		kGpioSetLevel,
		kGpioResetPin,
		kPoke
	};
	
	static constexpr uint8_t kArgLenMax = 4;
	static constexpr uint8_t kArgCntMax = 16;
	static constexpr uint8_t kCmdSizeMax = 128;
	static constexpr const char kCmdDelim = ';';

	static constexpr std::array kCmdHandles = {
		std::string_view{"gpio_set_direction"},
		std::string_view{"gpio_get_direction"},
		std::string_view{"gpio_set_level"},
		std::string_view{"gpio_get_level"},
		std::string_view{"gpio_reset_pin"},
		std::string_view{"poke"}
	};

	kCmdCode CodeFromHandle(std::string_view raw)
	{
	}

	kCmdCode code;
	std::array<const char[arglen], argcnt> args;

public:
	Cmd(std::string_view handle, std::array<constchar) // keep most of our state changes within this and the change_state function
	:
	{
		code = CodeFromHandle(handle);	
		
	}	

};


class Shell {
private:
	// The execution will go 
	// 1. read into cmdBuffer, 2. Parse into Cmd, 4. Execute.
	/* The structure of the commands for this are going to be of form
		C_FUNCTION(ARG1, ARG2, ARG3)
	*/

	// wrapper over raw esp function
	template<size_t sz>
	std::array<const char, sz> read_line(std::string_view prompt)
	{
		const char raw* = linenoise(prompt); // linenoise is already heap allocated so we can just take a pointer to it.

		std::array<const char, rawsz> bc_rawline;

		if (rawsz > sz)
			return bc_rawline;

		for (int i = 0; i < rawsz; i++)
			bc_rawline[i] = raw[i];

		return bc_rawline;
	}

	esp_err_t execute(const Cmd& cmd);
	
	std::array<const char, cmd::KCmdSizeMax> raw_cmd;
	Cmd current_cmd;
	esp_err_t err;
	bool exit;
public:
	Shell(std::string_view prompt)
	:
	{
		err = ESP_OK;	
		exit = false;

		while (err == ESP_OK && !quit) {
			err = main(prompt);
		}
	}



	esp_err_t main(std::string_view prompt)
	{
		raw_cmd = readline(prompt);
		current_cmd = Cmd(prompt);

		if (rawcmd.size() > 0) {
			err = execute(cmd);		
		} else {
			err = ESP_ERR_INVALID_SIZE;
			return err;
		}
		
		return err;
		
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
