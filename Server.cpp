#include "header.hpp"

Server::Server(int port, const std::string& password)
    : _serverSocket(-1), _port(port), _password(password) {
    setupSocket();
}


Server::~Server()
{
    for (size_t i = 0; i < _pollFds.size(); ++i)
    {
        close(_pollFds[i].fd);
    }
}

void Server::setupSocket()
{
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
    {
        throw std::runtime_error("Socket creation failed");
    }

    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
    {
        throw std::runtime_error("setsockopt failed");
    }

    fcntl(_serverSocket, F_SETFL, O_NONBLOCK); // non-blocking

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_port);

    if (bind(_serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        throw std::runtime_error("Bind failed");
    }

    if (listen(_serverSocket, 10) < 0)
    {
        throw std::runtime_error("Listen failed");
    }

    struct pollfd serverPollFd;
    serverPollFd.fd = _serverSocket;
    serverPollFd.events = POLLIN;
    _pollFds.push_back(serverPollFd);

    std::cout << "Server listening on port " << _port << std::endl;
}

void Server::run()
{
    while (true)
    {
        int ret = poll(&_pollFds[0], _pollFds.size(), -1);
        if (ret < 0)
        {
            std::cerr << "Poll failed" << std::endl;
            break;
        }

        for (size_t i = 0; i < _pollFds.size(); ++i)
        {
            if (_pollFds[i].revents & POLLIN)
            {
                if (_pollFds[i].fd == _serverSocket)
                {
                    handleNewConnection();
                }
                else
                {
                    handleClientData(i);
                }
            }
        }
    }
}

void Server::handleNewConnection()
{
    struct sockaddr_in clientAddr;
    socklen_t addrLen = sizeof(clientAddr);
    int clientFd = accept(_serverSocket, (struct sockaddr *)&clientAddr, &addrLen);
    if (clientFd < 0)
    {
        std::cerr << "Failed to accept connection" << std::endl;
        return;
    }

    fcntl(clientFd, F_SETFL, O_NONBLOCK);

    struct pollfd newPollFd;
    newPollFd.fd = clientFd;
    newPollFd.events = POLLIN;
    _pollFds.push_back(newPollFd);

    _clients[clientFd] = new Client(clientFd);

    std::cout << "New client connected: FD " << clientFd << std::endl;
}

void Server::handleCommand(Client *client, const std::string &line)
{
    std::vector<std::string> tokens = split(line, ' ');
    if (tokens.empty())
        return;

    std::string cmd = tokens[0];
    if (cmd == "PASS")
    {
        if (tokens.size() < 2)
            return;
        if (tokens[1] == _password)
        {
            client->setRegistered(true); // temp logic for demo
            std::cout << "PASS accepted for FD " << client->getFd() << std::endl;
        }
        else
        {
            std::cout << "Invalid PASS from FD " << client->getFd() << std::endl;
            send(client->getFd(), "ERROR :Wrong password\r\n", 24, 0);
            removeClientByFd(client->getFd());
        }
    }
    else if (cmd == "NICK")
    {
        if (tokens.size() < 2)
            return;
        client->setNickname(tokens[1]);
        std::cout << "Set NICK for FD " << client->getFd() << ": " << tokens[1] << std::endl;
    }
    else if (cmd == "USER")
    {
        if (tokens.size() < 2)
            return;
        client->setUsername(tokens[1]);
        std::cout << "Set USER for FD " << client->getFd() << ": " << tokens[1] << std::endl;

        // Finish registration if everything is set
        if (client->isRegistered() && !client->getNickname().empty())
        {
            std::string welcome = ":ircserv 001 " + client->getNickname() + " :Welcome to IRC!\r\n";
            send(client->getFd(), welcome.c_str(), welcome.length(), 0);
        }
    }
    else if (cmd == "JOIN")
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
        channel->addClient(client);

        std::string msg = ":" + client->getNickname() + " JOIN " + channelName + "\r\n";
        const std::set<Client *> &members = channel->getClients();
        for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
        {
            send((*it)->getFd(), msg.c_str(), msg.length(), 0);
        }

        std::cout << "FD " << client->getFd() << " joined " << channelName << std::endl;
    }
    else if (cmd == "PRIVMSG")
    {
        if (tokens.size() < 3)
            return;

        std::string target = tokens[1];

        // Rebuild message content after ":"
        std::string message;
        size_t colonPos = line.find(":", line.find("PRIVMSG"));
        if (colonPos != std::string::npos)
            message = line.substr(colonPos + 1);
        else
            return;

        std::string fullMsg = ":" + client->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";

        if (target[0] == '#')
        {
            // Channel message
            if (_channels.find(target) == _channels.end())
                return;
            Channel *channel = _channels[target];

            const std::set<Client *> &members = channel->getClients();
            for (std::set<Client *>::const_iterator it = members.begin(); it != members.end(); ++it)
            {
                if (*it != client)
                { // Don't echo to sender
                    send((*it)->getFd(), fullMsg.c_str(), fullMsg.length(), 0);
                }
            }
        }
        else
        {
            // Private message to user
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
    }
}
void Server::handleClientData(size_t index)
{
    int clientFd = _pollFds[index].fd;
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0)
    {
        std::cout << "Client disconnected: FD " << clientFd << std::endl;
        removeClient(index);
        return;
    }

    Client *client = _clients[clientFd];
    client->appendBuffer(std::string(buffer));

    std::string &buf = const_cast<std::string &>(client->getBuffer());
    size_t pos;
    while ((pos = buf.find("\n")) != std::string::npos)
    {
        std::string line = buf.substr(0, pos);
        buf.erase(0, pos + 1);
        handleCommand(client, trim(line));
    }
}

void Server::removeClientByFd(int fd)
{
    for (size_t i = 0; i < _pollFds.size(); ++i)
    {
        if (_pollFds[i].fd == fd)
        {
            removeClient(i);
            break;
        }
    }
}

void Server::removeClient(size_t index)
{
    int fd = _pollFds[index].fd;

    if (_clients.find(fd) != _clients.end())
    {
        delete _clients[fd];
        _clients.erase(fd);
    }

    close(fd);
    _pollFds.erase(_pollFds.begin() + index);
}
