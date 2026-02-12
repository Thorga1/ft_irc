#pragma once

#include "ACommand.hpp"

class Quit : public ACommand
{
public:
	Quit();
	Quit(Server *server);
	Quit(const Quit &other);
	~Quit();
	Quit &operator=(const Quit &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
