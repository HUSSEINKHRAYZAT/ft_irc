#include "header.hpp"

void Server::handleCommand(Client *client, const std::string &line)
{
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.empty())
        return;

    const std::string &cmd = tokens[0];
    if (cmd == "PASS")
        handlePass(client, tokens);
    else if (cmd == "NICK")
        handleNick(client, tokens);
    else if (cmd == "USER")
        handleUser(client, tokens);
    else if (cmd == "JOIN")
        handleJoin(client, tokens);
    else if (cmd == "PRIVMSG")
        handlePrivMsg(client, line, tokens);
    else if (cmd == "KICK")
        handleKick(client, tokens);
    else if (cmd == "TOPIC")
        handleTopic(client, line, tokens);
    else if (cmd == "MODE")
        handleMode(client, tokens);
    else if (cmd == "INVITE")
        handleInvite(client, tokens);
    else if (cmd == "PART")
        handlePart(client, tokens);
    else if (cmd == "NAMES")
        handleNames(client, tokens);
    else if (cmd == "PING")
        handlePing(client, line);
    else if (cmd == "QUIT")
        handleQuit(client);
    else
        handlePrivateMessage(client, line, tokens);
}

void Server::handlePass(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    if (tokens[1] == _password)
    {
        client->setRegistered(true);
        std::cout << "PASS accepted for FD " << client->getFd() << std::endl;
    }
    else
    {
        std::cout << "Invalid PASS from FD " << client->getFd() << std::endl;
        send(client->getFd(), "ERROR :Wrong password\r\n", 24, 0);
        removeClientByFd(client->getFd());
    }
}

void Server::handleNick(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    client->setNickname(tokens[1]);
    std::cout << "Set NICK for FD " << client->getFd() << ": " << tokens[1] << std::endl;
}

void Server::handleUser(Client *client, const std::vector<std::string> &tokens)
{
    if (tokens.size() < 2)
        return;
    client->setUsername(tokens[1]);
    std::cout << "Set USER for FD " << client->getFd() << ": " << tokens[1] << std::endl;

    if (client->isRegistered() && !client->getNickname().empty())
    {
        std::string welcome = ":ircserv 001 " + client->getNickname() + " :Welcome to IRC!\r\n";
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
        std::cout << "Created new channel: " << channelName << std::endl;
    }
    Channel *channel = _channels[channelName];
    if (channel->isInviteOnly() && !channel->isInvited(client))
    {
        std::string err = "473 " + channelName + " :Cannot join channel (+i)\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    if (channel->hasKey())
    {
        if (tokens.size() < 3 || tokens[2] != channel->getKey())
        {
            std::string err = "475 " + channelName + " :Cannot join channel (+k)\r\n";
            send(client->getFd(), err.c_str(), err.length(), 0);
            return;
        }
    }
    if (channel->hasUserLimit() && channel->getClients().size() >= static_cast<std::size_t>(channel->getUserLimit()))
    {
        std::string err = "471 " + channelName + " :Cannot join channel (+l)\r\n";
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
    if (!channel->isOperator(client))
    {
        std::string err = "482 " + chanName + " :You're not channel operator\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    Client *target = NULL;
    for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
        if (it->second->getNickname() == targetNick)
            target = it->second;
    if (!target || !channel->hasClient(target))
    {
        std::string err = "441 " + targetNick + " " + chanName + " :They aren't on that channel\r\n";
        send(client->getFd(), err.c_str(), err.length(), 0);
        return;
    }
    channel->removeClient(target);
    std::string kickMsg = ":" + client->getNickname() + " KICK " + chanName + " " + targetNick + " :bye\r\n";
    for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
        send((*it)->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
    send(target->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
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
            std::string err = "482 " + chanName + " :You're not channel operator\r\n";
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
        std::string err = "482 " + chanName + " :You're not channel operator\r\n";
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
        std::string err = "472 " + mode + " :Unknown mode\r\n";
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
        std::string err = "482 " + chanName + " :You're not channel operator\r\n";
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
    std::string msg = ":" + client->getNickname() + " PART " + chanName + "\r\n";
    for (std::set<Client *>::const_iterator it = channel->getClients().begin(); it != channel->getClients().end(); ++it)
        send((*it)->getFd(), msg.c_str(), msg.length(), 0);
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
        nameList += (*it)->getNickname() + " ";
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
    for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
    {
        Channel *channel = it->second;
        if (channel->hasClient(client))
        {
            for (std::set<Client *>::const_iterator c = channel->getClients().begin(); c != channel->getClients().end(); ++c)
                if (*c != client)
                    send((*c)->getFd(), quitMsg.c_str(), quitMsg.length(), 0);
            channel->removeClient(client);
        }
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
        std::string errMsg = "401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
        send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
    }
}
//----------------------------------------------------------------------
// void Server::handleCommand(Client *client, const std::string &line)
// {
//     std::vector<std::string> tokens = split(line, ' ');
//     if (tokens.empty())
//         return;

//     std::string cmd = tokens[0];
//     if (cmd == "PASS")
//     {
//         if (tokens.size() < 2)
//             return;
//         if (tokens[1] == _password)
//         {
//             client->setRegistered(true); // temp logic for demo
//             std::cout << "PASS accepted for FD " << client->getFd() << std::endl;
//         }
//         else
//         {
//             std::cout << "Invalid PASS from FD " << client->getFd() << std::endl;
//             send(client->getFd(), "ERROR :Wrong password\r\n", 24, 0);
//             removeClientByFd(client->getFd());
//         }
//     }
//     else if (cmd == "NICK")
//     {
//         if (tokens.size() < 2)
//             return;
//         client->setNickname(tokens[1]);
//         std::cout << "Set NICK for FD " << client->getFd() << ": " << tokens[1] << std::endl;
//     }
//     else if (cmd == "USER")
//     {
//         if (tokens.size() < 2)
//             return;
//         client->setUsername(tokens[1]);
//         std::cout << "Set USER for FD " << client->getFd() << ": " << tokens[1] << std::endl;

//         // Finish registration if everything is set
//         if (client->isRegistered() && !client->getNickname().empty())
//         {
//             std::string welcome = ":ircserv 001 " + client->getNickname() + " :Welcome to IRC!\r\n";
//             send(client->getFd(), welcome.c_str(), welcome.length(), 0);
//         }
//     }
//     else if (cmd == "JOIN")
//     {
//         if (tokens.size() < 2)
//             return;
//         std::string channelName = tokens[1];

//         // Create channel if it doesn't exist
//         if (_channels.find(channelName) == _channels.end())
//         {
//             _channels[channelName] = new Channel(channelName);
//             std::cout << "Created new channel: " << channelName << std::endl;
//         }

//         Channel *channel = _channels[channelName];
//         if (channel->isInviteOnly() && !channel->isInvited(client))
//         {
//             std::string err = "473 " + channelName + " :Cannot join channel (+i)\r\n";
//             send(client->getFd(), err.c_str(), err.length(), 0);
//             return;
//         }

//         // Join the channel
//         channel->addClient(client);
//         channel->removeInvite(client); // consume the invite

//         // Make first client the operator
//         if (channel->getClients().size() == 1)
//             channel->addOperator(client);
//         // Check key (password)
//         if (channel->hasKey())
//         {
//             if (tokens.size() < 3 || tokens[2] != channel->getKey())
//             {
//                 std::string err = "475 " + channelName + " :Cannot join channel (+k)\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }
//         }

//         // Check user limit
//         if (channel->hasUserLimit() && channel->getClients().size() >= static_cast<std::size_t>(channel->getUserLimit()))

//         {
//             std::string err = "471 " + channelName + " :Cannot join channel (+l)\r\n";
//             send(client->getFd(), err.c_str(), err.length(), 0);
//             return;
//         }

//         // Notify others in channel
//         std::string joinMsg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
//         const std::set<Client *> &members = channel->getClients();
//         for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//         {
//             send((*it)->getFd(), joinMsg.c_str(), joinMsg.length(), 0);
//         }
//     }

//     else if (cmd == "PRIVMSG")
//     {
//         if (tokens.size() < 3)
//             return;

//         std::string target = tokens[1];

//         // Rebuild message content after ":"
//         std::string message;
//         size_t colonPos = line.find(":", line.find("PRIVMSG"));
//         if (colonPos != std::string::npos)
//             message = line.substr(colonPos + 1);
//         else
//             return;

//         std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

//         if (target[0] == '#')
//         {
//             // Channel message
//             if (_channels.find(target) == _channels.end())
//                 return;
//             Channel *channel = _channels[target];

//             const std::set<Client *> &members = channel->getClients();
//             for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//             {
//                 if (*it != client)
//                 { // Don't echo to sender
//                     send((*it)->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
//                 }
//             }
//         }
//         else if (cmd == "KICK")
//         {
//             if (tokens.size() < 3)
//                 return;

//             std::string chanName = tokens[1];
//             std::string targetNick = tokens[2];

//             if (_channels.find(chanName) == _channels.end())
//                 return;
//             Channel *channel = _channels[chanName];

//             if (!channel->isOperator(client))
//             {
//                 std::string err = "482 " + chanName + " :You're not channel operator\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }

//             Client *target = NULL;
//             for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
//             {
//                 if (it->second->getNickname() == targetNick)
//                 {
//                     target = it->second;
//                     break;
//                 }
//             }

//             if (!target || !channel->hasClient(target))
//             {
//                 std::string err = "441 " + targetNick + " " + chanName + " :They aren't on that channel\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }

//             channel->removeClient(target);

//             std::string kickMsg = ":" + client->getNickname() + " KICK " + chanName + " " + targetNick + " :bye\r\n";

//             const std::set<Client *> &members = channel->getClients();
//             for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//             {
//                 send((*it)->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
//             }
//             send(target->getFd(), kickMsg.c_str(), kickMsg.length(), 0);
//         }
//         else if (cmd == "TOPIC")
//         {
//             if (tokens.size() < 2)
//                 return;
//             std::string chanName = tokens[1];

//             if (_channels.find(chanName) == _channels.end())
//                 return;

//             Channel *channel = _channels[chanName];

//             // View topic
//             if (tokens.size() == 2)
//             {
//                 const std::string &topic = channel->getTopic();
//                 std::string msg;

//                 if (!topic.empty())
//                     msg = ":" + client->getNickname() + " 332 " + client->getNickname() + " " + chanName + " :" + topic + "\r\n";
//                 else
//                     msg = ":" + client->getNickname() + " 331 " + client->getNickname() + " " + chanName + " :No topic is set\r\n";

//                 send(client->getFd(), msg.c_str(), msg.length(), 0);
//             }
//             // Set topic
//             else
//             {
//                 if (!channel->isOperator(client))
//                 {
//                     std::string err = "482 " + chanName + " :You're not channel operator\r\n";
//                     send(client->getFd(), err.c_str(), err.length(), 0);
//                     return;
//                 }

//                 size_t colonPos = line.find(':');
//                 if (colonPos == std::string::npos)
//                     return;

//                 std::string newTopic = line.substr(colonPos + 1);
//                 channel->setTopic(newTopic);

//                 std::string topicMsg = ":" + client->getNickname() + " TOPIC " + chanName + " :" + newTopic + "\r\n";
//                 const std::set<Client *> &members = channel->getClients();
//                 for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//                 {
//                     send((*it)->getFd(), topicMsg.c_str(), topicMsg.length(), 0);
//                 }
//             }
//         }
//         else if (cmd == "MODE")
//         {
//             if (tokens.size() < 3)
//                 return;

//             std::string chanName = tokens[1];
//             std::string mode = tokens[2];

//             if (_channels.find(chanName) == _channels.end())
//                 return;
//             Channel *channel = _channels[chanName];

//             if (!channel->isOperator(client))
//             {
//                 std::string err = "482 " + chanName + " :You're not channel operator\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }

//             if (mode == "+i")
//             {
//                 channel->setInviteOnly(true);
//             }
//             else if (mode == "-i")
//             {
//                 channel->setInviteOnly(false);
//             }
//             // +k key
//             if (mode == "+k")
//             {
//                 if (tokens.size() < 4)
//                     return;
//                 channel->setKey(tokens[3]);
//             }
//             else if (mode == "-k")
//             {
//                 channel->removeKey();
//             }

//             // +l limit
//             else if (mode == "+l")
//             {
//                 if (tokens.size() < 4)
//                     return;
//                 int limit = std::atoi(tokens[3].c_str());
//                 channel->setUserLimit(limit);
//             }
//             else if (mode == "-l")
//             {
//                 channel->removeUserLimit();
//             }

//             // +o nick (grant/remove operator)
//             else if (mode == "+o" || mode == "-o")
//             {
//                 if (tokens.size() < 4)
//                     return;
//                 std::string targetNick = tokens[3];
//                 Client *target = NULL;
//                 for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
//                 {
//                     if (it->second->getNickname() == targetNick)
//                     {
//                         target = it->second;
//                         break;
//                     }
//                 }

//                 if (!target || !channel->hasClient(target))
//                     return;

//                 if (mode == "+o")
//                     channel->addOperator(target);
//                 else
//                     channel->removeOperator(target);
//             }
//             else
//             {
//                 std::string err = "472 " + mode + " :Unknown mode\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }

//             std::string msg = ":" + client->getNickname() + " MODE " + chanName + " " + mode + "\r\n";
//             const std::set<Client *> &members = channel->getClients();
//             for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//             {
//                 send((*it)->getFd(), msg.c_str(), msg.length(), 0);
//             }
//         }
//         else if (cmd == "INVITE")
//         {
//             if (tokens.size() < 3)
//                 return;
//             std::string targetNick = tokens[1];
//             std::string chanName = tokens[2];

//             if (_channels.find(chanName) == _channels.end())
//                 return;
//             Channel *channel = _channels[chanName];

//             if (!channel->isOperator(client))
//             {
//                 std::string err = "482 " + chanName + " :You're not channel operator\r\n";
//                 send(client->getFd(), err.c_str(), err.length(), 0);
//                 return;
//             }

//             Client *target = NULL;
//             for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
//             {
//                 if (it->second->getNickname() == targetNick)
//                 {
//                     target = it->second;
//                     break;
//                 }
//             }

//             if (!target)
//                 return;

//             channel->invite(target);

//             std::string msg = ":" + client->getNickname() + " INVITE " + targetNick + " :" + chanName + "\r\n";
//             send(target->getFd(), msg.c_str(), msg.length(), 0);
//         }
//         else if (cmd == "PART")
//         {
//             if (tokens.size() < 2)
//                 return;
//             std::string chanName = tokens[1];

//             if (_channels.find(chanName) == _channels.end())
//                 return;
//             Channel *channel = _channels[chanName];

//             if (!channel->hasClient(client))
//                 return;

//             channel->removeClient(client);

//             std::string msg = ":" + client->getNickname() + " PART " + chanName + "\r\n";

//             const std::set<Client *> &members = channel->getClients();
//             for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//             {
//                 send((*it)->getFd(), msg.c_str(), msg.length(), 0);
//             }

//             // Remove empty channel
//             if (channel->getClients().empty())
//             {
//                 delete channel;
//                 _channels.erase(chanName);
//             }
//         }
//         else if (cmd == "NAMES")
//         {
//             if (tokens.size() < 2)
//                 return;
//             std::string chanName = tokens[1];

//             if (_channels.find(chanName) == _channels.end())
//                 return;
//             Channel *channel = _channels[chanName];

//             std::string nameList;
//             const std::set<Client *> &members = channel->getClients();
//             for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
//             {
//                 nameList += (*it)->getNickname() + " ";
//             }

//             std::string msg = ":ircserv 353 " + client->getNickname() + " = " + chanName + " :" + nameList + "\r\n";
//             msg += ":ircserv 366 " + client->getNickname() + " " + chanName + " :End of NAMES list\r\n";

//             send(client->getFd(), msg.c_str(), msg.length(), 0);
//         }
//         else if (cmd == "PING")
//         {
//             size_t colonPos = line.find(":");
//             std::string token = (colonPos != std::string::npos) ? line.substr(colonPos + 1) : "";

//             std::string pong = "PONG :" + token + "\r\n";
//             send(client->getFd(), pong.c_str(), pong.length(), 0);
//         }

//         else if (cmd == "QUIT")
//         {
//             std::string quitMsg = ":" + client->getNickname() + " QUIT :Leaving\r\n";

//             // Notify everyone in all channels this client is in
//             for (std::map<std::string, Channel *>::iterator it = _channels.begin(); it != _channels.end(); ++it)
//             {
//                 Channel *channel = it->second;
//                 if (channel->hasClient(client))
//                 {
//                     const std::set<Client *> &members = channel->getClients();
//                     for (std::set<Client *>::const_iterator c = members.begin(); c != members.end(); ++c)
//                     {
//                         if (*c != client)
//                             send((*c)->getFd(), quitMsg.c_str(), quitMsg.length(), 0);
//                     }
//                     channel->removeClient(client);
//                 }
//             }

//             removeClientByFd(client->getFd());
//         }

//         else
//         {
//             // Private message to user
//             Client *targetClient = NULL;
//             for (std::map<int, Client *>::iterator it = _clients.begin(); it != _clients.end(); ++it)
//             {
//                 if (it->second->getNickname() == target)
//                 {
//                     targetClient = it->second;
//                     break;
//                 }
//             }

//             if (targetClient)
//             {
//                 send(targetClient->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
//             }
//             else
//             {
//                 std::string errMsg = "401 " + client->getNickname() + " " + target + " :No such nick/channel\r\n";
//                 send(client->getFd(), errMsg.c_str(), errMsg.length(), 0);
//             }
//         }
//     }
// }