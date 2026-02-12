#include "ACommand.hpp"

ACommand::ACommand() : _server(NULL)
{
}

ACommand::ACommand(Server *server) : _server(server)
{
}

ACommand::ACommand(const ACommand &other) : _server(other._server)
{
}

ACommand &ACommand::operator=(const ACommand &other)
{
	if (this != &other)
		_server = other._server;
	return *this;
}

ACommand::~ACommand()
{
}

void ACommand::sendReply(int fd, const std::string &reply)
{
	std::string message = reply + "\r\n";
	send(fd, message.c_str(), message.length(), 0);
}

void ACommand::checkRegistration(Client &client)
{
	if (client.getStatus() == Client::AUTHENTICATED &&
		!client.getNickname().empty() && !client.getUsername().empty())
	{
		client.setStatus(Client::REGISTERED);
		sendReply(client.getFd(), ":server 001 " + client.getNickname() + " :Welcome to the IRC Network");
	}
}
