#ifndef CLIENTHANDLER_HPP
#define CLIENTHANDLER_HPP

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
	std::map<std::string, ACommand *> _commands;
	std::vector<std::string> parseCommand(const std::string &command);

public:
	ClientHandler(Server *server);
	~ClientHandler();
	void processCommand(Client &client, const std::string &rawLine);
};

#endif