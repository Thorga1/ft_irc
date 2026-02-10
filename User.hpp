#ifndef USER_HPP
#define USER_HPP

#include "ACommand.hpp"

class User : public ACommand
{
public:
	User(Server *server);
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif