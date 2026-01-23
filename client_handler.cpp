#include "client_handler.hpp"
#include <sstream>
#include <algorithm>
#include <unistd.h>

ClientHandler::ClientHandler()
{
}

ClientHandler::~ClientHandler()
{
}

std::vector<std::string> ClientHandler::parseCommand(const std::string& command)
{
	std::vector<std::string> args;
	std::istringstream iss(command);
	std::string token;

	while (iss >> token)
		args.push_back(token);

	return args;
}

void ClientHandler::sendReply(int fd, const std::string& reply)
{
	std::string message = reply + "\r\n";
	send(fd, message.c_str(), message.length(), 0);
}

void ClientHandler::handleNick(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 431 * :No nickname given");
		return;
	}
	client.setNickname(args[1]);
	sendReply(client.getFd(), ":server 001 " + client.getNickname() + " :Welcome to IRC");
}

void ClientHandler::handleUser(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 * USER :Not enough parameters");
		return;
	}
	client.setUsername(args[1]);
}

void ClientHandler::handlePass(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2)
		sendReply(client.getFd(), ":server 461 * PASS :Not enough parameters");
}

void ClientHandler::handleMsg(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 3)
	{
		sendReply(client.getFd(), ":server 411 " + client.getNickname() + " :No recipient given (PRIVMSG)");
		return;
	}
	std::string target = args[1];
	std::string message = args[2];
	std::cout << "MSG from " << client.getNickname() << " to " << target << ": " << message << std::endl;
}

void ClientHandler::handleQuit(Client& client, const std::vector<std::string>& args)
{
	std::string reason = (args.size() > 1) ? args[1] : "Leaving";
	std::cout << client.getNickname() << " quit: " << reason << std::endl;
	client.disconnect();
}

void ClientHandler::processCommand(Client& client, const std::string& command)
{
	std::vector<std::string> args = parseCommand(command);

	if (args.empty())
		return;

	std::string cmd = args[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	if (cmd == "NICK")
		handleNick(client, args);
	else if (cmd == "USER")
		handleUser(client, args);
	else if (cmd == "PASS")
		handlePass(client, args);
	else if (cmd == "MSG" || cmd == "PRIVMSG")
		handleMsg(client, args);
	else if (cmd == "QUIT")
		handleQuit(client, args);
	else
		sendReply(client.getFd(), ":server 500 * :Unknown command");
}
