#include "irc.hpp"

int main(int ac, char **av)
{
	try
	{
		if (ac != 3)
			throw std::runtime_error("wrong paramater given");
		else if (ac == 3)
			parse_av(av);
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}

}