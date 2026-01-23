#include "channel.hpp"

int Channel::getUser(std::string userId){
    return this->users[userId];
}

void Channel::setUser(std::string userId, int value){
    this->users[userId] = value;
}

Channel::Channel() : creatorFd(-1) {
}

Channel::Channel(int creatorFd) : creatorFd(creatorFd)
{
    this->topic_str = "default_channel";
    std::string confirmation = "Created " + this->topic_str + " as a new Channel.\r\n";
    send(creatorFd, confirmation.c_str(), confirmation.length(), 0);
}


Channel::~Channel()
{

}
