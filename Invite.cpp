#include "Invite.hpp"

Invite::Invite(Server *server) : ACommand(server) {}
Invite::~Invite() {}

void Invite::execute(Client &client, const std::vector<std::string> &args) {
    if (args.size() < 3) {
        sendReply(client.getFd(), ":server 461 " + client.getNickname() + " INVITE :Not enough parameters");
        return;
    }

    std::string targetNick = args[1];
    std::string channelName = args[2];

    Client *targetClient = _server->getClientByNick(targetNick);
    if (!targetClient) {
        sendReply(client.getFd(), ":server 401 " + client.getNickname() + " " + targetNick + " :No such nick");
        return;
    }

    Channel *ch = _server->findChannel(channelName);
    if (!ch) {
        sendReply(client.getFd(), ":server 403 " + client.getNickname() + " " + channelName + " :No such channel");
        return;
    }

    if (!ch->hasUser(client.getNickname()) && !ch->hasAdmin(client.getNickname())) {
        sendReply(client.getFd(), ":server 442 " + client.getNickname() + " " + channelName + " :You're not on that channel");
        return;
    }
    if (ch->isInviteOnly() && !ch->hasAdmin(client.getNickname())) {
        sendReply(client.getFd(), ":server 482 " + client.getNickname() + " " + channelName + " :You're not channel operator");
        return;
    }

    if (ch->hasUser(targetNick) || ch->hasAdmin(targetNick)) {
        sendReply(client.getFd(), ":server 443 " + client.getNickname() + " " + targetNick + " " + channelName + " :is already on channel");
        return;
    }

    ch->invite(targetNick);

    sendReply(client.getFd(), ":server 341 " + client.getNickname() + " " + targetNick + " " + channelName);

    std::string inviteMsg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + targetNick + " :" + channelName + "\r\n";
    send(targetClient->getFd(), inviteMsg.c_str(), inviteMsg.size(), 0);
}