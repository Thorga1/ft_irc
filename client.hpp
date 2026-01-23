#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <poll.h>
#include <unistd.h>

class Client
{
private:
	int				_fd;
	std::string		_nickname;
	std::string		_username;
	std::string		_buffer;
	bool			_authenticated;

public:
	Client(int fd);
	~Client();

	int					getFd() const;
	const std::string&	getNickname() const;
	const std::string&	getUsername() const;
	bool				isAuthenticated() const;

	void				setNickname(const std::string& nickname);
	void				setUsername(const std::string& username);
	void				setAuthenticated(bool auth);

	void				appendBuffer(const std::string& data);
	std::string			getBuffer() const;
	void				clearBuffer();

	void				disconnect();
};
