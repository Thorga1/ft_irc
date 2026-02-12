#include "server.hpp"

int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::runtime_error("Usage: ./ft_irc <port> <password>");

		Server server = parseArguments(av);
		std::signal(SIGINT, Server::handleSignal);
		std::signal(SIGTERM, Server::handleSignal);
		server.start();
		server.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
