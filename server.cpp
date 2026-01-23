#include "server.hpp"

Server::Server(std::string password, unsigned int port)
	: _password(password), _port(port), _socket_fd(-1)
{
}

Server::~Server()
{
	stop();
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
}

unsigned int Server::getPort() const
{
	return _port;
}

std::string Server::getPassword() const
{
	return _password;
}

std::map<int, Client*> Server::getClients() const
{
	return _clients;
}

void Server::start()
{
	struct sockaddr_in sin;

	_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket_fd < 0)
		throw std::runtime_error("Error: Failed to create socket");

	int reuse = 1;
	if (setsockopt(_socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
		throw std::runtime_error("Error: setsockopt failed");

	sin.sin_family = AF_INET;
	sin.sin_port = htons(_port);
	sin.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(_socket_fd, (struct sockaddr*)&sin, sizeof(sin)) < 0)
		throw std::runtime_error("Error: Bind failed");

	if (listen(_socket_fd, SOMAXCONN) < 0)
		throw std::runtime_error("Error: Listen failed");

	fcntl(_socket_fd, F_SETFL, O_NONBLOCK);

	pollfd server_fd;
	server_fd.fd = _socket_fd;
	server_fd.events = POLLIN;
	_fds.push_back(server_fd);

	std::cout << "IRC Server started on port " << _port << std::endl;
}

void Server::acceptNewClient()
{
	struct sockaddr_in client_addr;
	socklen_t client_len = sizeof(client_addr);

	int client_fd = accept(_socket_fd, (sockaddr*)&client_addr, &client_len);
	if (client_fd < 0)
		return;
	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	pollfd client_poll;
	client_poll.fd = client_fd;
	client_poll.events = POLLIN;
	_fds.push_back(client_poll);

	Client* client = new Client(client_fd);
	_clients[client_fd] = client;

	std::cout << "New client connected: fd=" << client_fd << " from "
			  << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
}

void Server::handleClientData(size_t fd_index, std::map<int, Client*> clients)
{
	int fd = _fds[fd_index].fd;
	Client* client = _clients[fd];

	char buffer[4096];
	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);
	if (bytes <= 0)
	{
		std::cout << "Client disconnected: fd=" << fd << std::endl;
		removeClient(fd_index);
		return;
	}

	buffer[bytes] = '\0';
	std::string data(buffer);
	size_t pos = 0;
	while (pos < data.length())
	{
		size_t end = data.find('\n', pos);
		if (end == std::string::npos)
			break;

		std::string command = data.substr(pos, end - pos);
		if (!command.empty() && command[command.length() - 1] == '\r')
			command.erase(command.length() - 1);

		if (!command.empty())
			_handler.processCommand(*client, command, clients);

		pos = end + 1;
	}
}

void Server::removeClient(size_t fd_index)
{
	int fd = _fds[fd_index].fd;
	if (_clients.find(fd) != _clients.end())
	{
		delete _clients[fd];
		_clients.erase(fd);
	}
	close(fd);
	_fds.erase(_fds.begin() + fd_index);
}

void Server::run()
{
	std::cout << "IRC Server running... (press Ctrl+C to stop)" << std::endl;

	while (true)
	{
		int ret = poll(_fds.data(), _fds.size(), -1);

		if (ret < 0)
			throw std::runtime_error("Error: Poll failed");

		if (ret == 0)
			continue;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents == 0)
				continue;
			if (_fds[i].fd == _socket_fd && (_fds[i].revents & POLLIN))
				acceptNewClient();
			else if (_fds[i].revents & POLLIN)
				handleClientData(i, getClients());
			else if (_fds[i].revents & (POLLHUP | POLLERR))
			{
				std::cout << "Error or hangup on fd=" << _fds[i].fd << std::endl;
				removeClient(i);
				i--;
			}
		}
	}
}

void Server::stop()
{
	if (_socket_fd >= 0)
	{
		close(_socket_fd);
		_socket_fd = -1;
	}

	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
	_fds.clear();

	std::cout << "IRC Server stopped" << std::endl;
}

bool isPortNumber(const std::string& str)
{
	if (str.empty())
		return false;
	for (int i = 0; char c = str[i]; i++)
	{
		if (!std::isdigit(c))
			return false;
	}
	return true;
}

bool isValidPort(unsigned int port)
{
	return port >= 1 && port <= 65535;
}

Server parseArguments(char **av)
{
	if (!av[1] || !av[1][0])
		throw std::runtime_error("Error: No port specified");

	if (!isPortNumber(av[1]))
		throw std::runtime_error("Error: Port must contain only digits");

	if (!av[2] || !av[2][0])
		throw std::runtime_error("Error: No password specified");

	unsigned int port = static_cast<unsigned int>(std::atoi(av[1]));

	if (!isValidPort(port))
		throw std::runtime_error("Error: Port must be between 1 and 65535");

	return Server(av[2], port);
}