#include "header.hpp"
bool Server::handleCommand(Client *client, const std::string &line)
{
    std::vector<std::string> tokens = split(line, ' ');
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

    const std::string chanName = tokens[1];

    // 1) Does the channel exist?
    std::map<std::string, Channel*>::iterator cit = _channels.find(chanName);
    if (cit == _channels.end())
    {
        std::string err = ":ircserv 403 " 
            + client->getNickname() + " "
            + chanName 
            + " :No such channel\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    Channel *channel = cit->second;

    // 2) Is the client on that channel?
    if (!channel->hasClient(client))
    {
        std::string err = ":ircserv 442 "
            + client->getNickname() + " "
            + chanName
            + " :You're not on that channel\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }

    // 3) Build the name list
    std::string nameList;
    for (std::set<Client*>::const_iterator it = channel->getClients().begin();
         it != channel->getClients().end(); ++it)
    {
        if (channel->isOperator(*it))
            nameList += "@" + (*it)->getNickname() + " ";
        else
            nameList += (*it)->getNickname() + " ";
    }

    // 4) Send the 353 (NAMREPLY) and 366 (END OF NAMES) replies
    std::string reply  = ":ircserv 353 "
                       + client->getNickname()
                       + " = " + chanName
                       + " :" + nameList
                       + "\r\n";
    std::string reply2 = ":ircserv 366 "
                       + client->getNickname()
                       + " " + chanName
                       + " :End of NAMES list\r\n";
    send(client->getFd(), reply.c_str(),  reply.length(),  0);
    send(client->getFd(), reply2.c_str(), reply2.length(), 0);
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
        // everything after the space-colon
        message = line.substr(p + 2);
    }
    else
    {
        // fallback: join tokens[2..]
        message = tokens[2];
        for (size_t i = 3; i < tokens.size(); ++i)
            message += " " + tokens[i];
    }

    // 2) Build the outgoing line once
    std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

    // --- CHANNEL CASE ---
    if (!target.empty() && target[0] == '#')
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

        // c) broadcast to _all_ members (including sender)
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
    // --- USER-TO-USER CASE ---
    else
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
}
