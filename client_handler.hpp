#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include "client.hpp"

class Server;

class ClientHandler
{
public:
	ClientHandler(Server* server);
	~ClientHandler();
	void	processCommand(Client& client, const std::string& command);

private:
	Server* _server;

	std::vector<std::string>	parseCommand(const std::string& command);
	void						sendReply(int fd, const std::string& reply);
	void						checkRegistration(Client& client);

	void	handleNick(Client& client, const std::vector<std::string>& args);
	void	handleUser(Client& client, const std::vector<std::string>& args);
	void	handlePass(Client& client, const std::vector<std::string>& args);
	void	handleMsg(Client& client, const std::vector<std::string>& args);
	void	handleQuit(Client& client, const std::vector<std::string>& args);


};
