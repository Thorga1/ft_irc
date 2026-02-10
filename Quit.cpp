#include "Quit.hpp"

Quit::Quit(Server *server) : ACommand(server) {}
Quit::~Quit() {}

void Quit::execute(Client &client, const std::vector<std::string> &args)
{
    std::string reason = (args.size() > 1) ? args[1] : "Client quit";
    sendReply(client.getFd(), "ERROR :Closing Link: " + client.getNickname() + " (" + reason + ")");
    client.setStatus(Client::DISCONNECTED);

    std::cout << "[QUIT] Client " << client.getNickname() << " has quit (" << reason << ")." << std::endl;
}