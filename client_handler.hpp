#pragma once

#include <string>
#include <vector>
 #include <sys/socket.h>
#include "client.hpp"

class ClientHandler
{
public:
	ClientHandler();
	~ClientHandler();
	void				processCommand(Client& client, const std::string& command);

private:
	void				handleNick(Client& client, const std::vector<std::string>& args);
	void				handleUser(Client& client, const std::vector<std::string>& args);
	void				handlePass(Client& client, const std::vector<std::string>& args);
	void				handleMsg(Client& client, const std::vector<std::string>& args);
	void				handleQuit(Client& client, const std::vector<std::string>& args);
	std::vector<std::string>	parseCommand(const std::string& command);
	void						sendReply(int fd, const std::string& reply);
};
