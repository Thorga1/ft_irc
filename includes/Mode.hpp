#pragma once

#include "ACommand.hpp"

class Mode : public ACommand
{
public:
	Mode();
	Mode(Server *server);
	Mode(const Mode &other);
	~Mode();
	Mode &operator=(const Mode &other);
	void execute(Client &client, const std::vector<std::string> &args);

private:
	void handleOperatorMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding);
	void handleKeyMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding);
	void handleLimitMode(Client &client, Channel *ch, const std::vector<std::string> &args, int &count, bool adding);
	void handleFlagModes(Client &client, std::string modeStr, Channel *ch, char c, bool adding);
};
