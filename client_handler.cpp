#include "client_handler.hpp"
#include <sstream>
#include <algorithm>
#include "server.hpp"
#include <unistd.h>

ClientHandler::ClientHandler(Server *server) : _server(server)
{
}

ClientHandler::~ClientHandler()
{
}

ClientHandler::ClientHandler() {}

std::vector<std::string> ClientHandler::parseCommand(const std::string &command)
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

void ClientHandler::sendReply(int fd, const std::string &reply)
{
	std::string message = reply + "\r\n";
	send(fd, message.c_str(), message.length(), 0);
}

void ClientHandler::handleNick(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 431 * :No nickname given");
		return;
	}
	std::string newNick = args[1];

	if (client.getNickname() == newNick)
	{
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

void ClientHandler::handleKick(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients)
{
	if (args.size() < 3)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters");
		return;
	}
	std::string channelName = args[1];
	std::string targetNick = args[2];

	Channel *ch = _server->findChannel(channelName);
	if (!ch)
	{
		sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel");
		return;
	}
	if (!ch->hasAdmin(client.getNickname()))
	{
		sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
		return;
	}
	Client *targetClient = findUser(targetNick, clients);
	if (!targetClient || (!ch->hasUser(targetNick) && !ch->hasAdmin(targetNick)))
	{
		sendReply(client.getFd(), ":server 441 " + client.getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel");
		return;
	}
	if (ch->hasAdmin(targetNick))
	{
		sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + channelName + " :Cannot kick channel operator");
		return;
	}
	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + channelName + " " + targetNick + "\r\n";
	ch->broadcastMessage(kickMsg, -1);
	ch->kick(targetNick);
}

void ClientHandler::handleUser(Client &client, const std::vector<std::string> &args)
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

void ClientHandler::handlePass(Client &client, const std::vector<std::string> &args)
{
	if (client.getStatus() != Client::PENDING)
	{
		sendReply(client.getFd(), ":server 462 * :You may not reregister");
		return;
	}
	if (args.size() < 2)
	{
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

void ClientHandler::checkRegistration(Client &client)
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

Client *ClientHandler::findUser(const std::string &nickname, const std::map<int, Client *> &clients)
{
	for (std::map<int, Client *>::const_iterator it = clients.begin(); it != clients.end(); ++it)
	{
		Client *client = it->second;
		if (client->getNickname() == nickname)
			return client;
	}
	return NULL;
}

void ClientHandler::handleFlagModes(Client &client, Channel *ch, const std::vector<std::string> &args)
{
	int i = ch->mode(args[2]);
	std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + ch->getTopic() + " " + args[2][i];
	ch->broadcastMessage(modeMsg + "\r\n", -1);
}

void ClientHandler::handleLimitMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
{
	if (adding)
	{
		if (args.size() < 4)
		{
			sendReply(client.getFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters (+l needs a limit)");
			return;
		}
		int limit = std::atoi(args[count++].c_str());
		if (limit <= 0)
			return;
		ch->setLimit(limit);
		std::string modeMsg = ":" + client.getNickname() + " MODE " + ch->getTopic() + " +l " + args[3];
		ch->broadcastMessage(modeMsg + "\r\n", -1);
	}
	else
	{
		ch->setLimit(-1);
		std::string modeMsg = ":" + client.getNickname() + " MODE " + ch->getTopic() + " -l";
		ch->broadcastMessage(modeMsg + "\r\n", -1);
	}
}

void ClientHandler::handleKeyMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
{
	if (adding)
	{
		if (args.size() < 4)
		{
			sendReply(client.getFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters (+k needs a key)");
			return;
		}
		ch->setKey(args[count++]);
		std::string modeMsg = ":" + client.getNickname() + " MODE " + ch->getTopic() + " +k " + args[count - 1];
		ch->broadcastMessage(modeMsg + "\r\n", -1);
	}
	else
	{
		ch->setKey("");
		std::string modeMsg = ":" + client.getNickname() + " MODE " + ch->getTopic() + " -k";
		ch->broadcastMessage(modeMsg + "\r\n", -1);
	}
}

void ClientHandler::handleOperatorMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
{
	if (args.size() < 4)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters (+o needs a nickname)");
		return;
	}

	std::string targetNick = args[count++];
	if (adding)
		ch->promoteToAdmin(targetNick);
	else
		ch->demoteFromAdmin(targetNick);

	std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + ch->getTopic() + " " + (adding ? "+o " : "-o ") + targetNick;
	ch->broadcastMessage(modeMsg + "\r\n", -1);
}

void ClientHandler::handleMode(Client &client, const std::vector<std::string> &args)
{
	int count = 3;
	bool adding = true;

	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters");
		return;
	}
	Channel *ch = _server->findChannel(args[1]);
	if (!ch)
		return;
	if (!ch->hasAdmin(client.getNickname()))
	{
		sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + args[1] + " :You're not channel operator");
		return;
	}
	if (args.size() == 2)
	{
		sendReply(client.getFd(), ":server 324 " + client.getNickname() + " " + args[1] + " :" + ch->getModes());
		return;
	}
	std::string modeStr = args[2];
	// bool adding = (modeStr.find('+') != std::string::npos);

	// if (modeStr.find('o') != std::string::npos)
	// 	handleOperatorMode(client, ch, args, adding);
	// if (modeStr.find('k') != std::string::npos)
	// 	handleKeyMode(client, ch, args, adding);
	// if (modeStr.find('l') != std::string::npos)
	// 	handleLimitMode(client, ch, args, adding);
	// if (modeStr.find('i') != std::string::npos || modeStr.find('t') != std::string::npos)
	// 	handleFlagModes(client, ch, modeStr);

	for (size_t i = 0; i < modeStr.length(); i++)
	{
		char c = modeStr[i];
		if (c == '+')
		{
			adding = true;
			continue;
		}
		if (c == '-')
		{
			adding = false;
			continue;
		}
		if (c == 'o')
			handleOperatorMode(client, ch, args, count, adding);
		else if (c == 'k')
			handleKeyMode(client, ch, args, count, adding);
		else if (c == 'l')
			handleLimitMode(client, ch, args, count, adding);
		else if (c == 'i' || c == 't')
			handleFlagModes(client, ch, args);
		else
			sendReply(client.getFd(), ":server 472 " + client.getNickname() + " " + c + " :is unknown mode char");
	}
}
void ClientHandler::handleMsg(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients)
{
	if (args.size() < 3)
	{
		sendReply(client.getFd(), ":server 411 " + client.getNickname() + " :No recipient given (PRIVMSG)");
		return;
	}

	std::string target = args[1];
	std::string message = args[2];

	std::cout << "MSG from " << client.getNickname() << "!" << client.getUsername() << "@localhost PRIVMSG " << " to " << target << ":" << message << "\r\n"
			  << std::endl;

	if (!target.empty() && (target[0] == '#' || target[0] == '&'))
	{
		Channel *ch = _server->findChannel(target);
		if (!ch)
		{
			sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + target + " :No such channel");
			return;
		}
		std::string formatted = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message;
		// sendReply(client.getFd(), formatted);
		ch->broadcastMessage(formatted + "\r\n", client.getFd());
		return;
	}

	Client *receiver = findUser(target, clients);

	if (receiver == NULL || receiver->getFd() == -1)
	{
		sendReply(client.getFd(), ":server 401 " + client.getNickname() + " " + target + " :No such nick/channel");
		std::cerr << "Invalid receiver for target: " << target << std::endl;
		return;
	}
	std::string formattedMessage = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost PRIVMSG " + target + " :" + message + "\r\n";

	std::cerr << "Sending to " << target << ": " << formattedMessage << std::endl;

	ssize_t bytesSent = send(receiver->getFd(), formattedMessage.c_str(), formattedMessage.size(), 0);

	if (bytesSent == -1)
		std::cerr << "error: send()" << std::endl;
	else
		std::cout << "Sent " << bytesSent << " bytes to " << receiver->getNickname() << std::endl;
	std::cout << "MSG from " << client.getNickname() << " to " << target << ": " << message << std::endl;
}

void ClientHandler::handleQuit(Client &client, const std::vector<std::string> &args)
{
	std::string reason = (args.size() > 1) ? args[1] : "Client quit";
	sendReply(client.getFd(), "ERROR :Closing Link: " + client.getNickname() + " (" + reason + ")");
	client.setStatus(Client::DISCONNECTED);
	std::cout << "[QUIT] Client " << client.getNickname() << " a demandé à quitter." << std::endl;
}

void ClientHandler::handleJoin(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " JOIN :Not enough parameters");
		return;
	}

	std::string list = args[1];

	std::string providedKey = (args.size() > 2) ? args[2] : "";

	size_t start = 0;
	while (start <= list.size())
	{
		size_t comma = list.find(',', start);
		std::string name = (comma == list.npos) ? list.substr(start) : list.substr(start, comma - start);

		if (!name.empty())
		{
			if (name[0] != '#' && name[0] != '&')
			{
				sendReply(client.getFd(), ":server 479 " + client.getNickname() + " " + name + " :Illegal channel name");
			}
			else
			{
				Channel *ch = _server->findChannel(name);
				if (!ch)
				{
					Channel &created = _server->createChannel(name, client);
					ch = &created;
				}

				else
				{
					if (ch->isInviteOnly() && !ch->isInInvitedUsers(client.getNickname()))
					{
						sendReply(client.getFd(), ":server 473 " + client.getNickname() + " " + name + " :Cannot join channel (+i)");
						goto next_channel;
					}
					if (ch->hasKey() && providedKey != ch->getKey())
					{
						sendReply(client.getFd(), ":server 475 " + client.getNickname() + " " + name + " :Cannot join channel (+k)");
						goto next_channel;
					}
					if (ch->getLimit() != -1 && ch->getUserCount() >= ch->getLimit())
					{
						sendReply(client.getFd(), ":server 471 " + client.getNickname() + " " + name + " :Cannot join channel (+l)");
						goto next_channel;
					}
				}

				if (!ch->hasUser(client.getNickname()) && !ch->hasAdmin(client.getNickname()))
				{
					ch->setUser(client.getNickname(), client.getFd());
					std::string joinMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost JOIN " + name;
					sendReply(client.getFd(), joinMsg);
					ch->broadcastMessage(joinMsg + "\r\n", client.getFd());
					if (ch->isInviteOnly())
						ch->removeFromInvited(client.getNickname());
				}
			}
		}

	next_channel:
		if (comma == list.npos)
			break;
		start = comma + 1;
	}
}

void ClientHandler::handleTopic(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " TOPIC :Not enough parameters");
		return;
	}
	if (args[1][0] != '#' && args[1][0] != '&')
	{
		sendReply(client.getFd(), ":server 479 " + client.getNickname() + " " + args[1] + " :Illegal channel name");
		return;
	}
	std::string channelName = args[1];
	if (!_server->findChannel(channelName))
	{
		sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel");
		return;
	}
	if (!(_server->findChannel(channelName))->hasUser(client.getNickname()) && !(_server->findChannel(channelName))->hasAdmin(client.getNickname()))
	{
		sendReply(client.getFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
		return;
	}
	Channel *ch = _server->findChannel(channelName);
	if (args.size() == 2)
	{
		std::string topic = ch->getTopic();
		;
		if (topic.empty())
		{
			sendReply(client.getFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set");
		}
		else
		{
			sendReply(client.getFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + topic);
		}
	}
	else if (args.size() >= 3)
	{
		if (ch->isTopicProtected() && !ch->hasAdmin(client.getNickname()))
		{
			sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
			return;
		}

		std::string newTopic = args[2];
		ch->topic(newTopic);
		std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channelName + " :" + newTopic;
		sendReply(client.getFd(), topicMsg);
		ch->broadcastMessage(topicMsg + "\r\n", client.getFd());
	}
}

void ClientHandler::handleInvite(Client &client, const std::vector<std::string> &args, std::map<int, Client *> clients)
{
	int fd = client.getFd();
	if (args.size() < 3)
	{
		sendReply(fd, ":server 461 " + client.getNickname() + " INVITE :Not enough parameters");
		return;
	}
	std::string targetNick = args[1];
	std::string channelName = args[2];
	Client *targetClient = findUser(targetNick, clients);
	if (!targetClient)
	{
		sendReply(fd, ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel");
		return;
	}
	Channel *ch = _server->findChannel(channelName);
	if (!ch)
	{
		sendReply(fd, ":server 403 " + client.getNickname() + " " + channelName + " :No such channel");
		return;
	}
	if (!ch->hasUser(client.getNickname()) && !ch->hasAdmin(client.getNickname()))
	{
		sendReply(fd, ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
		return;
	}
	if (ch->isInviteOnly() && !ch->hasAdmin(client.getNickname()))
	{
		sendReply(fd, ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
		return;
	}
	ch->invite(targetNick);
	sendReply(fd, ":server 341 " + client.getNickname() + " " + targetNick + " " + channelName);
	std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + targetNick + " :" + channelName;
	sendReply(targetClient->getFd(), inviteMsg);
}
void ClientHandler::processCommand(Client &client, const std::string &command, std::map<int, Client *> clients)
{
	std::vector<std::string> args = parseCommand(command);

	if (args.empty())
		return;

	std::string cmd = args[0];
	std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::toupper);

	if (client.getStatus() == Client::PENDING)
	{
		if (cmd == "CAP")
			return;
		if (cmd == "PASS")
			handlePass(client, args);
		else if (cmd == "QUIT")
			handleQuit(client, args);
		else
			sendReply(client.getFd(), ":server 451 * :You must send a password first (PASS <password>)");
		return;
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
		handleMsg(client, args, clients);
	else if (cmd == "JOIN")
		handleJoin(client, args);
	else if (cmd == "INVITE")
		handleInvite(client, args, clients);
	else if (cmd == "KICK")
		handleKick(client, args, clients);
	else if (cmd == "QUIT")
		handleQuit(client, args);
	else if (cmd == "NICK")
		handleNick(client, args);
	else if (cmd == "MODE")
		handleMode(client, args);
	else if (cmd == "TOPIC")
		handleTopic(client, args);
	else if (cmd == "USER")
		sendReply(client.getFd(), ":server 462 " + client.getNickname() + " :You may not reregister");
	else
		sendReply(client.getFd(), ":server 421 " + client.getNickname() + " " + cmd + " :Unknown command");
}
