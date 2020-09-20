#pragma once

// WARNING! Not including this file first might cause linking errors
// The reason for this is that winsock sets up some flags that 
// needs to be set up before windows.h is included

#include <ws2tcpip.h>
// Need to link with Ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

#include <iphlpapi.h>

#include <winsock2.h>
#include <stdio.h>
#include <iostream> 
#include <string>
#include <list>
#include <fstream>

#include "../thread.h"
#include "../time_functions.h"
#include "../logger.h"
#include "../memory_leak_detector.h"
#define DEFAULT_BUFLEN 512

inline void initNeworkStuff() {
	// Collect data about your computer's socket implementation
	WSADATA WSAData;
	// Set up the program to work with your computer's socket implementation
	auto _ = WSAStartup(MAKEWORD(2, 0), &WSAData);
}

inline void deinitNeworkStuff() {
	WSACleanup();
}

inline std::string getIpOnLocalNetwork() {
	std::string line;
	std::ifstream IPFile;
	size_t offset;
	char search0[] = "IPv4 Address. . . . . . . . . . . :"; // search pattern
	system("ipconfig > ip.txt");
	IPFile.open("ip.txt");
	if (IPFile.is_open()) {
		while (!IPFile.eof()) {
			std::getline(IPFile, line);
			if ((offset = line.find(search0, 0)) != std::string::npos) {
				// IPv4 Address. . . . . . . . . . . : 1
				//1234567890123456789012345678901234567890
				line.erase(0, 39);
				IPFile.close();
				return line;
			}
		}
	}
	return "";
}

class SocketStatusChecker;

class SocketClass {
protected:
	SOCKET m_socket;
	friend SocketStatusChecker;
};

class SocketStatusChecker {
public:
	SocketStatusChecker();
	void insertReadStatusCheck(SocketClass* socket_class);
	void insertWriteStatusCheck(SocketClass* socket_class);
	void insertExceptStatusCheck(SocketClass* socket_class);
	bool canRead(SocketClass* socket_class);
	bool canWrite(SocketClass* socket_class);
	bool hasExcept(SocketClass* socket_class);
	int doChecks();

	// Resets everything, you have to insert the checks again
	void reset();
private:
	FD_SET writeset;
	FD_SET readset;
	FD_SET exceptset;
	struct timeval tv_timeout;
	bool getStatus_called = false;
	bool has_recieved_checks = false;
};

class TCPPortListener;


// TCP connection that can send and recieve data
class TCPConnection: public SocketClass {
public:
	// Send data in buffer, buffer_size = -1 for entire buffer
	int sendData(const char* data, int buffer_size);

	// MAKE SURE THERE IS ENOUGH SPACE IN BUFFER!
	// Returns how many bytes it recieved
	int recieveData(char* buffer, int buffer_size);

	~TCPConnection() {
		if (closesocket(m_socket) == SOCKET_ERROR) {
			int error = WSAGetLastError();
			log__fatal_error("Close socket failed with error: ", error);
		}
	}
	// Connect to IP (127.0.0.1 local computer ip)
	static TCPConnection* createTCPConnection(const char* dest_ip, unsigned int dest_port);
private:
	SOCKADDR_IN m_connection_addr;
	friend TCPPortListener;
};


// Listens and accepts connections on a given port
class TCPPortListener: public SocketClass {
public:
	TCPPortListener(int port_num);
	~TCPPortListener();

	// Accept incomming connections to port
	TCPConnection* acceptConnection();
	inline int getPortNum() { return m_port_num; }
private:
	SOCKADDR_IN m_server_addr;
	int m_port_num;
};


class UDPPort;

class UDPSender: public SocketClass {
public:
	UDPSender(const char* dest_ip, unsigned int dest_port);
	void sendData(char* buffer, int buffer_size);
	void setDest(const char* dest_ip, unsigned int dest_port) {
		m_send_addr.sin_port = htons(dest_port);
		InetPton(AF_INET, (dest_ip), &m_send_addr.sin_addr.s_addr);
	}
	~UDPSender() {
		/*if (closesocket(m_socket) == SOCKET_ERROR) {
			int error = WSAGetLastError();
			log__fatal_error("Close socket failed with error: ", error);
		}*/
	}
private:
	SOCKADDR_IN m_send_addr;
};

class UDPPortReciever: public SocketClass {
public:
	UDPPortReciever(int port_num);
	~UDPPortReciever() {
		// Close the reciever socket that was binded to port
		if (closesocket(m_socket) == SOCKET_ERROR) {
			int error = WSAGetLastError();
			log__fatal_error("Close socket failed with error: ", error);
		}
	}

	int recieveData(char(&buffer)[DEFAULT_BUFLEN]);
private:
	int m_port_num;
};


// Gets the ip of a domain and returns it as a ULONG
inline ULONG getDomainIPuLONG(const char* domainname) {
	struct addrinfo hints, * res;
	struct in_addr addr;
	int err;

	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_family = AF_INET;

	// Try getting the ip adress from domain name
	if ((err = getaddrinfo(domainname, NULL, &hints, &res)) != 0) {
		printf("could not get adress: error %d\n", err);
		return 0;
	}

	// Convert the respons to an ip adress
	addr.S_un = ((struct sockaddr_in*)(res->ai_addr))->sin_addr.S_un;

	return addr.S_un.S_addr;
}


// Converts an ip in ULONG format into a string format(what we humans read)
inline std::string convertIpUINTToSTR(unsigned int ip) {
	unsigned char bytes[4];
	bytes[0] = ip & 0xFF;
	bytes[1] = (ip >> 8) & 0xFF;
	bytes[2] = (ip >> 16) & 0xFF;
	bytes[3] = (ip >> 24) & 0xFF;
	std::string return_string =
		std::to_string(bytes[0]) + "." +
		std::to_string(bytes[1]) + "." +
		std::to_string(bytes[2]) + "." +
		std::to_string(bytes[3]);
	return return_string;
}


// Gets the ip of a domain name, for example "example.com", http:// and such cannot be passed in
inline std::string getDomainIP(const char* domainname) {
	ULONG ip = getDomainIPuLONG(domainname);
	return convertIpUINTToSTR(ip);
}