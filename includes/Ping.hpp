#pragma once

#include "ACommand.hpp"

class ping : public ACommand
{
public:
	ping();
	ping(Server *server);
	ping(const ping &other);
	~ping();
	ping &operator=(const ping &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
