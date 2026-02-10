#ifndef ACOMMAND_HPP
#define ACOMMAND_HPP

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
	ACommand(Server *server) : _server(server) {}
	virtual ~ACommand() {}
	virtual void execute(Client &client, const std::vector<std::string> &args) = 0;

	void sendReply(int fd, const std::string &reply)
	{
		std::string message = reply + "\r\n";
		send(fd, message.c_str(), message.length(), 0);
	}

	void checkRegistration(Client &client)
	{
		if (client.getStatus() == Client::AUTHENTICATED &&
			!client.getNickname().empty() && !client.getUsername().empty())
		{
			client.setStatus(Client::REGISTERED);
			sendReply(client.getFd(), ":server 001 " + client.getNickname() + " :Welcome to the IRC Network");
		}
	}
};

#endif