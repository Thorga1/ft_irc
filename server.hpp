#pragma once

#include <cerrno>
#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#include <map>
#include "client.hpp"
#include "client_handler.hpp"

class Server
{
private:
	std::string _password;
	unsigned int _port;
	int _socket_fd;
	std::vector<pollfd> _fds;
	std::map<int, Client *> _clients;
	ClientHandler _handler;

	void acceptNewClient();
	bool handleClientData(size_t fd_index);
	void removeClient(size_t fd_index);
	void broadcastMessage(const std::string &message);

public:
	Server(std::string password, unsigned int port);
	~Server();
	unsigned int getPort() const;
	std::string getPassword() const;
	void start();
	void run();
	void stop();
	bool isNickInUse(const std::string &nick) const;
};

Server parseArguments(char **av);
bool isValidPort(unsigned int port);
bool isPortNumber(const std::string &str);