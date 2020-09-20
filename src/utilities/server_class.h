#pragma once
#include "../utilities/memory_leak_detector.h"
#include "../utilities/network_sockets.h"
#include "../utilities/data_transfer.h"
#include "../utilities/logger.h"
#include "../utilities/mutex.h"
#include "../utilities/thread_synchronization.h"

#include <list>

#define log__server_vv(...) log__(logger__verbosity__vv, console_color_::LIGHT_TURQUOISE,				"SERVER:     ", __VA_ARGS__)
#define log__server_vvv(...) log__(logger__verbosity__vvv, console_color_::LIGHT_TURQUOISE,				"SERVER:     ", __VA_ARGS__)


namespace server_ {
	class HostServer;

	// A connection to a client, could be in the same program
	class ClientConnection {
	public:
		virtual void destructor() { delete this; }
		ClientConnection(HostServer* t_server);
		ClientConnection(TCPConnection* t_tcp_connection, HostServer* t_server);
		data_transfer_::DataSender* data_sender = nullptr;
		data_transfer_::DataReciever* data_reciever = nullptr;
		TCPConnection* tcp_connection = nullptr;
		std::list<ClientConnection*>::iterator client_list_erase_it;
		HostServer* server = nullptr;
		unsigned __int64 load_caller_id = 1;
	private:
		~ClientConnection();
	};

	class ServerConnection {
	public:
		virtual void destructor() { delete this; }
		ServerConnection(TCPConnection* t_tcp_connection) {
			tcp_connection = t_tcp_connection;
			data_sender = new data_transfer_::NetworkDataSender(tcp_connection);
			data_reciever = new data_transfer_::NetworkDataReciever(tcp_connection);
		}
		ServerConnection(){}

		// Should only be used by client
		data_transfer_::DataSender* data_sender = nullptr;
		data_transfer_::DataReciever* data_reciever = nullptr;
		TCPConnection* tcp_connection = nullptr;

	protected:
		~ServerConnection(){
			data_sender->destructor();
			data_reciever->destructor();
			if (tcp_connection) {
				delete(tcp_connection);
			}
		}
	};

	namespace server_request_ {
		enum {
			count_start = -1,
			no_request,
			close_server,
			close_server_sync,
			pause_server,
			pause_server_sync,
			accept_local_connection_sync,
			open_server_to_network,
			open_server_to_network_sync,
			remove_server_from_network,
			remove_server_from_network_sync,
			count_end,
		};
	}

	class HostServerInterface {
	public:
		// Functions that can be called client side
		void closeServerNonBlocking();
		void closeServerBlocking();
		void pauseServerNonBlocking();
		void pauseServerBlocking();
		ServerConnection* connectToServer();
		void openServerToNetworkNonBlocking(unsigned int port);
		void openServerToNetworkBlocking(unsigned int port);
		unsigned int request_open_port = 0;
		void removeServerFromNetworkNonBlocking();
		void removeServerFromNetworkBlocking();

		void unpauseServerNonBlocking();
		bool isServerRunning();
		void addServerRequest(unsigned int request_id) {
			interface_request_lock.lock();
			request_list.push_back({ request_id });
			interface_request_lock.unlock();
		}
		ServerConnection* latest_server_connection = nullptr;

	protected:
		HostServerInterface(HostServer* server) {
			m_host_server = server;
		}
		~HostServerInterface() {
		}
	private:
		HostServer* m_host_server = nullptr;

		std::mutex interface_request_lock;
		std::list<unsigned int> request_list = {};

		// To sync up with the server
		thread_::ThreadPairCommander* thread_pair_commander = nullptr;

		friend HostServer;
	};

	class HostServer {
	public:
		virtual void destructor() { delete this; }
		HostServerInterface* server_interface;
	protected:
		HostServer();
		~HostServer();

		// Keeps track of all client connections to gracefully close them
		//  at exit
		std::list<ClientConnection*> client_connection_list;

		bool recieveDataFromConnection(ClientConnection* t_client);
		void recieveDataFromAllConnections();
		bool acceptIncommingConnections();
		void updateSocketStatus();
		void sendStoredDataToConnections();
		bool updateInterfaceRequest() {
			bool return_value = false;
			// Make sure there are no requests already
			if (interface_request_id == server_request_::no_request) {
				// Avoid thread collisions
				server_interface->interface_request_lock.lock();
				// See if there are any new requests
				if (server_interface->request_list.size() > 0) {
					// Get the oldest request
					interface_request_id = server_interface->request_list.front();
					server_interface->request_list.pop_front();
					return_value = true;
				}
				server_interface->interface_request_lock.unlock();
			}
			return return_value;
		}
		bool isRequestingServerClosing();
		bool waitIfPauseRequest();
		bool updateNetworkStatus();

		// Virtual functions that are called when events happen
		virtual void unregisterClient(server_::ClientConnection* client_connection) {};
		virtual void registerClient(server_::ClientConnection* client_connection) {};
		virtual void server_main() = 0;

		// Locked while server is still running
		std::mutex server_running_lock;

		// Locked by interface while it wants server to stop
		std::mutex stop_lock;

		// For checking status of Sockets
		SocketStatusChecker socket_checker;

		// Copy requests from server_interface
		unsigned char interface_request_id = server_request_::no_request;

		TCPPortListener* port_listener = nullptr;

		friend ClientConnection;
		friend HostServerInterface;
	private:
		// Used since std::tread cannot take non static functions as arguments
		static void runMain(HostServer* hostserver);

		// To sync up with the interface
		thread_::ThreadPairListener* thread_pair_listener;
	};
}
