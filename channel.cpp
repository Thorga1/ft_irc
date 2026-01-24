#include "channel.hpp"
#include <sys/socket.h>

// int Channel::getUser(std::string userId)
// {
// 	return this->_users->_fd;
// }

void Channel::setUser(std::string userId, int value)
{
	ClientHandler a;

	if (a.findUser(userId, this->_users) != NULL || a.findUser(userId, this->_admin) != NULL)
	{
		Client *tmp;
		tmp = a.findUser(userId, this->_users);
		if (tmp == NULL)
			throw std::runtime_error("jkdslahgfdsuxc");
		tmp->setFd(value);

	}
	else
	{
		std::string error_msg = "Error: User " + userId + " is already in the channel.\r\n";
		send(value, error_msg.c_str(), error_msg.length(), 0);
	}
}

Channel::Channel() : _creatorFd(-1)
{
}

Channel::Channel(int creatorFd, int chanelid) : _creatorFd(creatorFd), _channelId(chanelid)
{
	this->_topicStr = "default_channel";
	std::string confirmation = "Created " + this->_topicStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

Channel::Channel(int creatorFd, std::string str, int chanelid) : _creatorFd(creatorFd), _channelId(chanelid), _topicStr(str)
{
	std::string confirmation = "Created " + this->_topicStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

void Channel::kick(char)
{
}

void Channel::invite(char)
{
}

void Channel::topic(std::string)
{
}

void Channel::mode(std::string)
{
}

void Channel::broadcastMessage(const std::string &message, int senderFd)
{
	for (std::map<int, Client *>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->second->getFd() != senderFd)
		{
			send(it->second->getFd(), message.c_str(), message.length(), 0);
		}
	}
	(void)_creatorFd;
	(void)_channelId;
}

Channel::~Channel()
{
}

bool Channel::hasUser(const std::string &userId)
{
	ClientHandler a;

	return a.findUser(userId, this->_users) != NULL || a.findUser(userId, this->_admin);
}

