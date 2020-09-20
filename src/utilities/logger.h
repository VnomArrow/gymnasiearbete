#pragma once
#include "memory_leak_detector.h"
#include "mutex.h"
#include <string>
#include <iostream>
#include <chrono>
#include <ctime>
#include <sstream>
#include <vector>
#include <tuple>
#include <intrin.h>

/*
description:
simple logger
 - supports threads
 - supports different priorities
 - supports colored text
 - saves logs to file
 - keeps track of line and path where it was called
 - supports multiple arguments

implement:
call the logger::init() at the beginning of your program

usage:
use log__<type>(args) to log things, there are multiple message types to choose from(it supports multiple arguments).
*/

template<typename T>
std::string toString(T val) {
	std::string return_val;
	return_val += std::to_string(val);
	return_val += ", ";
	return return_val;
}
template<typename T>
std::string toString(T* val) {
	std::string return_val;
	std::ostringstream adress;
	adress << (void const*)val;
	return_val += adress.str();
	return_val += ", ";
	return return_val;
}
template<>
inline std::string toString(const char* val) {
	std::string return_val;
	return_val += val;
	return_val += ", ";
	return return_val;
}
template<>
inline std::string toString(std::string val) {
	std::string return_val;
	return_val += val;
	return_val += ", ";
	return return_val;
}
template<typename T>
std::string toString(std::vector<T> val) {
	std::string return_val;
	return_val += "{";
	for (unsigned int i = 0; i < val.size(); ++i) {
		return_val += toString(val[i]);
	}
	return_val = return_val.substr(0, return_val.size() - 2);
	return_val += "}, ";
	return return_val;
}
template<size_t I = 0, typename... Tp>
std::string toString(std::tuple<Tp...> & t) {
	std::string return_val;
	return_val += toString(std::get<I>(t));

#pragma warning(push)
#pragma warning(disable: 4984)
	if constexpr (I + 1 != sizeof...(Tp))
#pragma warning(pop)
		return_val += toString<I + 1>(t);
	return return_val;
}

template<size_t I = 0, typename... Tp>
std::string tupleToString(std::tuple<Tp...> & t) {
	std::string return_val;
	return_val += toString(std::get<I>(t));
#pragma warning(push)
#pragma warning(disable: 4984)
	if constexpr (I + 1 != sizeof...(Tp))
#pragma warning(pop)
		return_val += tupleToString<I + 1>(t);

	else
		return_val = return_val.substr(0, return_val.size() - 2);
	return return_val;
}

template<size_t I = 0, typename... Tp>
std::string tupleToStringNotSplit(std::tuple<Tp...> & t) {
	std::string return_val;
	return_val += toString(std::get<I>(t));
	return_val = return_val.substr(0, return_val.size() - 2);
#pragma warning(push)
#pragma warning(disable: 4984)
	if constexpr (I + 1 != sizeof...(Tp))
#pragma warning(pop)
		return_val += tupleToStringNotSplit<I + 1>(t);
	return return_val;
}


namespace logger {
	//(message, path, message type)
	void log(std::string message, std::string path, long line, unsigned __int16 color, const char message_type[12]);
	void init();
}

namespace console_color_ {
	const unsigned __int16 BLACK = 0x0000;
	const unsigned __int16 BLUE = 0x0001;
	const unsigned __int16 GREEN = 0x0002;
	const unsigned __int16 TURQUOISE = GREEN | BLUE;
	const unsigned __int16 RED = 0x0004;
	const unsigned __int16 PURPLE = RED | BLUE;
	const unsigned __int16 YELLOW = RED | GREEN;
	const unsigned __int16 LIGHT_GREY = RED | GREEN | BLUE;
	const unsigned __int16 BRIGHT = 0x0008;
	const unsigned __int16 DARK_GREY = BRIGHT;
	const unsigned __int16 LIGHT_BLUE = BRIGHT | BLUE;
	const unsigned __int16 LIGHT_GREEN = BRIGHT | GREEN;
	const unsigned __int16 LIGHT_TURQUOISE = BRIGHT | TURQUOISE;
	const unsigned __int16 LIGHT_RED = BRIGHT | RED;
	const unsigned __int16 LIGHT_PURPLE = BRIGHT | PURPLE;
	const unsigned __int16 LIGHT_YELLOW = BRIGHT | YELLOW;
	const unsigned __int16 WHITE = BRIGHT | LIGHT_GREY;
}

#define logger__verbosity__max 100
#define logger__verbosity__mute 0
#define logger__verbosity__fatal_error 10
#define logger__verbosity__error 20
#define logger__verbosity__breakpoint 30
#define logger__verbosity__warning 40
#define logger__verbosity__debug 50
#define logger__verbosity__v 60   // Very important info
#define logger__verbosity__vv 70  // Moderetly important info
#define logger__verbosity__vvv 80 // Not very important info



#pragma warning(push)
#pragma warning(disable: 4005)
// Shows how verbose the logger should be
#define logger__verbosity logger__verbosity__vv
#pragma warning(pop)

#define log__(VERBOSITY_LEVEL, CONSOLE_COLOR, MESSAGE_TYPE, ...) { \
if (logger__verbosity >= VERBOSITY_LEVEL) { \
	auto MESSAGE = std::make_tuple(__VA_ARGS__); \
	logger::log(tupleToStringNotSplit(MESSAGE), __FILE__, __LINE__, CONSOLE_COLOR, MESSAGE_TYPE); }}

#define log__error(...) log__(logger__verbosity__error, console_color_::LIGHT_RED,				"ERROR:      ", __VA_ARGS__)
#define log__warning(...) log__(logger__verbosity__warning, console_color_::LIGHT_YELLOW,		"WARNING:    ", __VA_ARGS__)
#define log__debug(...) log__(logger__verbosity__debug, console_color_::LIGHT_TURQUOISE,		"DEBUG:      ", __VA_ARGS__)
#define log__v(...) log__(logger__verbosity__v, console_color_::BLUE,							"INFO:       ", __VA_ARGS__)
#define log__vv(...) log__(logger__verbosity__vv, console_color_::BLUE,							"INFO:       ", __VA_ARGS__)
#define log__vvv(...) log__(logger__verbosity__vvv, console_color_::BLUE,						"INFO:       ", __VA_ARGS__)

#define log__fatal_error(...){ \
if (logger__verbosity >= logger__verbosity__fatal_error) { \
	auto MESSAGE = std::make_tuple(__VA_ARGS__); \
	logger::log(tupleToStringNotSplit(MESSAGE), __FILE__, __LINE__, console_color_::LIGHT_RED,  "FATAL ERROR:"); \
__debugbreak();}}

#define log__breakpoint(...){ \
if (logger__verbosity >= logger__verbosity__breakpoint) { \
	auto MESSAGE = std::make_tuple(__VA_ARGS__); \
	logger::log(tupleToStringNotSplit(MESSAGE), __FILE__, __LINE__, console_color_::LIGHT_YELLOW,"BREAKPOINT: "); \
__debugbreak();}}