#include "header.hpp"
bool Server::handleCommand(Client *client, const std::string &line)
{
	std::vector<std::string> tokens = splitTrimmed(line);
	if (tokens.empty())
		return true;

	const std::string &cmd = tokens[0];

	// Allow PASS anytime
	if (cmd == "PASS" || cmd == "pass")
	{
		if (!handlePass(client, tokens))
			return false; // ❗ Stop, client was removed
		client->setPassGiven(true);
		return true;
	}

	if (!client->isPassGiven())
	{
		std::string msg = "❌ ERROR :You must enter PASS before using any commands 🔐\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return true;
	}

	if (cmd == "USER" || cmd == "user")
	{
		handleUser(client, tokens);
		client->setUserGiven(true);
		return true;
	}

	if (!client->isUserGiven())
	{
		std::string msg = "❌ ERROR :You must enter USER after PASS 👤\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return true;
	}

	if (cmd == "NICK" || cmd == "nick")
	{
		handleNick(client, tokens);
		client->setNickGiven(true);

		if (!client->isRegistered() && client->isPassGiven() && client->isUserGiven() && client->isNickGiven())
		{
			client->setRegistered(true);
			std::string welcome = ":ircserv 001 " + client->getNickname() + " :Welcome to IRC! 🎊\r\n";
			send(client->getFd(), welcome.c_str(), welcome.length(), 0);
		}
		return true;
	}

	if (!client->isNickGiven())
	{
		std::string msg = "❌ ERROR :You must enter NICK before using other commands 🆔\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return true;
	}

	// ✅ Fully authenticated — allow normal commands
	if (cmd == "JOIN" || cmd == "join")
		handleJoin(client, tokens);
	else if (cmd == "PRIVMSG" || cmd == "privmsg")
		handlePrivMsg(client, line, tokens);
	else if (cmd == "KICK" || cmd == "kick")
		handleKick(client, tokens);
	else if (cmd == "TOPIC" || cmd == "topic")
		handleTopic(client, line, tokens);
	else if (cmd == "MODE" || cmd == "mode")
		handleMode(client, tokens);
	else if (cmd == "INVITE" || cmd == "invite")
		handleInvite(client, tokens);
	else if (cmd == "PART" || cmd == "part")
		handlePart(client, tokens);
	else if (cmd == "NAMES" || cmd == "names")
		handleNames(client, tokens);
	else if (cmd == "PING" || cmd == "ping")
		handlePing(client, line);
	else if (cmd == "QUIT" || cmd == "quit")
		return handleQuit(client);
	// else
	//     handlePrivateMessage(client, line, tokens);

	return true;
}

bool Server::handlePass(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
	{
		std::string msg = "❌ PASS command requires a password 🛑\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return false;
	}

	if (tokens[1] == _password)
	{
		client->setRegistered(true);
		std::cout << "✅ PASS accepted for FD " << client->getFd() << std::endl;
		return true;
	}
	else
	{
		std::cout << "❌ Invalid PASS from FD " << client->getFd() << std::endl;
		send(client->getFd(), "❌ ERROR :Wrong password\r\n", 28, 0);
		removeClientByFd(client->getFd());
		return false;
	}
}

void Server::handleNick(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
	{
		std::string msg = "❌ NICK command requires a nickname 🙅‍♀️\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return;
	}

	const std::string &nickname = tokens[1];

	// Length check
	if (nickname.length() <= 4)
	{
		std::string msg = "❌ Nickname too short (must be > 4 characters) 🚫\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return;
	}

	// Alphabetic-only check
	for (size_t i = 0; i < nickname.length(); ++i)
	{
		if (!std::isalpha(static_cast<unsigned char>(nickname[i])))
		{
			std::string msg = "❌ Nickname must contain only alphabetic characters 🔤\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
			return;
		}
	}

	// Prevent duplicates
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		Client *other = it->second;
		if (other != client && other->getNickname() == nickname)
		{
			std::string msg = "🚫 ERROR :Nickname is already in use 😤\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
			return;
		}
	}

	// Set nickname
	client->setNickname(nickname);
	std::cout << "✅ Set NICK for FD " << client->getFd() << ": " << nickname << " 🎯" << std::endl;

	if (client->isRegistered() && !client->getUsername().empty())
	{
		std::string welcome = ":ircserv 001 " + nickname + " :Welcome to IRC! 🎊\r\n";
		send(client->getFd(), welcome.c_str(), welcome.length(), 0);
	}
}


void Server::handleUser(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
	{
		std::string msg = "❌ USER command requires a username 🙅‍♂️\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return;
	}

	const std::string &username = tokens[1];

	// 🔍 Check length
	if (username.length() <= 3)
	{
		std::string msg = "❌ Username too short (must be > 4 characters) 🧱\r\n";
		send(client->getFd(), msg.c_str(), msg.length(), 0);
		return;
	}

	// 🔠 Check alphabetic only
	for (size_t i = 0; i < username.length(); ++i)
	{
		if (!std::isalpha(username[i]))
		{
			std::string msg = "❌ Username must contain only alphabetic characters 🔤\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
			return;
		}
	}

	// ✅ Valid username
	client->setUsername(username);
	std::cout << "✅ Set USER for FD " << client->getFd() << ": " << username << " 🧑‍💻" << std::endl;

	// 🎉 Send welcome if nickname already set
	if (client->isRegistered() && !client->getNickname().empty())
	{
		std::string welcome = ":ircserv 001 " + client->getNickname() + " :Welcome to IRC! 🎊\r\n";
		send(client->getFd(), welcome.c_str(), welcome.length(), 0);
	}
}

void	Server::handleJoin(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2 || tokens.size() > 3)
	{
		std::string err = "🚫 Join <Channel> <Pass[optional]>\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (!isValidChannelName(tokens[1]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string channelName = tokens[1].substr(1);
	if (channelName.size() < 2)
	{
		std::string err = "🚫 479 " + channelName + " :Illegal channel name\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (_channels.find(channelName) == _channels.end())
	{
		_channels[channelName] = new Channel(channelName);
		std::cout << "✅ Created new channel: " << channelName << std::endl;
		if (tokens.size() > 2)
			_channels[channelName]->setKey(tokens[2]);
	}
	Channel *channel = _channels[channelName];
	if (channel->hasClient(client))
	{
		std::string err = "🌀 Already joined! 🌀\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (channel->isInviteOnly() && !channel->isInvited(client))
	{
		std::string err = "🚫 473 " + channelName + " :Cannot join channel (+i)\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (channel->hasKey()
		&& !channel->isInvited(client)
		&& (tokens.size() < 3 || tokens[2] != channel->getKey()))
	{

		std::string err = "🚫 475 " + channelName + " :Cannot join channel (+k)\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	if (channel->hasUserLimit()
		&& channel->getClients().size() >= static_cast<std::size_t>(channel->getUserLimit()))
	{
		std::string err = "🚫 471 " + channelName + " :Cannot join channel (+l)\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
		channel->addClient(client);
		channel->removeInvite(client);
		if (channel->getClients().size() == 1)
			channel->addOperator(client);
		std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
		for (std::set<Client *>::const_iterator it = channel->getClients().begin();
			it != channel->getClients().end(); ++it)
			send((*it)->getFd(), joinMsg.c_str(), joinMsg.length(), 0);

}


void Server::handleKick(Client *client, const std::vector<std::string> &tokens) {
	if (tokens.size() < 3)
	{
		std::string err2 = "🚫 kick <channel> <user1> [user2] [user3]...\n";
		send(client->getFd(), err2.c_str(), err2.length(), 0);
		return;
	}
	if (!isValidChannelName(tokens[1]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string chanName = tokens[1].substr(1);
	if (_channels.find(chanName) == _channels.end())
		return;
	Channel *channel = _channels[chanName];
	// :white_check_mark: Allow only operators to kick others
	if (!channel->isOperator(client))
	{
		std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	// Process each user to kick (starting from tokens[2])
	for (size_t i = 2; i < tokens.size(); ++i) {
		std::string targetNick = tokens[i];
		// Find the target client
		Client *target = NULL;
		for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
			if (it->second->getNickname() == targetNick) {
				target = it->second;
				break;
			}
		}
		// Check if target exists and is in the channel
		if (!target || !channel->hasClient(target)) {
			std::string err = ":x: 441 " + targetNick + " " + chanName + " :They aren't on that channel\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			continue; // Skip this user but continue with others
		}
		// Don't allow kicking yourself (optional - remove if you want to allow self-kick)
		if (target == client) {
			std::string err = ":no_entry_sign: You cannot kick yourself\r\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			continue;
		}
		// Remove target from channel
		channel->removeClient(target);
		channel->removeOperator(target);
		// Notify all clients about the kick
		std::string kickMsg = ":" + client->getNickname() + " KICK " + chanName + " " + targetNick + " :bye\r\n";
		for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
			send((*it)->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
		}
		send(target->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
	}
	// :white_check_mark: Promote a new operator if needed (only once after all kicks)
	if (!channel->hasOperators() && !channel->getClients().empty()) {
		Client *newOp = *channel->getClients().begin();
		channel->addOperator(newOp);
		std::string opMsg = ":" + newOp->getNickname() + " MODE " + chanName + " +o " + newOp->getNickname() + "\r\n";
		for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it) {
			send((*it)->getFd(), opMsg.c_str(), opMsg.length(), 0);
		}
	}
	// :broom: Clean up channel if it's now empty
	if (channel->getClients().empty()) {
		delete channel;
		_channels.erase(chanName);
	}
}
void	Server::handleTopic(Client *client, const std::string &line, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
		return;

	if (!isValidChannelName(tokens[1]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string chanName = tokens[1].substr(1);

	std::map<std::string, Channel*>::iterator it = _channels.find(chanName);
	if (it == _channels.end())
		return ;

	Channel *channel = it->second;

	// Must be in the channel
	if (!channel->hasClient(client))
	{
		std::string err = ":ircserv 442 " + client->getNickname()
			+ " " + chanName + " :You're not on that channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	// Just viewing the topic
	if (tokens.size() == 2)
	{
		const std::string &topic = channel->getTopic();
		if (topic.empty())
		{
			std::string msg = ":ircserv 331 " + client->getNickname()
				+ " " + chanName + " :No topic is set\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
		}
		else
		{
			std::string msg = ":ircserv 332 " + client->getNickname()
				+ " " + chanName + " :" + topic + "\r\n";
			send(client->getFd(), msg.c_str(), msg.length(), 0);
		}
		return;
	}

	// Setting a topic
	size_t colonPos = line.find(" :");
	if (colonPos == std::string::npos)
	{
		std::string err = "🚫 Error: <TOPIC> <CHANNEL> ';' <MESSAGE>\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	std::string newTopic = line.substr(colonPos + 2);

	// Check +t (topic restriction mode)
	if (channel->isTopicRestricted() && !channel->isOperator(client))
	{
		std::string err = ":ircserv 482 " + client->getNickname()
			+ " " + chanName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	// Set topic (can be empty to clear it)
	channel->setTopic(newTopic);

	// Broadcast to all clients
	std::string topicMsg = ":" + client->getNickname()
		+ " TOPIC " + chanName + " :" + newTopic + "\r\n";

	for (std::set<Client *>::const_iterator it = channel->getClients().begin();
		 it != channel->getClients().end(); ++it)
	{
		send((*it)->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
	}
}


void Server::handleMode(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 3)
		return;
	if (!isValidChannelName(tokens[1]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string chanName = tokens[1].substr(1), mode = tokens[2];
	if (_channels.find(chanName) == _channels.end())
		return;
	Channel *channel = _channels[chanName];
	if (!channel->isOperator(client))
	{
		std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	if (mode == "+i" && tokens.size() >= 3)
		channel->setInviteOnly(true);
	else if (mode == "-i" && tokens.size() >= 3)
		channel->setInviteOnly(false);
	else if (mode == "+k" && tokens.size() >= 4)
		channel->setKey(tokens[3]);
	else if (mode == "-k")
		channel->removeKey();
	else if (mode == "+l" && tokens.size() >= 4)
		channel->setUserLimit(std::atoi(tokens[3].c_str()));
	else if (mode == "-l")
		channel->removeUserLimit();
	else if ((mode == "+o" || mode == "-o") && tokens.size() >= 4)
	{
		std::string targetNick = tokens[3];
		Client *target = NULL;
		for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
			if (it->second->getNickname() == targetNick)
				target = it->second;
		if (target && channel->hasClient(target))
		{
			if (mode == "+o")
				channel->addOperator(target);
			else
				channel->removeOperator(target);
		}
	}
	else if (mode == "+t")
		channel->setTopicPermission(true);
	else if (mode == "-t")
		channel->setTopicPermission(false);
	else
	{
		std::string err = "❌ 472 " + mode + " :Unknown mode\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	std::string msg = ":" + client->getNickname() + " MODE " + chanName + " " + mode + "\r\n";
	for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
		send((*it)->getFd(), msg.c_str(), msg.length(), 0);
}

void Server::handleInvite(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 3)
		return;
	if (!isValidChannelName(tokens[2]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string targetNick = tokens[1], chanName = tokens[2].substr(1);
	if (_channels.find(chanName) == _channels.end())
		return;
	Channel *channel = _channels[chanName];
	if (!channel->isOperator(client))
	{
		std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}
	Client *target = NULL;
	for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
		if (it->second->getNickname() == targetNick)
			target = it->second;
	if (!target)
		return;
	channel->invite(target);
	std::string msg = ":" + client->getNickname() + " INVITE " + targetNick + " :" + chanName + "\r\n";
	send(target->getFd(), msg.c_str(), msg.length(), 0);
}

void Server::handlePart(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
		return;
	if (!isValidChannelName(tokens[1]))
	{
		std::string err = "🚫 Channel name must start with '#'\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return ;
	}
	std::string chanName = tokens[1].substr(1);
	if (_channels.find(chanName) == _channels.end())
		return;
	Channel *channel = _channels[chanName];
	if (!channel->hasClient(client))
		return;
	channel->removeClient(client);
	channel->removeOperator(client);
	std::string msg = ":" + client->getNickname() + " PART " + chanName + "\r\n";
	std::string err = "🌀 Note: You left the Channel\n";
	send(client->getFd(), err.c_str(), err.length(), 0);
	for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
		send((*it)->getFd(), msg.c_str(), msg.length(), 0);

	if (!channel->hasOperators() && !channel->getClients().empty())
	{
		Client *newOp = *channel->getClients().begin();
		channel->addOperator(newOp);
		std::string opMsg = ":" + newOp->getNickname() + " MODE " + chanName + " +o " + newOp->getNickname() + "\r\n";
		for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
			send((*it)->getFd(), opMsg.c_str(), opMsg.length(), 0);
	}

	if (channel->getClients().empty())
	{
		delete channel;
		_channels.erase(chanName);
		std::string err = "🌀 Channel Deleted\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
	}
}

std::vector<std::string>	splitChannels(const std::string &input)
{
	std::vector<std::string>	result;
	std::string					token;
	for (size_t i = 0; i < input.length(); ++i)
	{
		if (input[i] == ',')
		{
			if (!token.empty())
				result.push_back(token);
			token.clear();
		}
		else
			token += input[i];
	}
	if (!token.empty())
		result.push_back(token);
	return result;
}


void	Server::handleNames(Client *client, const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
		return;

	std::vector<std::string> channelList = splitChannels(tokens[1]);

	for (size_t i = 0; i < channelList.size(); ++i)
	{
		if (!isValidChannelName(channelList[i]))
		{
			std::string err = "🚫 Channel name must start with '#'\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			continue;
		}
		const std::string &chanName = channelList[i].substr(1);
		std::map<std::string, Channel *>::iterator cit = _channels.find(chanName);
		if (cit == _channels.end())
			continue;

		Channel *channel = cit->second;

		if (!channel->hasClient(client))
		{
			std::string err = "🚫 Error: You are not in the channel! \n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			continue;
		}

		std::string nameList;
		const std::set<Client *> &clients = channel->getClients();

		for (std::set<Client *>::const_iterator it = clients.begin();
			 it != clients.end(); ++it)
		{
			if (channel->isOperator(*it))
				nameList += "@" + (*it)->getNickname() + " ";
			else
				nameList += (*it)->getNickname() + " ";
		}

		std::string reply = ":ircserv 353 " + client->getNickname()
			+ " = " + chanName + " :" + nameList + "\r\n";
		std::string endReply = ":ircserv 366 " + client->getNickname()
			+ " " + chanName + " :End of NAMES list\r\n";

		send(client->getFd(), reply.c_str(), reply.length(), 0);
		send(client->getFd(), endReply.c_str(), endReply.length(), 0);
	}
}



void Server::handlePing(Client *client, const std::string &line)
{
	size_t colonPos = line.find(":");
	std::string token = (colonPos != std::string::npos) ? line.substr(colonPos + 1) : "";
	std::string pong = "PONG :" + token + "\r\n";
	send(client->getFd(), pong.c_str(), pong.length(), 0);
}

bool Server::handleQuit(Client *client)
{
	// --- 1) Cache everything you’ll need AFTER deletion ---
	int fd            = client->getFd();
	std::string nick  = client->getNickname();
	std::string quitMsg = ":" + nick + " QUIT :Leaving\r\n";

	// --- 2) Notify all channels (client is still live here) ---
	for (std::map<std::string, Channel *>::iterator it = _channels.begin();
		 it != _channels.end(); )
	{
		Channel *chan = it->second;
		if (chan->hasClient(client))
		{
			// notify other members
			for (std::set<Client *>::const_iterator cit = chan->getClients().begin();
				 cit != chan->getClients().end(); ++cit)
			{
				if (*cit != client)
					send((*cit)->getFd(),
						 quitMsg.c_str(),
						 quitMsg.length(),
						 0);
			}

			// remove from channel & operator list
			chan->removeClient(client);
			chan->removeOperator(client);

			// promote a new operator if needed
			if (!chan->hasOperators() && !chan->getClients().empty())
			{
				Client *newOp = *chan->getClients().begin();
				chan->addOperator(newOp);

				std::string opMsg = ":" + newOp->getNickname()
					+ " MODE " + chan->getName()
					+ " +o " + newOp->getNickname() + "\r\n";
				for (std::set<Client *>::const_iterator itc = chan->getClients().begin();
					 itc != chan->getClients().end(); ++itc)
				{
					send((*itc)->getFd(),
						 opMsg.c_str(),
						 opMsg.length(),
						 0);
				}
			}

			// erase the channel if empty
			if (chan->getClients().empty())
			{
				delete chan;
				std::map<std::string,Channel*>::iterator tmp = it++;
				_channels.erase(tmp);
				continue;
			}
		}
		++it;
	}

	// --- 3) Now delete the client once (using cached fd) ---
	removeClientByFd(fd);

	// --- 4) We can still log using the local fd variable ---
	std::cout << "✅ Disconnected client FD: " << fd << std::endl;

	// --- 5) Signal “client removed” so callers bail out before touching it ---
	return false;
}

void Server::handlePrivMsg(Client *client,
						   const std::string &line,
						   const std::vector<std::string> &tokens)
{
	if (tokens.size() < 3)
		return;

	std::string target = tokens[1];

	// 1) Extract the message text
	std::string message;
	size_t p = line.find(" :");
	if (p != std::string::npos)
	{
			message = line.substr(p + 2);

	}
	else
	{
		if (tokens.size() == 3)
		{
			message = tokens[2];
			for (size_t i = 3; i < tokens.size(); ++i)
				message += " " + tokens[i];
		}
		else
		{
			std::string err = "🚫 Error: <PRIVMSG> <TARGET> ';' <MESSAGE>\n";
			send(client->getFd(), err.c_str(), err.length(), 0);
			return;
		}
	}

	// 2) Build the outgoing line once
	std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

	// --- CHANNEL CASE ---
	if (!target.empty() && target[0] == '#')
	{
		handleChannelPrivMsg(client, target.substr(1), fullMsg);
	}
	// --- USER-TO-USER CASE ---
	else
	{
		handleUserPrivMsg(client, target, fullMsg);
	}
}

void Server::handleChannelPrivMsg(Client *client, const std::string &target, const std::string &fullMsg)
{
	// a) channel exists?
	std::map<std::string, Channel *>::iterator cit = _channels.find(target);
	if (cit == _channels.end())
	{
		std::string err = ":ircserv 403 " + client->getNickname() + " " + target + " :No such channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	Channel *channel = cit->second;

	// b) sender in channel?
	if (!channel->hasClient(client))
	{
		std::string err = ":ircserv 404 " + client->getNickname() + " " + target + " :Cannot send to channel\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	// c) broadcast to *all* members (including sender)
	const std::set<Client *> &members = channel->getClients();
	for (std::set<Client *>::const_iterator it = members.begin();
		 it != members.end(); ++it)
	{
		send((*it)->getFd(),
			 fullMsg.c_str(),
			 fullMsg.length(),
			 0);
	}
}

void Server::handleUserPrivMsg(Client *client, const std::string &target, const std::string &fullMsg)
{
	// find the target client by nick
	Client *dest = NULL;
	for (std::map<int, Client *>::iterator it = _clients.begin();
		 it != _clients.end(); ++it)
	{
		if (it->second->getNickname() == target)
		{
			dest = it->second;
			break;
		}
	}

	// no such nick?
	if (!dest)
	{
		std::string err = ":ircserv 401 " + client->getNickname() + " " + target + " :No such nick\r\n";
		send(client->getFd(), err.c_str(), err.length(), 0);
		return;
	}

	// send the PM (and echo back to sender if you like)
	send(dest->getFd(),
		 fullMsg.c_str(),
		 fullMsg.length(),
		 0);

	// echo to sender for netcat testing:
	send(client->getFd(),
		 fullMsg.c_str(),
		 fullMsg.length(),
		 0);
}




// wrong pass, correct pass, quit.

// c2r3s1% nc 127.0.0.1 6697
// 🟢 🟢 🟢 YOU ARE CONNECTED NOW TO THE SERVER YOU MUST ENTER YOUR CARDINALITES TO ACCESS IT
// FIRST ENTER THE PASSWORD THEN THE USER THEN THE NICK
// pass ss
// nick afayad
// ❌ ERROR :You must enter USER after PASS 👤
// pass afayad
// ❌ ERROR :Wrong password
// dsf

