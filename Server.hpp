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
	Server(int port,
		   const std::string &password);
	~Server();
	void setupSocket();
	void handleNewConnection();
	bool handleClientData(size_t index);
	void removeClient(size_t index);
	bool handleCommand(Client *client, const std::string &line);
	void removeClientByFd(int fd);
	bool handlePass(Client *client, const std::vector<std::string> &tokens);
	void handleNick(Client *client, const std::vector<std::string> &tokens);
	void handleUser(Client *client, const std::vector<std::string> &tokens);
	void handleJoin(Client *client, const std::vector<std::string> &tokens);
	void handlePrivMsg(Client *client, const std::string &line, const std::vector<std::string> &tokens);
	void handleKick(Client *client, const std::vector<std::string> &tokens);
	void handleTopic(Client *client, const std::string &line, const std::vector<std::string> &tokens);
	void handleMode(Client *client, const std::vector<std::string> &tokens);
	void handleInvite(Client *client, const std::vector<std::string> &tokens);
	void handlePart(Client *client, const std::vector<std::string> &tokens);
	void handleNames(Client *client, const std::vector<std::string> &tokens);
	void handlePing(Client *client, const std::string &line);
	bool handleQuit(Client *client);
	void handlePrivateMessage(Client *client, const std::string &line, const std::vector<std::string> &tokens);
	void run();

private:
	int _serverSocket;
	int _port;
	std::string _password;
	std::map<int, Client *> _clients;
	std::map<std::string, Channel *> _channels;
	std::vector<struct pollfd> _pollFds;

	void	handleUserPrivMsg(Client *sender, const std::string &target, const std::string &fullMsg);
	void	handleChannelPrivMsg(Client *sender, const std::string &target, const std::string &fullMsg);
};

#endif // SERVER_HPP
