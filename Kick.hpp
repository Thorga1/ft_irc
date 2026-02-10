#ifndef KICK_HPP
#define KICK_HPP

#include "ACommand.hpp"

class Kick : public ACommand
{
public:
	Kick(Server *server);
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif