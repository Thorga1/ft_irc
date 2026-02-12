#include "Kick.hpp"

Kick::Kick() : ACommand(NULL)
{

}

Kick::Kick(Server *server) : ACommand(server)
{

}

Kick::Kick(const Kick &other) : ACommand(other)
{

}

Kick::~Kick()
{

}

Kick &Kick::operator=(const Kick &other)
{
	ACommand::operator=(other);
	return *this;
}

void Kick::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 3)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " KICK :Not enough parameters");
		return;
	}

	Channel *ch = _server->findChannel(args[1]);
	if (!ch)
	{
		sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + args[1] + " :No such channel");
		return;
	}
	if (!ch->hasAdmin(client.getNickname()))
	{
		sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + args[1] + " :You're not channel operator");
		return;
	}

	std::string targetNick = args[2];
	if (!ch->hasUser(targetNick) && !ch->hasAdmin(targetNick))
	{
		sendReply(client.getFd(), ":server 441 " + client.getNickname() + " " + targetNick + " " + args[1] + " :They aren't on that channel");
		return;
	}

	std::string kickMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost KICK " + args[1] + " " + targetNick + "\r\n";
	ch->broadcastMessage(kickMsg, -1);
	ch->kick(targetNick);

	if (ch->getUserCount() == 0)
		_server->removeChannel(args[1]);
}
