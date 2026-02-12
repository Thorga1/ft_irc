#pragma once

#include "ACommand.hpp"

class Privmsg : public ACommand
{
public:
	Privmsg();
	Privmsg(Server *server);
	Privmsg(const Privmsg &other);
	~Privmsg();
	Privmsg &operator=(const Privmsg &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
