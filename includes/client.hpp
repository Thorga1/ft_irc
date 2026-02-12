#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <poll.h>
#include <unistd.h>

class Client
{
public:
	enum e_status
	{
		PENDING,
		AUTHENTICATED,
		REGISTERED,
		DISCONNECTED
	};

private:
	int _fd;
	std::string _nickname;
	std::string _username;
	std::string _realname;
	std::string _buffer;
	e_status _status;

public:
	Client(int fd);
	Client(const Client &other);
	~Client();
	Client();

	int getFd() const;
	const std::string &getNickname() const;
	const std::string &getUsername() const;
	const std::string &getRealname() const;

	void setNickname(const std::string &nickname);
	void setUsername(const std::string &username);
	void setRealname(const std::string &realname);

	void appendBuffer(const std::string &data);
	std::string &getBuffer();
	void clearBuffer();

	void disconnect();

	e_status getStatus() const;
	void setStatus(e_status status);
	bool isRegistered() const;
	void setFd(int fd);
	Client &operator=(const Client &other);
};
