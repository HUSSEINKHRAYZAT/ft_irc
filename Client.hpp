#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client
{
public:
    Client(int fd);
    ~Client();

    int getFd() const;
    const std::string &getNickname() const;
    const std::string &getUsername() const;
    const std::string &getBuffer() const;

    void setNickname(const std::string &nickname);
    void setUsername(const std::string &username);
    void appendBuffer(const std::string &data);
    void clearBuffer();
    bool isRegistered() const;
    void setRegistered(bool status);
    void setPassGiven(bool val);
    void setUserGiven(bool val);
    void setNickGiven(bool val);

    bool isPassGiven() const ;
    bool isUserGiven() const ;
    bool isNickGiven() const ;

private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _buffer;
    bool _registered;

private:
    bool _passGiven;
    bool _userGiven;
    bool _nickGiven;
};

#endif
