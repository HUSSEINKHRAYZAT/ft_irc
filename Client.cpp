#include "header.hpp"


Client::Client(int fd)
    : _fd(fd),
      _nickname(""),
      _username(""),
      _registered(false),
      _passGiven(false),
      _userGiven(false),
      _nickGiven(false){}

Client::~Client() {}

int Client::getFd() const
{
    return _fd;
}

const std::string &Client::getNickname() const
{
    return _nickname;
}

const std::string &Client::getUsername() const
{
    return _username;
}

const std::string &Client::getBuffer() const
{
    return _buffer;
}

void Client::setNickname(const std::string &nickname)
{
    _nickname = nickname;
}

void Client::setUsername(const std::string &username)
{
    _username = username;
}

void Client::appendBuffer(const std::string &data)
{
    _buffer += data;
}

void Client::clearBuffer()
{
    _buffer.clear();
}

bool Client::isRegistered() const
{
    return _registered;
}

void Client::setRegistered(bool status)
{
    _registered = status;
}
void Client::setPassGiven(bool val) { _passGiven = val; };
void Client::setUserGiven(bool val) { _userGiven = val; };
void Client::setNickGiven(bool val) { _nickGiven = val; };

bool Client::isPassGiven() const { return _passGiven; };
bool Client::isUserGiven() const { return _userGiven; };
bool Client::isNickGiven() const { return _nickGiven; };