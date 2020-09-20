#include "server_class.h"
#include "time_functions.h"

void server_::HostServerInterface::closeServerNonBlocking() {
	log__vvv("Request to shut down server");
	addServerRequest(server_request_::close_server);
}
void server_::HostServerInterface::closeServerBlocking() {
	log__vvv("Request to shut down server");
	addServerRequest(server_request_::close_server_sync);
	thread_pair_commander->sync();
}
void server_::HostServerInterface::pauseServerNonBlocking() {
	log__vv("Pausing server");
	m_host_server->stop_lock.lock();
	addServerRequest(server_request_::pause_server);
}
void server_::HostServerInterface::pauseServerBlocking() {
	log__vv("Pausing server");
	m_host_server->stop_lock.lock();
	addServerRequest(server_request_::pause_server_sync);
	thread_pair_commander->sync();
}
server_::ServerConnection* server_::HostServerInterface::connectToServer() {
	log__vv("Connecting to server");
	addServerRequest(server_request_::accept_local_connection_sync);
	thread_pair_commander->sync();
	ServerConnection* return_value = latest_server_connection;
	latest_server_connection = nullptr;
	return return_value;
}
void server_::HostServerInterface::openServerToNetworkNonBlocking(unsigned int port) {
	log__vv("Open server to network");
	request_open_port = port;
	addServerRequest(server_request_::open_server_to_network);
}
void server_::HostServerInterface::openServerToNetworkBlocking(unsigned int port) {
	log__vv("Open server to network");
	request_open_port = port;
	addServerRequest(server_request_::open_server_to_network_sync);
	thread_pair_commander->sync();
}
void server_::HostServerInterface::removeServerFromNetworkNonBlocking() {
	log__vv("Remove server from network network");
	addServerRequest(server_request_::remove_server_from_network);
}
void server_::HostServerInterface::removeServerFromNetworkBlocking() {
	log__vv("Remove server from network network");
	addServerRequest(server_request_::remove_server_from_network_sync);
	thread_pair_commander->sync();
}
void server_::HostServerInterface::unpauseServerNonBlocking() {
	log__vv("Unpausing server");
	m_host_server->stop_lock.unlock();
}
bool server_::HostServerInterface::isServerRunning() {
	if (m_host_server->server_running_lock.try_lock()) {
		m_host_server->server_running_lock.unlock();
		return false;
	}
	else {
		return true;
	}
}

void server_::HostServer::runMain(HostServer* hostserver) {
	// Setup the channel to sync with the interface
	hostserver->thread_pair_listener->init();

	// Wait for the client to start server
	hostserver->stop_lock.lock();
	hostserver->stop_lock.unlock();

	// Show interface the server is running
	hostserver->server_running_lock.lock();

	// Run the main loop until the server should to close
	hostserver->server_main();

	// Make sure to close all locked mutexes
	hostserver->server_running_lock.unlock();
	hostserver->thread_pair_listener->deinit();
}

server_::HostServer::HostServer() {
	// Setup a server interface to communicate through
	server_interface = new HostServerInterface(this);

	// Create a channel to sync with the interface and server
	thread_::createThreadPairCommunication(server_interface->thread_pair_commander, thread_pair_listener);
	server_interface->thread_pair_commander->init();

	// Make sure the server doesnt start running right away
	stop_lock.lock();

	// Start a separate server thread
	thread__createThread(&runMain, this);

	// Make sure the channel has been inited
	while (server_interface->thread_pair_commander->isListenerInited() == false) {
		sleep(10);
	}
}

server_::HostServer::~HostServer() {
	while (client_connection_list.size() > 0) {
		client_connection_list.front()->destructor();
	}
	server_interface->thread_pair_commander->deinit();
	delete(thread_pair_listener);
	delete(server_interface->thread_pair_commander);
	delete(server_interface);
	if (port_listener) {
		delete(port_listener);
	}
}

bool server_::HostServer::acceptIncommingConnections() {
	// See if there are any local connection requests
	if (interface_request_id == server_request_::accept_local_connection_sync) {
		// Update request so the request is not executed twice
		interface_request_id = server_request_::no_request;

		// Create a new connection for server
		server_::ClientConnection* new_client_connection = new server_::ClientConnection(this);

		// Create a new connection for client
		server_interface->latest_server_connection = new server_::ServerConnection();

		// Link up the server and the client
		data_transfer_::createLocalConnection(new_client_connection->data_sender, 
			new_client_connection->data_reciever,
			server_interface->latest_server_connection->data_sender,
			server_interface->latest_server_connection->data_reciever);

		// Sync with the client to show the request is done
		thread_pair_listener->sync();

		log__server_vv("New local client connected!");

		// Register the new client
		registerClient(new_client_connection);

	}

	// See if there are any pending tcp connections
	if (port_listener && socket_checker.canRead(port_listener)) {
		// Connecting new client
		log__server_vv("Connecting new client...");
		TCPConnection* connection = port_listener->acceptConnection();
		if (connection != nullptr) {
			server_::ClientConnection* new_client_connection = new server_::ClientConnection(connection, this);
			registerClient(new_client_connection);
			log__server_vv("New client connected!");
			return true;
		}
		else {
			log__server_vv("New client connection failed");
			return false;
		}
	}
}

bool server_::HostServer::recieveDataFromConnection(server_::ClientConnection* t_client_connection) {
	// Make sure it can fetch data
	if (t_client_connection->data_reciever->canFetchData(&socket_checker)) {
		// Recieve new data
		if (t_client_connection->data_reciever->fetchData()) {
			data_transfer_::loadData(t_client_connection->data_reciever, t_client_connection->load_caller_id);
		}
		else {
			// There was an error while fetching data, remove client
			t_client_connection->destructor();
			return false;
		}
		return true;
	}
	return false;
}

void server_::HostServer::sendStoredDataToConnections() {
	std::list<server_::ClientConnection*>::iterator it = client_connection_list.begin();
	while (it != client_connection_list.end()) {
		if ((*it)->data_sender->canSendData(&socket_checker)) {
			(*it)->data_sender->sendData();
		}
		it++;
	}
}

void server_::HostServer::recieveDataFromAllConnections() {
	std::list<server_::ClientConnection*>::iterator it = client_connection_list.begin();
	while (it != client_connection_list.end()) {
		server_::ClientConnection* client_connection = (*it);
		it++;
		recieveDataFromConnection(client_connection);
	}
}

void server_::HostServer::updateSocketStatus() {
	socket_checker.reset();
	// Check for incomming connection requests
	if (port_listener) {
		socket_checker.insertReadStatusCheck(port_listener);
	}

	// Check clients sockets
	std::list<server_::ClientConnection*>::iterator it = client_connection_list.begin();
	while (it != client_connection_list.end()) {
		if ((*it)->tcp_connection) {
			socket_checker.insertReadStatusCheck((*it)->tcp_connection);
			socket_checker.insertWriteStatusCheck((*it)->tcp_connection);
		}
		it++;
	}
	socket_checker.doChecks();
}

bool server_::HostServer::isRequestingServerClosing() {
	if (interface_request_id == server_request_::close_server ||
		interface_request_id == server_request_::close_server_sync) {
		return true;
	}
	return false;
}

bool server_::HostServer::updateNetworkStatus() {
	if (interface_request_id == server_request_::open_server_to_network ||
		interface_request_id == server_request_::open_server_to_network_sync) {

		log__server_vv("Opening server to network");
		// Open a listening port
		port_listener = new TCPPortListener(server_interface->request_open_port);

		log__server_vv("Server open with ip: ", getIpOnLocalNetwork(), ":", server_interface->request_open_port);

		if (interface_request_id == server_request_::open_server_to_network_sync) {
			thread_pair_listener->sync();
		}

		// Make sure the request is not run twice
		interface_request_id = server_request_::no_request;
		return true;
	}
	else if (interface_request_id == server_request_::remove_server_from_network ||
		interface_request_id == server_request_::remove_server_from_network_sync) {
		log__server_vv("Removing server from network");
		// Remove the listening port
		delete(port_listener);
		port_listener = nullptr;

		if (interface_request_id == server_request_::remove_server_from_network_sync) {
			thread_pair_listener->sync();
		}

		// Make sure the request is not run twice
		interface_request_id = server_request_::no_request;
		return true;
	}
	return false;
}

bool server_::HostServer::waitIfPauseRequest() {
	if (interface_request_id == server_request_::pause_server ||
		interface_request_id == server_request_::pause_server_sync) {
		// Show the interface the server has stopped
		server_running_lock.unlock();

		if (interface_request_id == server_request_::pause_server_sync) {
			// Show it has read the request and stopped
			thread_pair_listener->sync();
		}

		// Wait for stop lock to be unlocked
		stop_lock.lock();
		stop_lock.unlock();

		// Show the interface that the server is running
		server_running_lock.lock();

		// Make sure the request is not run twice
		interface_request_id = server_request_::no_request;
		return true;
	}
	return false;
}

server_::ClientConnection::ClientConnection(HostServer* t_server) {
	// Add connection to server
	server = t_server;
	server->client_connection_list.push_front(this);
	client_list_erase_it = server->client_connection_list.begin();
}

server_::ClientConnection::~ClientConnection() {
	server->unregisterClient(this);
	data_reciever->destructor();
	data_sender->destructor();
	if (tcp_connection) {
		delete(tcp_connection);
	}
	server->client_connection_list.erase(client_list_erase_it);
}

server_::ClientConnection::ClientConnection(TCPConnection* t_tcp_connection, HostServer* t_server) {
	// Add connection to server
	server = t_server;
	server->client_connection_list.push_front(this);
	client_list_erase_it = server->client_connection_list.begin();

	// Setup the data transfer to send and recieve data
	tcp_connection = t_tcp_connection;
	data_reciever = new data_transfer_::NetworkDataReciever(t_tcp_connection);
	data_sender = new data_transfer_::NetworkDataSender(t_tcp_connection);
}


/*
void deinitServer() {
	using namespace server_;
	log__server_vv("Shutting down...");
	// Delete all connections
	while (server_::client_connection_list.size() > 0) {
		server_::client_connection_list.front()->destructor();
	}
	delete(port_listener);
	server_running_lock.unlock();
	log__server_vv("Exit");
}


void server_::wmain() {
	initServer();
	loadGame();
	log__server_vv("Running...");
	FpsTracker tick_tracker = FpsTracker(server_::TPS);
	FpsTracker closing_tracker = FpsTracker(10);

	while (true) {
		if (tick_tracker.isUpdateTime()) {
			tick_tracker.update_count++;
			updateSocketStatus();
			sendDataToClients();
			recieveDataFromClients();
			acceptIncommingConnections();
			executeTick();
		}

		if (closing_tracker.isUpdateTime()) {
			closing_tracker.update_count++;
			if (isRequestingServerClosing()) {
				break;
			}
		}
	}
	unloadGame();
	deinitServer();
}
*/