#include "server.hpp"


int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::runtime_error("wrong paramater given");
		else if (ac == 3)
		{
			server serv = parse_av(av);
			ft_main_socket(serv);
		}
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
		return (1);
	}

}