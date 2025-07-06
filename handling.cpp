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
    _pollFds.push_back(newPollFd);

    _clients[clientFd] = new Client(clientFd);

    std::cout << "New client connected: FD " << clientFd << std::endl;
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
