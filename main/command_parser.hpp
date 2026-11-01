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
#include <exception>
#include <functional>





namespace lds {

// @brief Packs two arrays of equal size into one structure.
// This exists so you can use the handles of the Cmds as
// keys for the actual pointers to said Cmds.
// @param k: Function key array to index values.
// @param v: values to be accessed by [] indexing.
// @param s: size that BOTH arrays must be.
// @return StaticMap
template<typename T1, typename T2, std::size_t s>
struct StaticMap {
	StaticMap(T1 k, T2 v) :
	{
		// for the lds specific stuff, I'm going to try to stick to exceptions.
		if (k.size() != v.size() || k.size() == 0 || v.size() == 0) {
			throw std::invalid_argument(); 
		} else {
			std::copy(k.begin(), k.end(), key.begin());
			std::copy(v.begin(), v.end(), value.begin());
		} 


	}
	operator[]	
	std::array<T1, s> key;
	std::array<T2, s> value;
};

// @brief Allows association with an object by cstr.
template<typename T>
struct CstrValuePair {
		KeyValuePair(const char *s, const T& ref)
		: s(s) 
		, objectRef(ref)
		{}

		const char* str;
		T& objectRef;
};


// @brief Construct a lookup table for values using cstr associated with them.
// size must be known at compile time.
// @param std::array<CstrValuePair, sz> pairs: Used to create hashes for easy lookup
template<typename T, std::size_t sz>
class CstrLookupTable {
public:
	CstrLookupTable(const std::array<CstrValuePair<T>, sz>& pairs)
	{
		for (int i = 0; i < pairs.size(); i++)
			hashes[i] = std::hash<const char*>()(pairs.str);
	}
	const T& operator[](const char* cstr) 
	{
		check(cstr); // TODO: throws if doesn't exist, make sure this is handled somewhere.
		const std::size_t lookup_hash = std::hash<const char *>()(t cstr);
		const std::HashArray::iterator index = std::find(hashes.begin(), hashes.end(), lookup_hash);
		if (std::find(hashes.begin(), hashes.end(), lookup_hash))

	}
private:
	using HashArray = std::array<size_t, sz>;
	HashArray hashes;
	void check(const char* cstr) // throws if cstr doesn't exist
	{
		if
	}
};



// @param addr: address you wish to overwrite
// @param value: the value you wish to write into addr.
void poke(void *addr, uint16_t value)
{
	if (addr == nullptr)
		return ESP_ERR_INVALID_ARG;

	*addr = value;
	if (*addr == value)
		return ESP_OK;
	else
		return ESP_FAIL;
}

class Cmd {
private:
 
	/* TYPES */
	/* ------------------------------------------------------------- */
	/* ------------------------------------------------------------- */
	static constexpr uint8_t kArgLenMax = 4;
	static constexpr uint8_t kArgCntMax = 16;
	static constexpr uint8_t kCmdSizeMax = 128;
	static constexpr const char kCmdDelim = ';';

	// @brief these a functions that simply take a representation of a location
	// in memory and write a value to it. Poke is the most basic form, but another
	// good example is taking the gpio number, which esp then converts into a memory address
	// and writing your state (0 or 1) into it.
	using std::function<void(std::uint16_t, std::uint16_t)> = write_addr_f;

	static std::variant<addr_write_f> cli_funcs;
	// @brief lds functions wrap esp functions and force them to assume two 16-bit words
	// as arguments. This simplifies aggregating the functions under one data structure.
	template <std::size_t sz>
	using FuncArray = std::array<cli_funcs, sz>;

	/* ------------------------------------------------------------- */
	/* ------------------------------------------------------------- */

	// usually esp GPIO functions can be simplified to pin # as arg
	// then some value to use for the operation (e.g. gpio_set_level(1, 0))
	// going to attempt to make function pointers indexable by an array of std::variant
	static void gpioSetMode(std::uint16_t pin, std::uint16_t mode);
	static void gpioSetLevel(std::uint16_t pin, std::uint_16t level);
	static void poke(std::uint16_t addr, std::uint16_t val);



	// TODO: ugly declaration, will  clean later
	// This is supposed to be the lookup table of the CMDS
	// indexable by the CmdHandles
	static constexpr StaticMap<CmdHandleArray, 
					FuncArray, 
					kCmdHandles.size()> CmdMap(kCmdHandles, kCmdFuncs);





	static constexpr CmdHandleArray kCmdHandles = {
			std::string_view{"gpioSetMode"},
			std::string_view{"gpioSetLevel"},
			std::string_view{"poke"}
	};

	static constexpr FuncArray kCmdFuncs = {
			&gpioSetMode,
			&gpioSetLevel,
			&poke
	};

	std::array<uint16_t, kArgCntMax> args;

	kCmdCode code;

public:
	Cmd(std::string_view handle, std::array<constchar) // keep most of our state changes within this and the change_state function
	:
	{
		code = CodeFromHandle(handle);	
		
	}	

};




// @brief The job of the shell is to take commands from the user input,
// parse them, validate, then call the appropriate LDS command with arguments
// given in parentheses in c_style_syntax();
// @param prompt: the indicator that the user will be greeted with to enter a command
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

		if (rawsz >= sz)
			return bc_rawline;
		else
			throw std::length_error("Input too long... aborting.");
		
		for (int i = 0; i < rawsz; i++)
			bc_rawline[i] = raw[i];

		return bc_rawline;
	}

	esp_err_t execute(const Cmd& cmd);
	const char* prompt;	
	std::array<const char, cmd::KCmdSizeMax> raw_cmd;
	Cmd current_cmd;

	bool exit;
public:
	Shell(const char* p)
	:
	{
		err = ESP_OK;	
		exit = false;

		prompt = p;
	}



	esp_err_t main(std::string_view prompt)
	{
		raw_cmd = readline(prompt);
		current_cmd = Cmd(prompt);

		if (rawcmd.size() > 0) {
			err = execute(cmd);		
		} else {
			throw std::
		}
		
		
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
