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
	std::string _channelIdStr;
	std::vector<std::string> _invitedUsers;
	bool i; // invite SEULEMENT les admins peuvent inviter
	// bool t; // topic SEULEMENT les admins peuvent changer le topic
	// bool k; // pour voir si le channel est protege par mot de passe
	// std::string key; // mot de passe du channel si k est activé
	// bool o; // ops SEULEMENT les admins peuvent donner des droits admin a d'autres
	// int l; //  limite le nombre de membres dans le channel et est egal a -1 si pas defini
public:
	Channel();
	Channel(int creatorFd, int chanelid);
	Channel(int creatorFd, std::string str, int chanelid);
	~Channel();
	bool isInInvitedUsers(std::string userId);
	bool isInviteOnly();
	void kick(std::string clientId);
	void invite(std::string clientId);
	void topic(std::string newTopic);
	void mode(std::string token); // i t k o l PEUT ETRE +ikl ou -kl etc
	// int getUser(std::string userId);
	void setUser(std::string userId, int value);
	void broadcastMessage(const std::string &message, int senderFd);
	void setAdmin(std::string adminId, int value);
	void promoteToAdmin(std::string userId, int value);
	void demoteFromAdmin(std::string adminId);
	void removeFromInvited(std::string userId);
	bool hasUser(const std::string &userId);
	bool hasAdmin(const std::string &adminId);
	std::string getTopic() const;
};

#endif