#include "client.hpp"

Client::Client(int fd) : _fd(fd), _nickname(""), _username(""), _buffer(""), _authenticated(false)
{
}

Client::~Client()
{
	if (_fd >= 0)
		close(_fd);
}

int Client::getFd() const
{
	return _fd;
}

const std::string& Client::getNickname() const
{
	return _nickname;
}

const std::string& Client::getUsername() const
{
	return _username;
}

bool Client::isAuthenticated() const
{
	return _authenticated;
}

void Client::setNickname(const std::string& nickname)
{
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	_username = username;
}

void Client::setAuthenticated(bool auth)
{
	_authenticated = auth;
}

void Client::appendBuffer(const std::string& data)
{
	_buffer += data;
}

std::string Client::getBuffer() const
{
	return _buffer;
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
