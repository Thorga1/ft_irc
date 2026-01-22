#include "server.hpp"

server::server(std::string password, unsigned int port) : _password(password), _port(port) {}

server::~server() {}


unsigned int server::getport()
{
	return _port;
}

std::string server::getpassword()
{
	return _password;
}