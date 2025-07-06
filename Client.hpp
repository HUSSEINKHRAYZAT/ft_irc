#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>

class Client {
public:
    Client(int fd);
    ~Client();

    int getFd() const;
    const std::string& getNickname() const;
    const std::string& getUsername() const;
    const std::string& getBuffer() const;

    void setNickname(const std::string& nickname);
    void setUsername(const std::string& username);
    void appendBuffer(const std::string& data);
    void clearBuffer();
    bool isRegistered() const;
    void setRegistered(bool status);

private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _buffer;
    bool _registered;
};

#endif
