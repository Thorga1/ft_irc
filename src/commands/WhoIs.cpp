#include "WhoIs.hpp"

WhoIs::WhoIs() : ACommand(NULL) {}

WhoIs::WhoIs(Server *server) : ACommand(server) {}

WhoIs::WhoIs(const WhoIs &other) : ACommand(other) {}

WhoIs::~WhoIs() {}

WhoIs &WhoIs::operator=(const WhoIs &other)
{
	ACommand::operator=(other);
	return *this;
}

void WhoIs::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 461 " + client.getNickname() + " WHOIS :Not enough parameters");
		return;
	}

	std::string targetNick = args[1];
	Client *targetClient = _server->getClientByNick(targetNick);

	if (!targetClient)
	{
		sendReply(client.getFd(), ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick/channel");
		return;
	}
	sendReply(client.getFd(),
			  ":server 311 " + client.getNickname() + " " +
				  targetClient->getNickname() + " " +
				  targetClient->getUsername() + " " +
				  "localhost * :" + targetClient->getRealname());
	sendReply(client.getFd(),
			  ":server 318 " + client.getNickname() + " " +
				  targetClient->getNickname() + " :End of WHOIS list");
}
