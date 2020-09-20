#pragma once
#include "memory_leak_detector.h"
#include <vector>
#include <string>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <chrono>
#include <fstream>
#include <sstream>
#include <vector>
#include <iostream>

static inline std::vector<std::string> split(std::string strToSplit, char delimeter) {
	std::stringstream ss(strToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		if (item != "") {
			splittedStrings.push_back(item);
		}
	}
	return splittedStrings;
}

//like split, but splits on all delimiters, therefore having "" if there are two delimiters next to each other
static inline std::vector<std::string> splitAll(std::string strToSplit, char delimeter) {
	std::stringstream ss(strToSplit);
	std::string item;
	std::vector<std::string> splittedStrings;
	while (std::getline(ss, item, delimeter))
	{
		splittedStrings.push_back(item);
	}
	return splittedStrings;
}

inline void substitute(std::string* text, std::string f, std::string r) {
	size_t index = 0;
	while (true) {
		index = text->find(f, index);
		if (index == std::string::npos) break;
		text->replace(index, f.size(), r);
		index += r.size();
	}
}

// trim from start (in place)
static inline void ltrim(std::string& s) {
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](int ch) {
		return !std::isspace(ch);
		}));
}

// trim from end (in place)
static inline void rtrim(std::string& s) {
	s.erase(std::find_if(s.rbegin(), s.rend(), [](int ch) {
		return !std::isspace(ch);
		}).base(), s.end());
}

// trim from both ends (in place)
static inline void trim(std::string& s) {
	ltrim(s);
	rtrim(s);
}

// trim from start (copying)
static inline std::string ltrimCopy(std::string s) {
	ltrim(s);
	return s;
}

// trim from end (copying)
static inline std::string rtrimCopy(std::string s) {
	rtrim(s);
	return s;
}

// trim from both ends (copying)
static inline std::string trimCopy(std::string s) {
	trim(s);
	return s;
}

static inline std::string removeSpaces(std::string str) {
	str.erase(remove(str.begin(), str.end(), ' '), str.end());
	return str;
}

static inline std::string getFile(std::string name) {
	std::string line;
	std::string output;
	std::ifstream myfile(name);
	if (myfile.is_open()) {
		while (getline(myfile, line)) {
			output += line + '\n';
		}
		myfile.close();
	}

	else std::cout << "Unable to open file";
	return output;
}


static inline bool isDigits(std::string str) {
	return str.find_first_not_of("0123456789") == std::string::npos;
}

static inline bool isNumber(std::string str) {
	if (std::count(str.begin(), str.end(), '.') < 2) {
		if (str[0] == '-') {
			std::string number = str.substr(1, str.size());
			return number.find_first_not_of("0123456789.") == std::string::npos;
		}
		return str.find_first_not_of("0123456789.") == std::string::npos;
	}
	return false;
}

//get if string is a valid variable name
static inline bool isVariable(std::string str) {
	std::string digit = "0123456789";
	if (digit.find(str[0]) == std::string::npos) {
		return str.find_first_not_of("0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ_") == std::string::npos;
	}
	return false;
}