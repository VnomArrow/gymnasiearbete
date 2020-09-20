#pragma once
#include "byte_functions.h"
#include "logger.h"
#include <vector>

class CharArrayStorage {
public:
	CharArrayStorage(int t_size) {
		m_max_size = t_size;
		m_storage_array = new char[t_size];
		m_extract_index = 0;
	}
	~CharArrayStorage() {
		delete(m_storage_array);
	}
	template<typename T>
	inline void insertData(T data, int t_size = sizeof(T)) {
		int prev_storage_size = used_storage_size;
		used_storage_size += t_size;
		if (used_storage_size <= m_max_size) {
			T* data_ptr = castPtr<char, T>(&m_storage_array[prev_storage_size]);
			*data_ptr = data;
			return;
		}
		throw_error_log("CharArrayStorage::insertData() storage outside range");
	}
	template<typename T>
	inline int extractData(T& data_holder, int t_size = sizeof(T)) {
		if (getUnExtractedStorageSize() >= t_size) {
			T* data_ptr = castPtr<char, T>(&m_storage_array[m_extract_index]);
			data_holder = *data_ptr;
			m_extract_index += t_size;
			return 0;
		}
		return -1;
	}
	inline int getUnusedStorageSize() {
		return m_max_size - used_storage_size;
	}
	inline int getUsedStorageSize() {
		return used_storage_size;
	}
	inline int getUnExtractedStorageSize() {
		return used_storage_size - m_extract_index;
	}
	inline int getExtractIndex() {
		return m_extract_index;
	}
	inline int setExtractIndex(int t_size) {
		if (t_size < used_storage_size) {
			m_extract_index = t_size;
			return 0;
		}
		return -1;
	}
	inline void resetStorage(int up_to_storage_index = -1) {
		if (up_to_storage_index == -1) {
			used_storage_size = 0;
			m_extract_index = 0;
		}
		else if(up_to_storage_index > 0) {
			int new_size = used_storage_size - up_to_storage_index;
			for (int i = 0; i < new_size; ++i) {
				m_storage_array[i] = m_storage_array[i + up_to_storage_index];
			}
			used_storage_size = new_size;
			m_extract_index = 0;
		}
		else if(up_to_storage_index != 0) {
			throw_error_log("CharArrayStorage::resetStorage got negative array index");
		}
	}
	inline int insertArray(char* array_ptr, int t_size) {
		int new_used_size = used_storage_size + t_size;
		if (new_used_size <= m_max_size) {
			int start_copy_index = used_storage_size;
			used_storage_size = new_used_size;
			for (int i = 0; i < t_size; ++i) {
				m_storage_array[start_copy_index + i] = array_ptr[i];
			}
			return 0;
		}
		return -1;
	}
	inline char* getStoragePtr() { return &m_storage_array[0]; }
	inline char* getUnusedStoragePtr() { return &m_storage_array[used_storage_size]; }
	inline int setUsedStorageSize(unsigned int t_size) {
		if (t_size <= m_max_size) {
			used_storage_size = t_size;
			return 0;
		}
		return -1;
	}
private:
	int m_max_size;
	int m_extract_index;
	char* m_storage_array;
	int used_storage_size;
};