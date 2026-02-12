#pragma once

#include "ACommand.hpp"

class User : public ACommand
{
public:
	User();
	User(Server *server);
	User(const User &other);
	~User();
	User &operator=(const User &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
