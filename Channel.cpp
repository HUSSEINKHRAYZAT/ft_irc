#include "header.hpp"

Channel::Channel(const std::string& name)
    : _name(name),
      _topic(""),
      _inviteOnly(false),
      _key(""),
      _hasKey(false),
      _userLimit(0),
      _hasLimit(false)
{}


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
void Channel::addOperator(Client* client) {
    _operators.insert(client);
}
void Channel::removeOperator(Client* client) {
    _operators.erase(client);
}


bool Channel::isOperator(Client* client) const {
    return _operators.find(client) != _operators.end();
}
void Channel::setTopic(const std::string& topic) {
    _topic = topic;
}

const std::string& Channel::getTopic() const {
    return _topic;
}
void Channel::setInviteOnly(bool mode) {
    _inviteOnly = mode;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}

void Channel::invite(Client* client) {
    _invitedUsers.insert(client);
}

bool Channel::isInvited(Client* client) const {
    return _invitedUsers.find(client) != _invitedUsers.end();
}

void Channel::removeInvite(Client* client) {
    _invitedUsers.erase(client);
}
void Channel::setKey(const std::string& key) {
    _key = key;
    _hasKey = true;
}

void Channel::removeKey() {
    _key = "";
    _hasKey = false;
}

bool Channel::hasKey() const {
    return _hasKey;
}

const std::string& Channel::getKey() const {
    return _key;
}

void Channel::setUserLimit(int limit) {
    _userLimit = limit;
    _hasLimit = true;
}

void Channel::removeUserLimit() {
    _userLimit = 0;
    _hasLimit = false;
}

bool Channel::hasUserLimit() const {
    return _hasLimit;
}

int Channel::getUserLimit() const {
    return _userLimit;
}

bool Channel::hasOperators() const
{
    return !_operators.empty();
}

// Set topic permission (+t mode)
void Channel::setTopicPermission(bool mode)
{
	_istopic = mode;
}

// Check if topic permission (+t) is enabled
bool Channel::isTopicRestricted() const
{
	return _istopic;
}

