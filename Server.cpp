#include "header.hpp"


Server::Server(int port, const std::string &password)
    : _serverSocket(-1), _port(port), _password(password)
{
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
    addr.sin_addr.s_addr = htonl(INADDR_ANY);;
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
    std::cout << "👋 Welcome to IRC server ..." << std::endl;
    std::cout << "🟢 🟢 🟢 Server listening on port  " << _port << " 🧲 "<<std::endl;
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
    return;
}
