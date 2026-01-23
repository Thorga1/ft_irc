#include "client_handler.hpp"
#include <sstream>
#include <algorithm>
#include "server.hpp"
#include <unistd.h>

ClientHandler::ClientHandler(Server* server) : _server(server)
{
}

ClientHandler::~ClientHandler()
{
}

std::vector<std::string> ClientHandler::parseCommand(const std::string& command)
{
	std::vector<std::string> args;
	size_t pos = command.find(" :");
	
	std::string beforeColon = (pos == std::string::npos) ? command : command.substr(0, pos);
	std::istringstream iss(beforeColon);
	std::string token;
	while (iss >> token)
		args.push_back(token);

	if (pos != std::string::npos)
		args.push_back(command.substr(pos + 2));
	
	return args;
}

void ClientHandler::sendReply(int fd, const std::string& reply)
{
	std::string message = reply + "\r\n";
	send(fd, message.c_str(), message.length(), 0);
}

void ClientHandler::handleNick(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 2) {
		sendReply(client.getFd(), ":server 431 * :No nickname given");
		return;
	}
	std::string newNick = args[1];

	if (client.getNickname() == newNick) {
		return;
	}

	if (_server->isNickInUse(newNick))
	{
		std::string current = client.getNickname().empty() ? "*" : client.getNickname();
		sendReply(client.getFd(), ":server 433 " + current + " " + newNick + " :Nickname is already in use");
		return;
	}
	std::cout << "[NICK] FD " << client.getFd() << " change son pseudo pour : " << newNick << std::endl;
	client.setNickname(newNick);
	checkRegistration(client);
}

void ClientHandler::handleUser(Client& client, const std::vector<std::string>& args)
{
	if (args.size() < 5)
	{ // USER <username> <mode> <unused> :<realname> d'ou les 5 parametres :D
		sendReply(client.getFd(), ":server 461 * USER :Not enough parameters");
		return;
	}
	client.setUsername(args[1]);
	client.setRealname(args[4]); // On utilise l'argument après le ":"
	checkRegistration(client);
}

void ClientHandler::handlePass(Client& client, const std::vector<std::string>& args)
{
	if (client.getStatus() != Client::PENDING) {
		sendReply(client.getFd(), ":server 462 * :You may not reregister");
		return;
	}
	if (args.size() < 2) {
		sendReply(client.getFd(), ":server 461 * PASS :Not enough parameters");
		return;
	}
	if (args[1] == _server->getPassword())
	{
		client.setStatus(Client::AUTHENTICATED);
	}
	else
		sendReply(client.getFd(), ":server 464 * :Password incorrect");
}

void ClientHandler::checkRegistration(Client& client)
{
	if (client.getStatus() == Client::AUTHENTICATED && 
		!client.getNickname().empty() && !client.getUsername().empty()) 
	{
		client.setStatus(Client::REGISTERED);
		std::string nick = client.getNickname();
		sendReply(client.getFd(), ":server 001 " + nick + " :Welcome to the IRC Network, " + nick);
	}

	if (client.getStatus() == Client::REGISTERED)
	{
	std::cout << "[SUCCESS] Client '" << client.getNickname() 
			  << "' (FD: " << client.getFd() << ") est maintenant enregistré." << std::endl;
	}
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


	if (client.getStatus() == Client::PENDING)
	{
		if (cmd == "PASS")
			handlePass(client, args);
		else if (cmd == "QUIT")
			handleQuit(client, args);
		else
			sendReply(client.getFd(), ":server 451 * :You must send a password first (PASS <password>)");
		return ;
	}

	if (client.getStatus() == Client::AUTHENTICATED)
	{
		if (cmd == "PASS")
			sendReply(client.getFd(), ":server 462 * :Unauthorized command (already authenticated)");
		else if (cmd == "NICK")
			handleNick(client, args);
		else if (cmd == "USER")
			handleUser(client, args);
		else if (cmd == "QUIT")
			handleQuit(client, args);
		else
			sendReply(client.getFd(), ":server 451 * :You must set your NICK and USER before chatting");
		return;
	}

	if (cmd == "PRIVMSG" || cmd == "MSG")
		handleMsg(client, args);
	else if
		(cmd == "QUIT")
		handleQuit(client, args);
	else if
		(cmd == "NICK")
		handleNick(client, args);
	else if (cmd == "USER")
		sendReply(client.getFd(), ":server 462 " + client.getNickname() + " :You may not reregister");
	else
		sendReply(client.getFd(), ":server 421 " + client.getNickname() + " " + cmd + " :Unknown command");
}
