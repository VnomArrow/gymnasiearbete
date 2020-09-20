#include "data_transfer.h"
#include "byte_functions.h"

#pragma warning(push)
#pragma warning(disable: 4005)
// Shows how verbose the logger should be
#define logger__verbosity logger__verbosity__v
#pragma warning(pop)

std::vector<data_transfer_::load_func_type> data_transfer_::g_load_func = { nullptr };

template<typename T>
bool data_transfer_::DataReciever::readData(T& value_holder) {
	if (read_buffer_index + sizeof(T) <= used_buffer_size) {
		value_holder = *castPtr<char, T>(&buffer[read_buffer_index]);
		read_buffer_index += sizeof(T);
		return true;
	}
	return false;
}

void data_transfer_::DataReciever::copyUnreadDataToStartOfBuffer() {
	int i = 0;
	// Move back the data that has not been read yet
	for (int i = 0; i < used_buffer_size - read_buffer_index; ++i) {
		buffer[i] = buffer[i + read_buffer_index];
	}

	used_buffer_size = used_buffer_size - read_buffer_index;
	read_buffer_index = 0;
}

template<typename T>
bool data_transfer_::DataSender::inputData(T data) {
	if (used_buffer_size + sizeof(T) < SENDER_BUFFER_SIZE) {
		*castPtr<char, T>(&buffer[used_buffer_size]) = data;
		used_buffer_size += sizeof(T);
		return true;
	}
	return false;
}

data_transfer_::LocalDataSender::LocalDataSender(data_transfer_::LocalDataReciever* t_reciever) {
	reciever = t_reciever;
	t_reciever->shared_stream.clear();
}

bool data_transfer_::LocalDataSender::sendData() {
	// Return if there is no data to copy
	if (used_buffer_size == 0) {
		return true;
	}

	// Make sure there are no thread collisions
	reciever->shared_lock.lock();

	// Send the data
	reciever->shared_stream.write(&buffer[0], used_buffer_size);

	log__vv("Sent: ", used_buffer_size);

	// Reset the buffer
	used_buffer_size = 0;

	reciever->shared_lock.unlock();
	return false;
}
bool data_transfer_::LocalDataSender::canSendData(SocketStatusChecker* status_checker) {
	return (this->used_buffer_size > 0);
}


bool data_transfer_::LocalDataReciever::fetchData() {
	// Make sure there are no thread collisions
	shared_lock.lock();

	// Try copying until buffer is full
	shared_stream.read(&buffer[used_buffer_size], RECIEVER_BUFFER_SIZE - used_buffer_size);

	// Update the used buffer size
	used_buffer_size += shared_stream.gcount();

	if (used_buffer_size != read_buffer_index) {
		log__vv("Fetched: ", shared_stream.gcount());
	}

	// Clear flags
	shared_stream.clear();

	shared_lock.unlock();
	return true;
}

bool data_transfer_::LocalDataReciever::canFetchData(SocketStatusChecker* status_checker) {
	/*shared_lock.lock();
	bool return_val = shared_stream.rdbuf()->in_avail() == 0;
	shared_lock.unlock();
	return return_val;*/
	return true;
}

void data_transfer_::createLocalConnection(DataSender*& con1_sender,
	DataReciever*& con1_reciever,
	DataSender*& con2_sender,
	DataReciever*& con2_reciever) {

	LocalDataReciever* local_con1_reciever = new LocalDataReciever();
	con1_reciever = local_con1_reciever;
	con2_sender = new LocalDataSender(local_con1_reciever);

	LocalDataReciever* local_con2_reciever = new LocalDataReciever();
	con2_reciever = local_con2_reciever;
	con1_sender = new LocalDataSender(local_con2_reciever);
}


data_transfer_::NetworkDataReciever::NetworkDataReciever(TCPConnection* t_connection) {
	tcp_connection = t_connection;
}

bool data_transfer_::NetworkDataReciever::fetchData() {
	int tResult = tcp_connection->recieveData(&buffer[used_buffer_size], getUnusedDataSize());
	if (tResult > 0) {
		used_buffer_size += tResult;
		return true;
	}
	return false;
}
bool data_transfer_::NetworkDataReciever::canFetchData(SocketStatusChecker* status_checker) {
	return status_checker->canRead(this->tcp_connection);
}

data_transfer_::NetworkDataSender::NetworkDataSender(TCPConnection* t_connection) {
	tcp_connection = t_connection;
}

bool data_transfer_::NetworkDataSender::sendData() {
	if (used_buffer_size > 0) {
		int tResult = tcp_connection->sendData(&buffer[0], used_buffer_size);
		used_buffer_size = 0;
		if (tResult == 0) {
			return true;
		}
	}
	return false;
}

bool data_transfer_::NetworkDataSender::canSendData(SocketStatusChecker* status_checker) {
	return status_checker->canWrite(this->tcp_connection);
}


