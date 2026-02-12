#pragma once

#include "ACommand.hpp"

class Invite : public ACommand {
public:
	Invite();
	Invite(Server *server);
	Invite(const Invite &other);
	~Invite();
	Invite &operator=(const Invite &other);
	void execute(Client &client, const std::vector<std::string> &args);
};
