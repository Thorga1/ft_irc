#pragma once

#include "ACommand.hpp"

class Nick : public ACommand
{
public:
	Nick();
	Nick(Server *server);
	Nick(const Nick &other);
	~Nick();
	Nick &operator=(const Nick &other);
	void execute(Client &client, const std::vector<std::string> &args);

private:
	void checkRegistration(Client &client);
};
