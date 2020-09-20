#pragma once
#include "memory_leak_detector.h"
#include "network_sockets.h"
#include "mutex.h"
#include <sstream>


namespace data_transfer_ {
	const int SENDER_BUFFER_SIZE = 256 * 128;
	const int RECIEVER_BUFFER_SIZE = 256 * 128;

	struct DataTransferer {
		virtual bool transferData(double& value_holder) = 0;
		virtual bool transferData(float& value_holder) = 0;
		virtual bool transferData(bool& value_holder) = 0;

		virtual bool transferData(__int8& value_holder) = 0;
		virtual bool transferData(__int16& value_holder) = 0;
		virtual bool transferData(__int32& value_holder) = 0;
		virtual bool transferData(__int64& value_holder) = 0;

		virtual bool transferData(unsigned __int8& value_holder) = 0;
		virtual bool transferData(unsigned __int16& value_holder) = 0;
		virtual bool transferData(unsigned __int32& value_holder) = 0;
		virtual bool transferData(unsigned __int64& value_holder) = 0;
	};


	// Template class for recieving data
	struct DataReciever : public DataTransferer {
		virtual void destructor() = 0;
		bool transferData(double& value_holder) { return(readData(value_holder)); }
		bool transferData(float& value_holder) { return(readData(value_holder)); }
		bool transferData(bool& value_holder) { return(readData(value_holder)); }

		bool transferData(__int8& value_holder) { return(readData(value_holder)); }
		bool transferData(__int16& value_holder) { return(readData(value_holder)); }
		bool transferData(__int32& value_holder) { return(readData(value_holder)); }
		bool transferData(__int64& value_holder) { return(readData(value_holder)); }

		bool transferData(unsigned __int8& value_holder) { return(readData(value_holder)); }
		bool transferData(unsigned __int16& value_holder) { return(readData(value_holder)); }
		bool transferData(unsigned __int32& value_holder) { return(readData(value_holder)); }
		bool transferData(unsigned __int64& value_holder) { return(readData(value_holder)); }

		template<typename T>
		bool readData(T& value_holder);
		void copyUnreadDataToStartOfBuffer(); // Used to make space for new data
		virtual bool fetchData() = 0;
		virtual bool canFetchData(SocketStatusChecker* status_checker) = 0;

		char buffer[RECIEVER_BUFFER_SIZE] = {};
		int used_buffer_size = 0;
		int read_buffer_index = 0;
		inline int getUnreadDataSize() { return used_buffer_size - read_buffer_index; }
		inline int getUnusedDataSize() { return RECIEVER_BUFFER_SIZE - used_buffer_size; }
	protected:
		// Call destructor() instead to make sure child class is destructed
		~DataReciever() {}
	};


	// Must speciefy all types that will be used
	template bool DataReciever::readData<double>(double&);
	template bool DataReciever::readData<float>(float&);
	template bool DataReciever::readData<bool>(bool&);

	template bool DataReciever::readData<__int8>(__int8&);
	template bool DataReciever::readData<__int16>(__int16&);
	template bool DataReciever::readData<__int32>(__int32&);
	template bool DataReciever::readData<__int64>(__int64&);

	template bool DataReciever::readData<unsigned __int8>(unsigned __int8&);
	template bool DataReciever::readData<unsigned __int16>(unsigned __int16&);
	template bool DataReciever::readData<unsigned __int32>(unsigned __int32&);
	template bool DataReciever::readData<unsigned __int64>(unsigned __int64&);


	// Template class for sending data
	struct DataSender : public DataTransferer {
		virtual void destructor() = 0;
		bool transferData(double& value_holder) { return(inputData(value_holder)); }
		bool transferData(float& value_holder) { return(inputData(value_holder)); }
		bool transferData(bool& value_holder) { return(inputData(value_holder)); }

		bool transferData(__int8& value_holder) { return(inputData(value_holder)); }
		bool transferData(__int16& value_holder) { return(inputData(value_holder)); }
		bool transferData(__int32& value_holder) { return(inputData(value_holder)); }
		bool transferData(__int64& value_holder) { return(inputData(value_holder)); }

		bool transferData(unsigned __int8& value_holder) { return(inputData(value_holder)); }
		bool transferData(unsigned __int16& value_holder) { return(inputData(value_holder)); }
		bool transferData(unsigned __int32& value_holder) { return(inputData(value_holder)); }
		bool transferData(unsigned __int64& value_holder) { return(inputData(value_holder)); }

		template<typename T>
		bool inputData(T data);
		virtual bool sendData() = 0;
		virtual bool canSendData(SocketStatusChecker* status_checker) = 0;

		char buffer[SENDER_BUFFER_SIZE] = {};
		int used_buffer_size = 0;
		inline int getUnusedDataSize() { return SENDER_BUFFER_SIZE - used_buffer_size; }
	protected:
		// Call destructor() instead to make sure child class is destructed
		~DataSender(){}
	};


	// Must speciefy all types that will be used
	template bool DataSender::inputData<double>(double);
	template bool DataSender::inputData<float>(float);
	template bool DataSender::inputData<bool>(bool);

	template bool DataSender::inputData<char>(char);
	template bool DataSender::inputData<int>(int);
	template bool DataSender::inputData<__int8>(__int8);
	template bool DataSender::inputData<__int16>(__int16);
	template bool DataSender::inputData<__int32>(__int32);
	template bool DataSender::inputData<__int64>(__int64);

	template bool DataSender::inputData<unsigned char>(unsigned char);
	template bool DataSender::inputData<int>(int);
	template bool DataSender::inputData<unsigned __int8>(unsigned __int8);
	template bool DataSender::inputData<unsigned __int16>(unsigned __int16);
	template bool DataSender::inputData<unsigned __int32>(unsigned __int32);
	template bool DataSender::inputData<unsigned __int64>(unsigned __int64);

	// Send data between threads on local computer
	struct LocalDataReciever : public DataReciever {
		void destructor() { delete(this); }
		bool fetchData();
		bool canFetchData(SocketStatusChecker* status_checker);
		std::stringstream shared_stream;
		std::mutex shared_lock;
	};

	// Send data between threads on local computer
	struct LocalDataSender : public DataSender {
		void destructor() { delete(this); }
		LocalDataSender(LocalDataReciever* t_reciever);
		bool sendData();
		bool canSendData(SocketStatusChecker* status_checker);
		LocalDataReciever* reciever;
	};

	void createLocalConnection(DataSender*& con1_sender,
		DataReciever*& con1_reciever,
		DataSender*& con2_sender,
		DataReciever*& con2_reciever);

	// Send data over the internet
	struct NetworkDataReciever : public DataReciever {
		void destructor() { delete(this); }
		NetworkDataReciever(TCPConnection* t_connection);
		bool fetchData();
		bool canFetchData(SocketStatusChecker* status_checker);
		TCPConnection* tcp_connection;
	};

	// Send data over the internet
	struct NetworkDataSender : public DataSender {
		void destructor() { delete(this); }
		NetworkDataSender(TCPConnection* t_connection);
		bool sendData();
		bool canSendData(SocketStatusChecker* status_checker);
		TCPConnection* tcp_connection;
	};

	typedef bool (*load_func_type)(data_transfer_::DataReciever* data_reciever, unsigned __int64 load_caller_id);
	extern std::vector<load_func_type> g_load_func;
	inline int addLoadFunc(load_func_type t_load_func) {
		g_load_func.push_back(t_load_func);
		return(g_load_func.size() - 1);
	}
	inline bool inputRecievingFunctionId(DataSender* sender, unsigned char func_id) {
		if (func_id && func_id < g_load_func.size()) {
			return(sender->inputData<unsigned char>(func_id));
		}
		else {
			log__fatal_error("Trying to input an invalid function id!");
		}
	}
	inline bool getRecievingFunctionId(DataReciever* reciever, unsigned char& func_id, unsigned int& pre_read_index) {
		pre_read_index = reciever->read_buffer_index;
		return reciever->readData<unsigned char>(func_id);
	}

	inline void loadData(DataReciever* data_reciever, unsigned __int64 load_caller_id) {
		// Store pre reading index in the event of not all data has arrived
		unsigned int pre_read_index;

		// Recieving function id
		unsigned char func_id;

		// Do while there is still data to read
		while (getRecievingFunctionId(data_reciever, func_id, pre_read_index)) {
			// Make sure it is a valid id
			if (func_id && func_id < g_load_func.size()) {
				// Call the load function
				if (g_load_func[func_id](data_reciever, load_caller_id) == false) {
					// Not all data has arrived yet
					data_reciever->read_buffer_index = pre_read_index;
					break;
				}
			}
			else {
				log__warning("Invalid function id detected");
			}
		}
		if ((data_reciever->read_buffer_index == data_reciever->used_buffer_size) ||
			(data_reciever->used_buffer_size > RECIEVER_BUFFER_SIZE / 2)) {
			// Make space for recieving new data
			data_reciever->copyUnreadDataToStartOfBuffer();
		}
	}
}

