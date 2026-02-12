#pragma once

#include "ACommand.hpp"

class Topic : public ACommand
{
public:
	Topic();
	Topic(Server *server);
	Topic(const Topic &other);
	~Topic();
	Topic &operator=(const Topic &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
