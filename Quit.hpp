#ifndef QUIT_HPP
#define QUIT_HPP

#include "ACommand.hpp"

class Quit : public ACommand
{
public:
	Quit(Server *server);
	~Quit();
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif