#include "Nick.hpp"

Nick::Nick(Server *server) : ACommand(server)
{
}

Nick::~Nick()
{
}

void Nick::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() < 2)
	{
		sendReply(client.getFd(), ":server 431 * :No nickname given");
		return;
	}

	std::string newNick = args[1];

	if (client.getNickname() == newNick)
		return;

	if (_server->isNickInUse(newNick))
	{
		std::string current = client.getNickname().empty() ? "*" : client.getNickname();
		sendReply(client.getFd(), ":server 433 " + current + " " + newNick + " :Nickname is already in use");
		return;
	}

	std::cout << "[NICK] FD " << client.getFd() << " changed nickname to: " << newNick << std::endl;

	client.setNickname(newNick);

	if (client.getStatus() != Client::REGISTERED)
		checkRegistration(client);
}

void Nick::checkRegistration(Client &client)
{
	if (client.getStatus() == Client::AUTHENTICATED &&
		!client.getNickname().empty() && !client.getUsername().empty())
	{
		client.setStatus(Client::REGISTERED);
		std::string nick = client.getNickname();

		sendReply(client.getFd(), ":server 001 " + nick + " :Welcome to the IRC Network, " + nick);

		std::cout << "[SUCCESS] Client '" << nick << "' (FD: " << client.getFd()
				  << ") is now registered." << std::endl;
	}
}
