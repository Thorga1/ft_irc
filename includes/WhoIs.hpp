#pragma once

#include "ACommand.hpp"

class WhoIs : public ACommand
{
public:
	WhoIs();
	WhoIs(Server *server);
	WhoIs(const WhoIs &other);
	~WhoIs();
	WhoIs &operator=(const WhoIs &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
