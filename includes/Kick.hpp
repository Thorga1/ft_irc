#pragma once

#include "ACommand.hpp"

class Kick : public ACommand
{
public:
	Kick();
	Kick(Server *server);
	Kick(const Kick &other);
	~Kick();
	Kick &operator=(const Kick &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
