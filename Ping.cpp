#include "Ping.hpp"

ping::ping(Server *server) : ACommand(server) {}

ping::~ping() {}

void ping::execute(Client &client, const std::vector<std::string> &args)
{
	if (args.size() != 2)
		return (sendReply(client.getFd(), ":server 421 " + client.getNickname() + " " + "PING" + " :Unknown command"));
	std::string inviteMsg = "PONG: " + args[args.size() - 1];
	sendReply(client.getFd(), inviteMsg);
}