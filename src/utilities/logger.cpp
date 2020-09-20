#include "logger.h"
#include "time_functions.h"
#include <fstream>

std::mutex log_lock;

void setConsoleColor(unsigned __int16 color);

#ifdef _WIN32
#include "windows.h"
void setConsoleColor(unsigned __int16 color) {
	WORD color_flag = color;
	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(handle, color_flag);
}
#endif

void logger::init() {
	std::cout << "logger::init" << std::endl;

	//clear log dump file
	std::ofstream myfile;
	myfile.open("log_dump.txt");
	myfile.close();
}

void logger::log(std::string message, std::string path, long line, unsigned __int16 color, const char message_type[12]) {
	// Make sure no other thread is writing
	log_lock.lock();

	// Open log dump file
	std::ofstream myfile;
	myfile.open("log_dump.txt", std::ofstream::out | std::ofstream::app);

	// Record which thread the call was made on
	setConsoleColor(console_color_::BLUE);
	std::cout << std::this_thread::get_id() << " ";
	myfile <<    std::this_thread::get_id() << " ";

	// Record when the call was made
	std::cout << time_stamp() + " ";
	myfile <<    time_stamp() + " ";
	
	// Record where the call was made
	std::cout << path << ": " << line << std::endl;
	myfile    << path << ": " << line << std::endl;

	// Record Message type
	setConsoleColor(color);
	std::cout << message_type;
	myfile    << message_type;

	// Record the message
	setConsoleColor(console_color_::WHITE);
	std::cout << message << std::endl;
	myfile    << message << std::endl;

	// Close file
	myfile << std::endl;
	myfile.close();

	// Make sure other threads are able to log
	log_lock.unlock();
}