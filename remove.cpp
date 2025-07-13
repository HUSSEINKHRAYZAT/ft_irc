#include "header.hpp"


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

void Server::removeClient(size_t index) {
    int fd = _pollFds[index].fd;
    Client* client = _clients[fd];

    // 🔁 Replace your old channel-removal loop with this:
    for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ) {
        Channel* channel = it->second;
        if (channel->hasClient(client)) {
            channel->removeClient(client);
            if (channel->getClients().empty()) {
                std::map<std::string, Channel*>::iterator toErase = it++;
                delete toErase->second;
                _channels.erase(toErase);
                continue;
            }
        }
        ++it;
    }


    delete _clients[fd];
    _clients.erase(fd);
    close(fd);
    _pollFds.erase(_pollFds.begin() + index);

    std::cout << "✅ Disconnected client FD: " << fd << std::endl;
}

