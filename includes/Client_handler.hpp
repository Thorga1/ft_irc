#pragma once

#include <sstream>
#include <sys/socket.h>
#include <map>
#include <vector>
#include <string>
#include "ACommand.hpp"
#include "server.hpp"

class ACommand;
class Server;

class ClientHandler
{
private:
	Server *_server;
	std::map<std::string, ACommand *> _commands;
	std::vector<std::string> parseCommand(const std::string &command);
	void initCommands();
	void clearCommands();

public:
	ClientHandler();
	ClientHandler(Server *server);
	ClientHandler(const ClientHandler &other);
	~ClientHandler();
	ClientHandler &operator=(const ClientHandler &other);
	void processCommand(Client &client, const std::string &rawLine);
};
