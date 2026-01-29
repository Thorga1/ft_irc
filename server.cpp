#include "server.hpp"

Server::Server(std::string password, unsigned int port)
	: _password(password), _port(port), _socket_fd(-1), _handler(this)
{
}

Server::~Server()
{
	stop();
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
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

	if (bind(_socket_fd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
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

	int client_fd = accept(_socket_fd, (sockaddr *)&client_addr, &client_len);
	if (client_fd < 0)
		return;

	fcntl(client_fd, F_SETFL, O_NONBLOCK);

	pollfd client_poll;
	client_poll.fd = client_fd;
	client_poll.events = POLLIN;
	_fds.push_back(client_poll);

	Client *client = new Client(client_fd);
	_clients[client_fd] = client;

	std::cout << "New client connected: fd=" << client_fd << " from "
			  << inet_ntoa(client_addr.sin_addr) << ":" << ntohs(client_addr.sin_port) << std::endl;
}

bool Server::handleClientData(size_t fd_index)
{
	int fd = _fds[fd_index].fd;
	Client *client = _clients[fd];
	char buffer[4096];

	ssize_t bytes = recv(fd, buffer, sizeof(buffer) - 1, 0);

	if (bytes < 0)
	{
		if (errno == EAGAIN || errno == EWOULDBLOCK)
			return true;
		std::cout << "Recv error on fd=" << fd << std::endl;
		removeClient(fd_index);
		return false;
	}
	else if (bytes == 0)
	{
		std::cout << "Client disconnected: fd=" << fd << std::endl;
		removeClient(fd_index);
		return false;
	}
	buffer[bytes] = '\0';
	client->appendBuffer(buffer);

	std::string &buf = client->getBuffer();
	size_t pos;

	while ((pos = buf.find('\n')) != std::string::npos)
	{
		std::string line = buf.substr(0, pos);

		if (!line.empty() && line[line.length() - 1] == '\r')
			line.erase(line.length() - 1);

		if (!line.empty())
			_handler.processCommand(*client, line, _clients);

		buf.erase(0, pos + 1);

		if (client->getStatus() == Client::DISCONNECTED)
		{
			removeClient(fd_index);
			return false;
		}
	}
	return true;
}

void Server::removeClient(size_t fd_index)
{
	int fd = _fds[fd_index].fd;
	close(fd);
	delete _clients[fd];
	_clients.erase(fd);
	_fds.erase(_fds.begin() + fd_index);
}

void Server::run()
{
	while (true)
	{
		int ret = poll(&_fds[0], _fds.size(), -1);
		if (ret < 0)
			break;

		for (size_t i = 0; i < _fds.size(); i++)
		{
			if (_fds[i].revents == 0)
				continue;

			if (_fds[i].fd == _socket_fd && (_fds[i].revents & POLLIN))
			{
				acceptNewClient();
			}
			else if (_fds[i].revents & POLLIN)
			{
				if (handleClientData(i) == false)
				{
					i--;
					continue;
				}
			}
			else if (_fds[i].revents & (POLLHUP | POLLERR))
			{
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

	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		delete it->second;
	_clients.clear();
	_fds.clear();

	std::cout << "IRC Server stopped" << std::endl;
}

// Utility functions
bool isPortNumber(const std::string &str)
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

bool Server::isNickInUse(const std::string &nick) const
{
	std::string lowerNick = nick;
	for (size_t i = 0; i < lowerNick.length(); ++i)
		lowerNick[i] = std::tolower(lowerNick[i]);

	for (std::map<int, Client *>::const_iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		std::string currentNick = it->second->getNickname();
		for (size_t i = 0; i < currentNick.length(); ++i)
			currentNick[i] = std::tolower(currentNick[i]);

		if (currentNick == lowerNick)
			return true;
	}
	return false;
}

Channel *Server::findChannel(const std::string &name)
{
    std::map<std::string, Channel>::iterator it = _channels.find(name);
    if (it == _channels.end())
        return NULL;
    return &it->second;
}

Channel &Server::createChannel(const std::string &name, const Client &creator)
{
    Channel ch(creator.getFd(), name);
    _channels[name] = ch;
    _channels[name].setAdmin(creator.getNickname(), creator.getFd());
    return _channels[name];
}