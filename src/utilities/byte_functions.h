#pragma once
#include "memory_leak_detector.h"
#include <string>
#include <bitset>

template<typename T>
inline void copyDataIntoCharArray(T& t_data, char* t_array, int t_index) {
	int data_size = sizeof(t_data);
	char* data_ref = static_cast<char*>(static_cast<void*>(&t_data));
	for (int i = 0; i < data_size; ++i) {
		t_array[i + t_index] = data_ref[i];
	}
}

// Warning! Bad standard but may be used if there is no other way
template<typename T_src, typename T_dest>
inline T_dest* castPtr(T_src* ptr) {
	return static_cast<T_dest*>(static_cast<void*>(ptr));
}

template<typename T>
inline void extractDataFromCharArray(T& t_data, char* t_array, int t_index) {
	t_data = *static_cast<T*>(static_cast<void*>(&t_array[t_index]));
}

template<typename T>
inline T* getPtrToCharArray(char* t_array, int t_index) {
	return static_cast<T*>(static_cast<void*>(&t_array[t_index]));
}

inline std::string charToBinaryString(char t_char) {
	return std::bitset<8>(t_char).to_string();
}

inline std::string charArrayToBinaryString(char* char_array, unsigned __int64 size) {
	std::string return_string = "";
	return_string.reserve(size*8);
	for (int i = 0; i < size; ++i) {
		return_string.append(charToBinaryString(char_array[i]));
	}
	return return_string;
}

inline std::string charArrayToBinaryStringWithSpaces(char* char_array, unsigned __int64 size) {
	std::string return_string = "";
	return_string.reserve(size * 9);
	for (int i = 0; i < size; ++i) {
		return_string.append(charToBinaryString(char_array[i]) + " ");
	}
	return return_string;
}

