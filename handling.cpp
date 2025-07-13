#include "header.hpp"

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
    newPollFd.revents = 0;
    _pollFds.push_back(newPollFd);

    _clients[clientFd] = new Client(clientFd);

    std::cout << "New client connected: FD " << clientFd << std::endl;
    std::string msg = "🟢 🟢 🟢 YOU ARE CONNECTED NOW TO THE SERVER YOU MUST ENTER YOUR CARDINALITES TO ACCESS IT\r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
    msg = "FIRST ENTER THE PASSWORD THEN THE USER THEN THE NICK \r\n";
    send(clientFd, msg.c_str(), msg.length(), 0);
}

bool Server::handleClientData(size_t index)
{
    int clientFd = _pollFds[index].fd;
    char buffer[512];
    std::memset(buffer, 0, sizeof(buffer));
    ssize_t bytesRead = recv(clientFd, buffer, sizeof(buffer) - 1, 0);

    if (bytesRead <= 0)
    {
        std::cout << "Client disconnected: FD " << clientFd << std::endl;
        removeClient(index);
        return false;
    }

    Client *client = _clients[clientFd];
    client->appendBuffer(std::string(buffer));

    std::string &buf = const_cast<std::string &>(client->getBuffer());
    size_t pos;
    while ((pos = buf.find("\n")) != std::string::npos)
    {
        std::string line = buf.substr(0, pos);
        buf.erase(0, pos + 1);

        if (!handleCommand(client, trim(line)))
            return false;  // client removed, stop

    }
    return true;
}
