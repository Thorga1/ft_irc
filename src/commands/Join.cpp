#include "Join.hpp"

Join::Join() : ACommand(NULL) {}

Join::Join(const Join &other) : ACommand(other) {}

Join::~Join() {}

Join &Join::operator=(const Join &other)
{
	ACommand::operator=(other);
	return *this;
}

void Join::execute(Client &client, const std::vector<std::string> &args)
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
