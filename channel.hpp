#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include "client_handler.hpp"

class Channel
{
private:
	std::map<int, Client *> _admin; // <NOM DE LUTILISATEUR, FD>
	std::map<int, Client *> _users;
	int _creatorFd;
	unsigned int _channelId;
	std::string	_topicStr;

public:
	Channel();
	Channel(int creatorFd, int chanelid);
	Channel(int creatorFd, std::string str, int chanelid);
	~Channel();
	void kick(char user);
	void invite(char user);
	void topic(std::string newTopic);
	void mode(std::string token); // i t k o l PEUT ETRE +ikl ou -kl etc
	// int getUser(std::string userId);
	void setUser(std::string userId, int value);
	void broadcastMessage(const std::string &message, int senderFd);

	bool hasUser(const std::string &userId);
};

#endif