#include "header.hpp"
void Server::handleCommand(Client *client, const std::string &line)
{

    // ❌🚫👋🟢🧲 for designing the masseges
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.empty())
        return;

    const std::string &cmd = tokens[0];

    // Allow PASS anytime
    if (cmd == "PASS" || cmd == "pass")
    {
        if (!handlePass(client, tokens))
            return;
        client->setPassGiven(true);
        return;
    }

    // Block everything if PASS not given
    if (!client->isPassGiven())
    {
        std::string msg = "❌ ERROR :You must enter PASS before using any commands 🔐\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }

    // Allow USER after PASS
    if (cmd == "USER" || cmd == "user")
    {
        handleUser(client, tokens);
        client->setUserGiven(true);
        return;
    }

    // Block everything if USER not given yet
    if (!client->isUserGiven())
    {
        std::string msg = "❌ ERROR :You must enter USER after PASS 👤\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }

    // Allow NICK after USER
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
        return;
    }

    // Block everything if NICK not given yet
    if (!client->isNickGiven())
    {
        std::string msg = "❌ ERROR :You must enter NICK before using other commands 🆔\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
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
        handleQuit(client);
    else
        handlePrivateMessage(client, line, tokens);
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

    // 📏 Length check
    if (nickname.length() <= 4)
    {
        std::string msg = "❌ Nickname too short (must be > 4 characters) 🚫\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
        return;
    }

    // 🔠 Alphabetic-only check
    for (size_t i = 0; i < nickname.length(); ++i)
    {
        if (!std::isalpha(nickname[i]))
        {
            std::string msg = "❌ Nickname must contain only alphabetic characters 🔤\r\n";
            send(client->getFd(), msg.c_str(), msg.length(), 0);
            return;
        }
    }

    // ✅ Set nickname
    client->setNickname(nickname);
    std::cout << "✅ Set NICK for FD " << client->getFd() << ": " << nickname << " 🎯" << std::endl;

    // 🎉 Send welcome if username is already set
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

void Server::handleJoin(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    std::string channelName = tokens[1];
    if (_channels.find(channelName) == _channels.end())
    {
        _channels[channelName] = new Channel(channelName);
        std::cout << "✅ Created new channel: " << channelName << std::endl;
    }
    Channel *channel = _channels[channelName];
    if (channel->isInviteOnly() && !channel->isInvited(client))
    {
        std::string err = "🚫 473 " + channelName + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    if (channel->hasKey())
    {
        if (tokens.size() < 3 || tokens[2] != channel->getKey())
        {
            std::string err = "🚫 475 " + channelName + " :Cannot join channel (+k)\r\n";
            send(client->getFd(), err.c_str(), err.length(), 0);
            return;
        }
    }
    if (channel->hasUserLimit() && channel->getClients().size() >= static_cast<std::size_t>(channel->getUserLimit()))
    {
        std::string err = "🚫 471 " + channelName + " :Cannot join channel (+l)\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    channel->addClient(client);
    channel->removeInvite(client);
    if (channel->getClients().size() == 1)
        channel->addOperator(client);
    std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
    for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
        send((*it)->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
}

void Server::handlePrivMsg(Client *client, const std::string &line, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 3)
        return;
    std::string target = tokens[1];
    size_t colonPos = line.find(":", line.find("PRIVMSG"));
    std::string message = (colonPos != std::string::npos) ? line.substr(colonPos + 1) : "";
    std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
    if (target[0] == '#')
    {
        if (_channels.find(target) == _channels.end())
            return;
        Channel *channel = _channels[target];
        for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
            if (*it != client)
                send((*it)->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
    }
    else
        handlePrivateMessage(client, line, tokens);
}

void Server::handleKick(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 3)
        return;

    std::string chanName = tokens[1], targetNick = tokens[2];

    if (_channels.find(chanName) == _channels.end())
        return;

    Channel *channel = _channels[chanName];

    // ✅ Allow only operators or self-kick
    if (!channel->isOperator(client) && client->getNickname() != targetNick)
    {
        std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }

    // Find the target client
    Client *target = NULL;
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == targetNick)
        {
            target = it->second;
            break;
        }
    }

    if (!target || !channel->hasClient(target))
    {
        std::string err = "❌ 441 " + targetNick + " " + chanName + " :They aren't on that channel\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }

    // Remove target from channel
    channel->removeClient(target);
    channel->removeOperator(target);

    // Notify all clients
    std::string kickMsg = ":" + client->getNickname() + " KICK " + chanName + " " + targetNick + " :bye\r\n";
    for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
        send((*it)->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
    send(target->getFd(), kickMsg.c_str(), kickMsg.length(), 0);

    // ✅ Promote a new operator if needed
    if (!channel->hasOperators() && !channel->getClients().empty())
    {
        Client *newOp = *channel->getClients().begin();
        channel->addOperator(newOp);

        std::string opMsg = ":" + newOp->getNickname() + " MODE " + chanName + " +o " + newOp->getNickname() + "\r\n";
        for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
            send((*it)->getFd(), opMsg.c_str(), opMsg.length(), 0);
    }

    // 🧹 Clean up channel if it's now empty
    if (channel->getClients().empty())
    {
        delete channel;
        _channels.erase(chanName);
    }
}


void Server::handleTopic(Client *client, const std::string &line, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    std::string chanName = tokens[1];
    if (_channels.find(chanName) == _channels.end())
        return;
    Channel *channel = _channels[chanName];
    if (tokens.size() == 2)
    {
        const std::string &topic = channel->getTopic();
        std::string msg = topic.empty()
                              ? ":" + client->getNickname() + " 331 " + client->getNickname() + " " + chanName + " :No topic is set\r\n"
                              : ":" + client->getNickname() + " 332 " + client->getNickname() + " " + chanName + " :" + topic + "\r\n";
        send(client->getFd(), msg.c_str(), msg.length(), 0);
    }
    else
    {
        if (!channel->isOperator(client))
        {
            std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
            send(client->getFd(), err.c_str(), err.length(), 0);
            return;
        }
        size_t colonPos = line.find(':');
        if (colonPos == std::string::npos)
            return;
        std::string newTopic = line.substr(colonPos + 1);
        channel->setTopic(newTopic);
        std::string topicMsg = ":" + client->getNickname() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
        for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
            send((*it)->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
    }
}

void Server::handleMode(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 3)
        return;
    std::string chanName = tokens[1], mode = tokens[2];
    if (_channels.find(chanName) == _channels.end())
        return;
    Channel *channel = _channels[chanName];
    if (!channel->isOperator(client))
    {
        std::string err = "🚫 482 " + chanName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    if (mode == "+i")
        channel->setInviteOnly(true);
    else if (mode == "-i")
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
    std::string targetNick = tokens[1], chanName = tokens[2];
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
    std::string chanName = tokens[1];
    if (_channels.find(chanName) == _channels.end())
        return;
    Channel *channel = _channels[chanName];
    if (!channel->hasClient(client))
        return;
    channel->removeClient(client);
    channel->removeOperator(client);
    std::string msg = ":" + client->getNickname() + " PART " + chanName + "\r\n";
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
    }
}

void Server::handleNames(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;

    std::string chanName = tokens[1];

    if (_channels.find(chanName) == _channels.end())
        return;

    Channel *channel = _channels[chanName];

    std::string nameList;

    for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
    {
        if (channel->isOperator(*it))
            nameList += "@" + (*it)->getNickname() + " ";
        else
            nameList += (*it)->getNickname() + " ";
    }

    std::string msg = ":ircserv 353 " + client->getNickname() + " = " + chanName + " :" + nameList + "\r\n";
    msg += ":ircserv 366 " + client->getNickname() + " " + chanName + " :End of NAMES list\r\n";

    send(client->getFd(), msg.c_str(), msg.length(), 0);
}


void Server::handlePing(Client *client, const std::string &line)
{
    size_t colonPos = line.find(":");
    std::string token = (colonPos != std::string::npos) ? line.substr(colonPos + 1) : "";
    std::string pong = "PONG :" + token + "\r\n";
    send(client->getFd(), pong.c_str(), pong.length(), 0);
}

void Server::handleQuit(Client *client)
{
    std::string quitMsg = ":" + client->getNickname() + " QUIT :Leaving\r\n";

    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end();)
    {
        Channel *channel = it->second;

        if (channel->hasClient(client))
        {
            // Notify others in the channel
            for (std::set<Client *>::const_iterator c = channel->getClients().begin(); c != channel->getClients().end(); ++c)
            {
                if (*c != client)
                    send((*c)->getFd(), quitMsg.c_str(), quitMsg.length(), 0);
            }

            // Remove client and operator status
            channel->removeClient(client);
            channel->removeOperator(client);

            // 👑 Promote a new operator if needed
            if (!channel->hasOperators() && !channel->getClients().empty())
            {
                Client *newOp = *channel->getClients().begin();
                channel->addOperator(newOp);

                std::string opMsg = ":" + newOp->getNickname() + " MODE " + channel->getName() + " +o " + newOp->getNickname() + "\r\n";
                for (std::set<Client *>::const_iterator itc = channel->getClients().begin(); itc != channel->getClients().end(); ++itc)
                    send((*itc)->getFd(), opMsg.c_str(), opMsg.length(), 0);
            }

            // 🧹 If channel is empty, remove it
            if (channel->getClients().empty())
            {
                delete channel;
                std::map<std::string, Channel *>::iterator toErase = it++;
                _channels.erase(toErase);
                continue;
            }
        }

        ++it;
    }

    removeClientByFd(client->getFd());
}

void Server::handlePrivateMessage(Client *client, const std::string &line, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    std::string target = tokens[1];
    size_t colonPos = line.find(":", line.find("PRIVMSG"));
    std::string message = (colonPos != std::string::npos) ? line.substr(colonPos + 1) : "";
    std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

    Client *targetClient = NULL;
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
    {
        if (it->second->getNickname() == target)
        {
            targetClient = it->second;
            break;
        }
    }

    if (targetClient)
    {
        send(targetClient->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
    }
    else
    {
        std::string errMsg = "❌ 401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
        send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
    }
}
