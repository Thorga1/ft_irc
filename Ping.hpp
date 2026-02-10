#pragma once

#include "ACommand.hpp"

class ping : public ACommand
{
public:
	ping(Server *server);
	~ping();
	void execute(Client &client, const std::vector<std::string> &args);
};