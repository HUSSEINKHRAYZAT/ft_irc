#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Client;

class Channel
{
public:
	Channel(const std::string &name);
	~Channel();

	const std::string &getName() const;
	void addClient(Client *client);
	void removeClient(Client *client);
	bool hasClient(Client *client) const;
	const std::set<Client *> &getClients() const;
	void addOperator(Client *client);
	void removeOperator(Client* client);

	bool isOperator(Client *client) const;
	void setInviteOnly(bool mode);
	bool isInviteOnly() const;

	void invite(Client *client);
	bool isInvited(Client *client) const;
	void removeInvite(Client *client);

	const std::string &getTopic() const;
	// Key (password)
	void setKey(const std::string &key);
	void removeKey();
	bool hasKey() const;
	const std::string &getKey() const;
	bool hasOperators() const;

	// User limit
	void setUserLimit(int limit);
	void removeUserLimit();
	bool hasUserLimit() const;
	int getUserLimit() const;

	//topic permissions
	void setTopic(const std::string &topic);
	void setTopicPermission(bool mode);
	bool isTopicRestricted() const;


private:

	std::string _name;
	std::set<Client *> _clients;
	std::set<Client *> _operators;
	bool    _istopic;
	std::string _topic;
	bool _inviteOnly;
	std::set<Client *> _invitedUsers;
	std::string _key;
	bool _hasKey;

	int _userLimit;
	bool _hasLimit;
};

#endif
