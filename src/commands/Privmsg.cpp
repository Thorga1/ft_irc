#include "Privmsg.hpp"

Privmsg::Privmsg() : ACommand(NULL)
{
}

Privmsg::Privmsg(Server *server) : ACommand(server)
{
}

Privmsg::Privmsg(const Privmsg &other) : ACommand(other)
{
}

Privmsg::~Privmsg()
{
}

Privmsg &Privmsg::operator=(const Privmsg &other)
{
	ACommand::operator=(other);
	return *this;
}

void Privmsg::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 411 " + client.getNickname() + " :No recipient given (PRIVMSG)");
		return;
	}
	if (args.size() < 3 || args[2].empty())
	{
		sendReply(client.getFd(), ":server 412 " + client.getNickname() + " :No text to send");
		return;
	}

	std::string target = args[1];
	std::string message = args[2];

	std::string prefix = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost";

	if (target[0] == '#' || target[0] == '&')
	{
		Channel *ch = _server->findChannel(target);
		if (!ch)
		{
			sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + target + " :No such channel");
			return;
		}

		if (!ch->hasUser(client.getNickname()) && !ch->hasAdmin(client.getNickname()))
		{
			sendReply(client.getFd(), ":server 404 " + client.getNickname() + " " + target + " :Cannot send to channel");
			return;
		}

		std::string formatted = prefix + " PRIVMSG " + target + " :" + message + "\r\n";

		ch->broadcastMessage(formatted, client.getFd());
	}
	else
	{
		Client *receiver = _server->getClientByNick(target);

		if (!receiver)
		{
			sendReply(client.getFd(), ":server 401 " + client.getNickname() + " " + target + " :No such nick");
			return;
		}

		std::string formatted = prefix + " PRIVMSG " + target + " :" + message + "\r\n";

		if (send(receiver->getFd(), formatted.c_str(), formatted.size(), 0) == -1)
		{
			std::cerr << "Error: Failed to send PRIVMSG to " << target << std::endl;
		}
	}
}
