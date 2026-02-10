#include "User.hpp"

User::User(Server *server) : ACommand(server)
{

}

void User::execute(Client &client, const std::vector<std::string> &args)
{
	if (client.getStatus() == Client::REGISTERED)
	{
		sendReply(client.getFd(), ":server 462 " + client.getNickname() + " :You may not reregister");
		return;
	}
	if (args.size() < 5)
	{
		sendReply(client.getFd(), ":server 461 * USER :Not enough parameters");
		return;
	}

	client.setUsername(args[1]);
	client.setRealname(args[4]);

	this->checkRegistration(client);
}
