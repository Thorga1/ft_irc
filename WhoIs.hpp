#pragma once

#include "ACommand.hpp"

class WhoIs : public ACommand
{
public:
	WhoIs(Server *server);
	~WhoIs();
	void execute(Client &client, const std::vector<std::string> &args);
};