#pragma once

#include "client.hpp"
#include "client_handler.hpp"
#include "server.hpp"

class Channel
{
    private:
        std::map<std::string, int> admin; // <NOM DE LUTILISATEUR, FD>
        std::map<std::string, int> users;
        std::string topic_str;
        int creatorFd;
    public:
        Channel();
        Channel(int creatorFd);
        ~Channel();
        void kick(char user);
        void invite(char user);
        void topic(std::string newTopic);
        void mode(std::string token); // i t k o l PEUT ETRE +ikl ou -kl etc
        int getUser(std::string userId);
        void setUser(std::string userId, int value);
};