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
#include <csignal>
#include "client.hpp"
#include "client_handler.hpp"
#include "channel.hpp"
#include <map>

class Server
{
private:
	std::string _password;
	unsigned int _port;
	int _socket_fd;
	std::vector<pollfd> _fds;
	std::map<int, Client *> _clients;
	std::map<std::string, Channel> _channels;
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
	std::map<int, Client *> getClients() const;
	std::map<std::string, Channel> &getChannels();
	const std::map<std::string, Channel> &getChannels() const;
	Channel *findChannel(const std::string &name);
	Channel &createChannel(const std::string &name, const Client &creator);
	void removeClientFromChannels(const std::string &nickname);
	void removeChannel(const std::string &name);
	void start();
	void run();
	void stop();
	bool isNickInUse(const std::string &nick) const;
	static void handleSignal(int signum);
	static bool shouldStop();
};

Server parseArguments(char **av);
bool isValidPort(unsigned int port);
bool isPortNumber(const std::string &str);