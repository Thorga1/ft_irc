#include "Quit.hpp"

Quit::Quit() : ACommand(NULL) {}

Quit::Quit(Server *server) : ACommand(server) {}

Quit::Quit(const Quit &other) : ACommand(other) {}

Quit::~Quit() {}

Quit &Quit::operator=(const Quit &other)
{
	ACommand::operator=(other);
	return *this;
}

void Quit::execute(Client &client, const std::vector<std::string> &args)
{
    std::string reason = (args.size() > 1) ? args[1] : "Client quit";
    sendReply(client.getFd(), "ERROR :Closing Link: " + client.getNickname() + " (" + reason + ")");
    client.setStatus(Client::DISCONNECTED);

    std::cout << "[QUIT] Client " << client.getNickname() << " has quit (" << reason << ")." << std::endl;
}
