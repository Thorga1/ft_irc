#ifndef JOIN_HPP
#define JOIN_HPP

#include "ACommand.hpp"

class Join : public ACommand
{
public:
	Join(Server *server) : ACommand(server) {}
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif