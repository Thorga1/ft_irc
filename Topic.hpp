#ifndef TOPIC_HPP
#define TOPIC_HPP

#include "ACommand.hpp"

class Topic : public ACommand
{
public:
	Topic(Server *server);
	~Topic();
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif