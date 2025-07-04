#ifndef SERVER_HPP
#define SERVER_HPP

#include <vector>
#include <string>
#include <map>
#include <poll.h>
#include <netinet/in.h>

class Client;
class Channel;

class Server
{
public:
    Server(int port, const std::string &password);
    ~Server();

    void run();

private:
    int _serverSocket;
    int _port;
    std::string _password;
    std::map<int, Client *> _clients; // maps fd -> Client*
    std::map<std::string, Channel *> _channels;
    std::vector<struct pollfd> _pollFds;

    void setupSocket();
    void handleNewConnection();
    void handleClientData(size_t index);
    void removeClient(size_t index);
    void handleCommand(Client *client, const std::string &line);
    void removeClientByFd(int fd);
};

#endif // SERVER_HPP
