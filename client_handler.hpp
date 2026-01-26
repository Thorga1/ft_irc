#pragma once

#include <string>
#include <vector>
#include <sys/socket.h>
#include "client.hpp"
#include "channel.hpp"
#include <map>

class Server;

class ClientHandler
{
public:
	ClientHandler(Server *server);
	ClientHandler();
	~ClientHandler();
	void processCommand(Client &client, const std::string &command, std::map<int, Client *> clients);
	Client *findUser(const std::string &nickname, const std::map<int, Client *> &clients);

private:
	void handleNick(Client &client, const std::vector<std::string> &args);
	void handleUser(Client &client, const std::vector<std::string> &args);
	void handlePass(Client &client, const std::vector<std::string> &args);
	void handleMsg(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients);
	void handleJoin(Client &client, const std::vector<std::string> &args);
	void handleKick(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients);
	void handleInvite(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients);
	void handleTopic(Client &client, const std::vector<std::string> &args);
	void handleMode(Client &client, const std::vector<std::string> &args);
	void handleQuit(Client &client, const std::vector<std::string> &args);

	std::vector<std::string> parseCommand(const std::string &command);
	void sendReply(int fd, const std::string &reply);
	void checkRegistration(Client &client);
	Server *_server;
};

Client *findUser(const std::string &nickname, const std::map<int, Client *> &clients);
