#include "server.hpp"
#include <poll.h>

server	parse_av(char **av)
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
	return (serv);
}

void ft_main_socket(server serv)
{
	struct sockaddr_in sin;
	int n;

	sin.sin_family = AF_INET;
	sin.sin_port = htons(serv.getport());
	inet_aton("127.0.0.1", &sin.sin_addr);


	n = socket(AF_INET, SOCK_STREAM, 0);
	bind(n, (struct sockaddr*)&sin, sizeof(sin));

	std::vector<pollfd> fds;
	while (1)
	{
		int ret = poll(fds.data(), fds.size(), -1);
		if (ret < 0)
			throw std::runtime_error("error: poll");
		
	}
}