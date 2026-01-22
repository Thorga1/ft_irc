#pragma once

#include <iostream>
#include <cstdlib>

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

void	parse_av(char **av);