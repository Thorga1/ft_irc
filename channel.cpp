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
	return this->i;
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

Channel::Channel() : _creatorFd(-1), i(false)
{
}

Channel::Channel(int creatorFd, int chanelid) : _creatorFd(creatorFd), _channelId(chanelid), _topicStr(""), i(false)
{
	this->_channelIdStr = "default_channel";
	std::string confirmation = "Created " + this->_channelIdStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

Channel::Channel(int creatorFd, std::string str, int chanelid) : _creatorFd(creatorFd), _channelId(chanelid), _topicStr(""), _channelIdStr(str), i(false)
{
	std::string confirmation = "Created " + this->_channelIdStr + " as a new Channel.\r\n";
	send(_creatorFd, confirmation.c_str(), confirmation.length(), 0);
}

void Channel::kick(std::string )
{
}

void Channel::topic(std::string newTopic)
{
	this->_topicStr = newTopic;
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