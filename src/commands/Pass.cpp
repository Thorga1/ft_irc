#include "Pass.hpp"

Pass::Pass() : ACommand(NULL)
{

}

Pass::Pass(Server *server) : ACommand(server)
{

}

Pass::Pass(const Pass &other) : ACommand(other)
{

}

Pass::~Pass()
{
	
}

Pass &Pass::operator=(const Pass &other)
{
	ACommand::operator=(other);
	return *this;
}

void Pass::execute(Client &client, const std::vector<std::string> &args)
{
	if (client.getStatus() != Client::PENDING)
	{
		sendReply(client.getFd(), ":server 462 * :You may not reregister");
		return;
	}

	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 * PASS :Not enough parameters");
		return;
	}

	if (args[1] == _server->getPassword())
		client.setStatus(Client::AUTHENTICATED);
	else
		sendReply(client.getFd(), ":server 464 * :Password incorrect");
}
