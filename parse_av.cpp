#include "irc.hpp"

void	parse_av(char **av)
{
	if (av[1][0])
		for (int i = 0; av[1][i]; i++)
		{
			if (!std::isdigit(av[1][i]))
				throw std::runtime_error("error: <port> contains not only digits");
			if (!av[2][0])
				throw std::runtime_error("error: no password been set");
		}

	else
		throw std::runtime_error("invalid <port>");
	server serv(av[2], (unsigned int)std::atoi(av[1]));
	if (serv.getport() < 1024 || serv.getport() > 65535)
		throw std::runtime_error("error: port not valid");
}