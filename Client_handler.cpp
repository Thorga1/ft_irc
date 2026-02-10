#include "Client_handler.hpp"
#include "Join.hpp"
#include "Nick.hpp"
#include "Mode.hpp"
#include "Pass.hpp"
#include "Quit.hpp"
#include "User.hpp"
#include "Invite.hpp"
#include "Kick.hpp"
#include "Topic.hpp"
#include "Privmsg.hpp"
#include <algorithm>
#include "Ping.hpp"
#include "WhoIs.hpp"

ClientHandler::ClientHandler(Server *server)
{
	_commands["PASS"] = new Pass(server);
	_commands["NICK"] = new Nick(server);
	_commands["JOIN"] = new Join(server);
	_commands["USER"] = new User(server);
	_commands["QUIT"] = new Quit(server);
	_commands["KICK"] = new Kick(server);
	_commands["PRIVMSG"] = new Privmsg(server);
	_commands["MSG"] = new Privmsg(server);
	_commands["INVITE"] = new Invite(server);
	_commands["TOPIC"] = new Topic(server);
	_commands["MODE"] = new Mode(server);
	_commands["PING"] = new ping(server);
	_commands["WHOIS"] = new WhoIs(server);
}

ClientHandler::~ClientHandler()
{
	for (std::map<std::string, ACommand *>::iterator it = _commands.begin(); it != _commands.end(); ++it)
		delete it->second;
}

void ClientHandler::processCommand(Client &client, const std::string &rawLine)
{
	std::vector<std::string> args = parseCommand(rawLine);
	if (args.empty())
		return;

	std::string cmdName = args[0];
	std::transform(cmdName.begin(), cmdName.end(), cmdName.begin(), ::toupper);

	if (client.getStatus() == Client::PENDING && cmdName != "PASS" && cmdName != "QUIT" && cmdName != "MODE" && cmdName != "CAP")
	{
		_commands["PASS"]->sendReply(client.getFd(), ":server 451 * :Register with PASS first");
		return;
	}

	if (_commands.count(cmdName))
	{
		_commands[cmdName]->execute(client, args);
	}
	else if (cmdName != "CAP")
	{
		_commands["JOIN"]->sendReply(client.getFd(), ":server 421 " + client.getNickname() + " " + cmdName + " :Unknown command");
	}
}

std::vector<std::string> ClientHandler::parseCommand(const std::string &command)
{
	std::vector<std::string> args;
	size_t pos = command.find(" :");

	std::string beforeColon = (pos == std::string::npos) ? command : command.substr(0, pos);
	std::istringstream iss(beforeColon);
	std::string token;
	while (iss >> token)
		args.push_back(token);

	if (pos != std::string::npos)
		args.push_back(command.substr(pos + 2));

	return args;
}