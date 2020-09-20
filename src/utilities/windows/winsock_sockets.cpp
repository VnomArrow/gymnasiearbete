#include "winsock_sockets.h"


SocketStatusChecker::SocketStatusChecker() {
	// Set up the struct timeval for the timeout when doing checking
	tv_timeout.tv_sec = 0;
	tv_timeout.tv_usec = 0;
	this->reset();
}

void SocketStatusChecker::insertReadStatusCheck(SocketClass* socket_class) {
	// Add to set
	if (getStatus_called == false) {
		FD_SET(socket_class->m_socket, &readset);
		has_recieved_checks = true;
	}
	else {
		log__fatal_error("call reset after getStatus before inserting new things!");
	}
}
void SocketStatusChecker::insertWriteStatusCheck(SocketClass* socket_class) {
	// Add to set
	if (getStatus_called == false) {
		FD_SET(socket_class->m_socket, &writeset);
		has_recieved_checks = true;
	}
	else {
		log__fatal_error("call reset after getStatus before inserting new things!");
	}
}
void SocketStatusChecker::insertExceptStatusCheck(SocketClass* socket_class) {
	// Add to set
	if (getStatus_called == false) {
		FD_SET(socket_class->m_socket, &exceptset);
		has_recieved_checks = true;
	}
	else {
		log__fatal_error("call reset after getStatus before inserting new things!");
	}
}
bool SocketStatusChecker::canRead(SocketClass* socket_class) {
	if (getStatus_called) {
		return FD_ISSET(socket_class->m_socket, &readset);
	}
	else {
		log__fatal_error("call getStatus before checking!");
		return false;
	}
}
bool SocketStatusChecker::canWrite(SocketClass* socket_class) {
	if (getStatus_called) {
		return FD_ISSET(socket_class->m_socket, &writeset);
	}
	else {
		log__fatal_error("call getStatus before checking!");
		return false;
	}
}
bool SocketStatusChecker::hasExcept(SocketClass* socket_class) {
	if (getStatus_called) {
		return FD_ISSET(socket_class->m_socket, &exceptset);
	}
	else {
		log__fatal_error("call getStatus before checking!");
		return false;
	}
}
int SocketStatusChecker::doChecks() {
	if (has_recieved_checks) {
		int total;
		if ((total = select(0, &readset, &writeset, &exceptset, &tv_timeout)) == SOCKET_ERROR) {
			int error = WSAGetLastError();
			if (error == 10038) {
				// Invalid socket
			}
			log__error("select() returned with error ", error);
			return 1;
		}
		else {
			getStatus_called = true;
			return total;
		}
	}
	return 0;
}
void SocketStatusChecker::reset() {
	has_recieved_checks = false;
	getStatus_called = false;
	FD_ZERO(&writeset);
	FD_ZERO(&readset);
	FD_ZERO(&exceptset);
}


int TCPConnection::sendData(const char* data, int buffer_size) {
	if (buffer_size == -1) {
		// Set buffer size to entire buffer
		buffer_size = (int)strlen(data);
	}

	// Send data in buffer
	int iResult = send(m_socket, data, buffer_size, 0);

	// Check for errors
	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == 10054) {
			//log__vv("The connection was shut down, so the message wasn't sent");
		}
		else {
			log__fatal_error("send failed: ", error);
		}
		return -1;
	}
	return 0;
}

int TCPConnection::recieveData(char* buffer, int buffer_size) {
	// Variable to store how many characters recieved
	int iResult = 0;

	// Recieve data
	iResult = recv(m_socket, buffer, buffer_size, 0);
	if (iResult >= 0) {
		return iResult;
	}
	else {
		// An error accured
		int error = WSAGetLastError();
		if (error == 10054) {
			// The connection was shut down by the other party
			return -1;
		}
		else if (error == 10053) {
			//the socket was closed down
		}
		else {
			log__fatal_error("recv failed: ", error);
		}
		return -1;
	}
}

TCPConnection* TCPConnection::createTCPConnection(const char* dest_ip, unsigned int dest_port) {
	// Create new instance of connection class
	TCPConnection* new_connection = new TCPConnection();

	// Find an unbound socket to use
	new_connection->m_socket = socket(AF_INET, SOCK_STREAM, 0);

	// Ensure socket could be found
	if (new_connection->m_socket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		log__fatal_error("Socket creation failed with error: ", error);
		delete(new_connection);
		return nullptr;
	}

	// Create class variables to store which port and protocoll to use
	SOCKADDR_IN addr;
	InetPton(AF_INET, (dest_ip), &addr.sin_addr.s_addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(dest_port);

	// Connect to a server using the specified port and protocoll 
	if (connect(new_connection->m_socket, (SOCKADDR*)& addr, sizeof(addr)) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		if (error == 10060) {
			log__vv("Seems like your friend didn't accept your invitation");
		}
		else if (error == 10061) {
			log__vv("Server actively refused the tcp connection");
		}
		else {
			log__fatal_error("Connection failed with error: ", error);
		}
		delete(new_connection);
		return nullptr;
	}
	return new_connection;
}

TCPPortListener::TCPPortListener(int port_num) {
	m_port_num = port_num;
	m_server_addr = SOCKADDR_IN();

	// Find an unbound socket to use
	m_socket = socket(AF_INET, SOCK_STREAM, 0);

	// Ensure socket was succesfully created
	if (m_socket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		log__fatal_error("Socket creation failed with error: ", error);
		return;
	}

	// Specify which port and protocoll to use
	m_server_addr.sin_addr.s_addr = INADDR_ANY;
	m_server_addr.sin_family = AF_INET;
	m_server_addr.sin_port = htons(port_num);

	// Make listening socket use the specified port and protocoll 
	if (bind(m_socket, (SOCKADDR*)& m_server_addr, sizeof(m_server_addr)) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("Bind function failed with error: ", error);
		return;
	}

	// Make program able to acces backlog of incomming connections
	if (listen(m_socket, 0) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("Listen function failed with error: ", error);
		return;
	}
}

TCPPortListener::~TCPPortListener() {
	// Close the listening socket
	if (closesocket(m_socket) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("Close socket failed with error: ", error);
	}
}

TCPConnection* TCPPortListener::acceptConnection() {
	// Create new instance of reciever class
	TCPConnection* new_connection = new TCPConnection();

	// Get size of connection adress
	int connectionAddrSize = sizeof(new_connection->m_connection_addr);

	// Wait for incomming connection
	if ((new_connection->m_socket = accept(m_socket, (SOCKADDR*)& new_connection->m_connection_addr, &connectionAddrSize)) != INVALID_SOCKET) {
		return new_connection;
	}
	else {
		std::cout << "connection error" << std::endl;
		delete(new_connection);
		return nullptr;
	}
	return new_connection;
}

UDPSender::UDPSender(const char* dest_ip, unsigned int dest_port) {
	// Find an unbound socket to use
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Ensure socket was succesfully created
	if (m_socket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		log__fatal_error("Socket creation failed with error: ", error);
	}

	// Setup adress to connect to
	m_send_addr.sin_family = AF_INET;
	m_send_addr.sin_port = htons(dest_port);
	InetPton(AF_INET, (dest_ip), &m_send_addr.sin_addr.s_addr);
}

void UDPSender::sendData(char* buffer, int buffer_size) {
	int iResult = sendto(m_socket, buffer, buffer_size, 0, (SOCKADDR*)& m_send_addr, sizeof(m_send_addr));
	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("udp sendto failed with error: ", error);
		delete(this);
	}
}

UDPPortReciever::UDPPortReciever(int port_num) {
	m_port_num = port_num;

	// Find an unbound socket to use
	m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Ensure socket was succesfully created
	if (m_socket == INVALID_SOCKET) {
		int error = WSAGetLastError();
		log__fatal_error("Socket creation failed with error: ", error);
		return;
	}

	// Create class variables to store which port and protocoll to use
	SOCKADDR_IN serverAddr;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(m_port_num);

	// Make socket use the specified port and protocoll 
	if (bind(m_socket, (SOCKADDR*)& serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("Bind function failed with error: ", error);
		return;
	}
}

int UDPPortReciever::recieveData(char(&buffer)[DEFAULT_BUFLEN]) {
	// Create variable to store the senders info
	struct sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	// Variable to store length of message
	int iResult;

	// Recieve message
	iResult = recvfrom(m_socket, buffer, DEFAULT_BUFLEN, 0, (SOCKADDR*)& SenderAddr, &SenderAddrSize);

	if (iResult == SOCKET_ERROR) {
		int error = WSAGetLastError();
		log__fatal_error("recvfrom failed with error: ", error);
		return 0;
	}
	return iResult;
}