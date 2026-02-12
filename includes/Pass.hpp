#pragma once

#include "ACommand.hpp"

class Pass : public ACommand
{
public:
	Pass();
	Pass(Server *server);
	Pass(const Pass &other);
	~Pass();
	Pass &operator=(const Pass &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
