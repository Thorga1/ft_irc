#pragma once

#include "ACommand.hpp"

class Join : public ACommand
{
public:
	Join();
	Join(Server *server) : ACommand(server) {}
	Join(const Join &other);
	~Join();
	Join &operator=(const Join &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
