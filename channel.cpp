#include "channel.hpp"
#include <sys/socket.h>

// int Channel::getUser(std::string userId)
// {
// 	return this->_users->_fd;
// }

bool Channel::isInInvitedUsers(std::string userId)
{
	for (size_t i = 0; i < _invitedUsers.size(); ++i)
	{
		if (_invitedUsers[i] == userId)
			return true;
	}
	return false;
}

bool Channel::isInviteOnly()
{
	return this->_i;
}

std::string Channel::getModes()
{
	std::string modes = "+";
	if (this->_i)
		modes += "i";
	if (this->_t)
		modes += "t";
	if (this->_k)
		modes += "k";
	if (this->_l != -1)
		modes += "l";
	return modes;
}

void Channel::setUser(std::string userId, int value)
{
	ClientHandler a;

	if (a.findUser(userId, this->_users) != NULL)
	{
		std::string error_msg = "Error: User " + userId + " is already in the channel.\r\n";
		send(value, error_msg.c_str(), error_msg.length(), 0);
	}
	else
	{
		Client *newUser = new Client(value);
		newUser->setNickname(userId);
		this->_users[value] = newUser;
	}
}

void Channel::setAdmin(std::string userId, int value)
{
	ClientHandler a;

	if (a.findUser(userId, this->_admin) != NULL)
	{
		std::string error_msg = "Error: User " + userId + " is already in the channel.\r\n";
		send(value, error_msg.c_str(), error_msg.length(), 0);
	}
	else
	{
		Client *newUser = new Client(value);
		newUser->setNickname(userId);
		this->_admin[value] = newUser;
	}
}

void Channel::kick(std::string str)
{
	ClientHandler a;
	Client *user = a.findUser(str, this->_users);
	if (!this->hasAdmin(str) && this->hasUser(str))
	{
		int fd = user->getFd();
		delete user;
		this->_users.erase(fd);
	}
}

bool Channel::isTopicProtected() const
{
	return _t;
}

int Channel::mode(std::string str)
{
	bool adding = true;
	int i = 0;

	while ((size_t)i < str.length())
	{
		char c = str[i];
		if (c == '+')
			adding = true;
		else if (c == '-')
			adding = false;
		else if (c == 'i')
			_i = adding;
		else if (c == 't')
			_t = adding;
		i++;
	}
	return i;
}

void Channel::removeFromInvited(std::string clientId)
{
	for (std::vector<std::string>::iterator it = _invitedUsers.begin(); it != _invitedUsers.end(); ++it)
	{
		if (*it == clientId)
		{
			_invitedUsers.erase(it);
			return;
		}
	}
}

void Channel::invite(std::string clientId)
{
	if (this->hasUser(clientId) || this->hasAdmin(clientId) || this->isInInvitedUsers(clientId))
		return;

	_invitedUsers.push_back(clientId);
}

void Channel::promoteToAdmin(std::string userId)
{
	std::map<int, Client *>::iterator it;
	for (it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->second->getNickname() == userId)
		{
			_admin[it->first] = it->second;
			_users.erase(it);
			return;
		}
	}
}

void Channel::demoteFromAdmin(std::string adminId)
{
	std::map<int, Client *>::iterator it;
	for (it = _admin.begin(); it != _admin.end(); ++it)
	{
		if (it->second->getNickname() == adminId)
		{
			_users[it->first] = it->second;
			_admin.erase(it);
			return;
		}
	}
}

Channel::Channel() : _creatorFd(-1),
					 _topicStr(""),
					 _i(false),
					 _t(false),
					 _k(false),
					 _key(""),
					 _l(-1)
{
}

Channel::Channel(int creatorFd) : _creatorFd(creatorFd), _topicStr("default_channel"), _i(false),
								  _t(false),
								  _k(false),
								  _key(""),
								  _l(-1)
{
	std::string confirmation = "Created " + this->_topicStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

Channel::Channel(int creatorFd, std::string str) : _creatorFd(creatorFd), _topicStr(str), _i(false),
												   _t(false),
												   _k(false),
												   _key(""),
												   _l(-1)
{
	std::string confirmation = "Created " + this->_topicStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

void Channel::topic(std::string newTopic)
{
	this->_topicStr = newTopic;
}

void Channel::broadcastMessage(const std::string &message, int senderFd)
{
	for (std::map<int, Client *>::iterator it = _users.begin(); it != _users.end(); it++)
	{
		if (it->second->getFd() != senderFd)
		{
			send(it->second->getFd(), message.c_str(), message.length(), 0);
		}
	}
	for (std::map<int, Client *>::iterator it = _admin.begin(); it != _admin.end(); it++)
	{
		if (it->second->getFd() != senderFd)
		{
			send(it->second->getFd(), message.c_str(), message.length(), 0);
		}
	}
}

Channel::~Channel()
{
	for (std::map<int, Client *>::iterator it = _users.begin(); it != _users.end(); ++it)
		delete it->second;
	_users.clear();
	for (std::map<int, Client *>::iterator it = _admin.begin(); it != _admin.end(); ++it)
		delete it->second;
	_admin.clear();
}

void Channel::removeUser(const std::string &userId)
{
	for (std::map<int, Client *>::iterator it = _users.begin(); it != _users.end(); ++it)
	{
		if (it->second->getNickname() == userId)
		{
			delete it->second;
			_users.erase(it);
			return;
		}
	}
	for (std::map<int, Client *>::iterator it = _admin.begin(); it != _admin.end(); ++it)
	{
		if (it->second->getNickname() == userId)
		{
			delete it->second;
			_admin.erase(it);
			return;
		}
	}
}

std::string Channel::getTopic() const
{
	return this->_topicStr;
}

bool Channel::hasUser(const std::string &userId)
{
	ClientHandler a;

	return a.findUser(userId, this->_users) != NULL;
}

bool Channel::hasAdmin(const std::string &adminId)
{
	for (std::map<int, Client *>::iterator it = _admin.begin(); it != _admin.end(); ++it)
	{
		if (it->second->getNickname() == adminId)
			return true;
	}
	return false;
}

void Channel::setKey(std::string key)
{
	this->_key = key;
	this->_k = !key.empty();
}

std::string Channel::getKey() const
{
	return this->_key;
}

bool Channel::hasKey() const
{
	return this->_k;
}

void Channel::setLimit(int limit)
{
	this->_l = limit;
}

int Channel::getLimit() const
{
	return this->_l;
}

int Channel::getUserCount() const
{
	return _admin.size() + _users.size();
}