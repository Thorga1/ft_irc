#ifndef NICK_HPP
#define NICK_HPP

#include "ACommand.hpp"

class Nick : public ACommand
{
public:
	Nick(Server *server);
	~Nick();
	void execute(Client &client, const std::vector<std::string> &args);

private:
	void checkRegistration(Client &client);
};

#endif