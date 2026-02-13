#pragma once

#include <string>
#include <map>
#include "Client_handler.hpp"

class Channel
{
private:
	std::map<int, Client *> _admin; // <NOM DE LUTILISATEUR, FD>
	std::map<int, Client *> _users;
	std::map<int, Client *> _kicked;
	int _creatorFd;
	std::string _topicStr;
	std::vector<std::string> _invitedUsers;
	bool _i;        // invite SEULEMENT les admins peuvent inviter
	bool _t;        // topic SEULEMENT les admins peuvent changer le topic
	bool _k;        // pour voir si le channel est protege par mot de passe
	std::string _key; // mot de passe du channel si k est activé
	int _l;          //  limite le nombre de membres dans le channel et est egal a -1 si pas defini
public:
	Channel();
	Channel(int creatorFd);
	Channel(int creatorFd, std::string str);
	Channel(const Channel &other);
	~Channel();
	Channel &operator=(const Channel &other);
	bool isInInvitedUsers(std::string userId);
	bool isInviteOnly();
	void kick(std::string clientId);
	void invite(std::string clientId);
	void topic(std::string newTopic);
	int mode(std::string token); // i t k o l PEUT ETRE +ikl ou -kl etc
	// int getUser(std::string userId);
	std::string getModes();
	;
	void setUser(std::string userId, int value);
	void broadcastMessage(const std::string &message, int senderFd);
	void setAdmin(std::string adminId, int value);
	void promoteToAdmin(std::string userId);
	void demoteFromAdmin(std::string adminId);
	void removeUser(const std::string &userId, uint8_t flag);
	void removeFromInvited(std::string userId);
	bool hasUser(const std::string &userId);
	bool hasAdmin(const std::string &adminId);
	std::string getTopic() const;

	bool isTopicProtected() const;

	void setKey(std::string key);
	std::string getKey() const;
	bool hasKey() const;

	void setLimit(int limit);
	int getLimit() const;
	int getUserCount() const;

	Client *findUser(const std::string &nickname, const std::map<int, Client *> &clients);
};
