#ifndef PASS_HPP
#define PASS_HPP

#include "ACommand.hpp"

class Pass : public ACommand
{
public:
	Pass(Server *server);
	~Pass();
	void execute(Client &client, const std::vector<std::string> &args);
};

#endif