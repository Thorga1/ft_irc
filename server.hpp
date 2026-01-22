#pragma once

#include <iostream>
#include <cstdlib>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <vector>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>

class server
{
private:
	std::string		_password;
	unsigned int	_port;

public:
	server(std::string password, unsigned int port);
	unsigned int getport();
	std::string getpassword();
	~server();
};

server	parse_av(char **av);
void ft_main_socket(server serv);