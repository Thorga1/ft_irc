#include "client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _realname(""), _buffer(""), _status(PENDING)
{
}

Client::Client() : _fd(-1), _nickname(""), _username(""), _realname(""), _buffer(""), _status(PENDING)
{
}

Client::Client(const Client &other)
	: _fd(other._fd), _nickname(other._nickname), _username(other._username), _realname(other._realname),
	  _buffer(other._buffer), _status(other._status)
{
}

Client::~Client()
{
	if (_fd >= 0)
		close(_fd);
}

Client &Client::operator=(const Client &other)
{
	if (this != &other)
	{
		_fd = other._fd;
		_nickname = other._nickname;
		_username = other._username;
		_realname = other._realname;
		_buffer = other._buffer;
		_status = other._status;
	}
	return *this;
}


int Client::getFd() const
{
	return _fd;
}

const std::string &Client::getNickname() const
{
	return _nickname;
}

const std::string &Client::getUsername() const
{
	return _username;
}

const std::string &Client::getRealname() const
{
	return _realname;
}

void Client::setNickname(const std::string &nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
	_username = username;
}

void Client::setRealname(const std::string &realname)
{
	_realname = realname;
}

void Client::appendBuffer(const std::string &data)
{
	_buffer += data;
}

void Client::clearBuffer()
{
	_buffer.clear();
}

void Client::disconnect()
{
	if (_fd >= 0)
	{
		close(_fd);
		_fd = -1;
	}
}

Client::e_status Client::getStatus() const
{
	return _status;
}

void Client::setStatus(e_status status)
{
	_status = status;
}

void Client::setFd(int fd)
{
	_fd = fd;
}

bool Client::isRegistered() const
{
	return _status == REGISTERED;
}

std::string &Client::getBuffer()
{
	return _buffer;
}
