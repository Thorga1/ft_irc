#pragma once

#include <vector>
#include <string>
#include <iostream>
#include <sys/socket.h>
#include "client.hpp"
#include "server.hpp"

class Server;

class ACommand
{
protected:
	Server *_server;

public:
	ACommand();
	ACommand(Server *server);
	ACommand(const ACommand &other);
	ACommand &operator=(const ACommand &other);
	virtual ~ACommand();
	virtual void execute(Client &client, const std::vector<std::string> &args) = 0;

	void sendReply(int fd, const std::string &reply);
	void checkRegistration(Client &client);
};
