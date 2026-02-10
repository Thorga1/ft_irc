#include "Mode.hpp"

Mode::Mode(Server *server) : ACommand(server)
{
}

Mode::~Mode()
{
}

void Mode::handleFlagModes(Client &client, std::string modeStr, Channel *ch, char c, bool adding)
{
	ch->mode(modeStr);
	std::string modeMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost MODE " + ch->getTopic() + " " + (adding ? "+" : "-") + c;
	ch->broadcastMessage(modeMsg + "\r\n", -1);
}

void Mode::handleLimitMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
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

void Mode::handleKeyMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
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

void Mode::handleOperatorMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding)
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

void Mode::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " MODE :Not enough parameters");
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

	if (args.size() == 2)
	{
		sendReply(client.getFd(), ":server 324 " + client.getNickname() + " " + args[1] + " :" + ch->getModes());
		return;
	}

	std::string modeStr = args[2];
	int count = 3;
	bool adding = true;

	for (size_t i = 0; i < modeStr.length(); i++)
	{
		char c = modeStr[i];
		if (c == '+')
			adding = true;
		else if (c == '-')
			adding = false;
		else if (c == 'o')
			handleOperatorMode(client, ch, args, count, adding);
		else if (c == 'k')
			handleKeyMode(client, ch, args, count, adding);
		else if (c == 'l')
			handleLimitMode(client, ch, args, count, adding);
		else if (c == 'i' || c == 't')
			handleFlagModes(client, modeStr, ch, c, adding);
		else
			sendReply(client.getFd(), ":server 472 " + client.getNickname() + " " + c + " :is unknown mode char");
	}
}