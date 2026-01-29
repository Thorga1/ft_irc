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
		this->_users.erase(fd);
	}
}

void Channel::mode(std::string str)
{
	if (str.find("+i") != std::string::npos)
	{
		this->_i = true; // JUSTE POUR TESTER INVITE ONLY
	}
	else if (str.find("-i") != std::string::npos)
	{
		this->_i = false;
	}
	// RAJOUTEZ LES MODES
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

void Channel::promoteToAdmin(std::string userId, int value)
{
	ClientHandler a;
	Client *user = a.findUser(userId, this->_users);
	if (user != NULL)
	{
		this->_admin[value] = user;
		this->_users.erase(value);
	}
}

void Channel::demoteFromAdmin(std::string adminId)
{
	ClientHandler a;
	Client *admin = a.findUser(adminId, this->_admin);
	if (admin != NULL)
	{
		this->_users[admin->getFd()] = admin;
		this->_admin.erase(admin->getFd());
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
}

Channel::~Channel()
{
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
	ClientHandler a;

	return a.findUser(adminId, this->_admin) != NULL;
}