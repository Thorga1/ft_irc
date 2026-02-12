#include "Topic.hpp"

Topic::Topic() : ACommand(NULL) {}

Topic::Topic(Server *server) : ACommand(server) {}
Topic::Topic(const Topic &other) : ACommand(other) {}
Topic::~Topic() {}

Topic &Topic::operator=(const Topic &other)
{
	ACommand::operator=(other);
	return *this;
}

void Topic::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " TOPIC :Not enough parameters");
		return;
	}

	std::string channelName = args[1];
	Channel *ch = _server->findChannel(channelName);

	if (!ch)
	{
		sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel");
		return;
	}

	if (!ch->hasUser(client.getNickname()) && !ch->hasAdmin(client.getNickname()))
	{
		sendReply(client.getFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
		return;
	}

	if (args.size() == 2)
	{
		std::string topic = ch->getTopic();
		if (topic.empty())
			sendReply(client.getFd(), ":server 331 " + client.getNickname() + " " + channelName + " :No topic is set");
		else
			sendReply(client.getFd(), ":server 332 " + client.getNickname() + " " + channelName + " :" + topic);
	}
	else
	{
		if (ch->isTopicProtected() && !ch->hasAdmin(client.getNickname()))
		{
			sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
			return;
		}

		std::string newTopic = args[2];
		ch->topic(newTopic);

		std::string topicMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channelName + " :" + newTopic + "\r\n";
		ch->broadcastMessage(topicMsg, -1);
	}
}
