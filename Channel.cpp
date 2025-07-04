#include "header.hpp"

Channel::Channel(const std::string& name) : _name(name) {}

Channel::~Channel() {}

const std::string& Channel::getName() const {
    return _name;
}

void Channel::addClient(Client* client) {
    _clients.insert(client);
}

void Channel::removeClient(Client* client) {
    _clients.erase(client);
}

bool Channel::hasClient(Client* client) const {
    return _clients.find(client) != _clients.end();
}

const std::set<Client*>& Channel::getClients() const {
    return _clients;
}
