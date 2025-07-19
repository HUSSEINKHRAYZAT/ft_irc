#include "header.hpp"


std::vector<std::string> split(const std::string& str, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string item;
	while (std::getline(ss, item, delimiter))
		result.push_back(item);
	return result;
}

std::string trim(const std::string& s)
{
	size_t start = 0;
	while (start < s.size() && std::isspace(s[start])) ++start;
	size_t end = s.size();
	while (end > start && std::isspace(s[end - 1])) --end;
	return s.substr(start, end - start);
}

bool	isValidChannelName(const std::string &name)
{
	if (name.empty())
		return false;
	if (name[0] != '#')
		return false;
	return true;
}

std::vector<std::string> splitTrimmed(const std::string &line)
{
	std::vector<std::string> tokens;
	std::string word;
	bool inWord = false;

	for (std::size_t i = 0; i < line.length(); ++i)
	{
		char c = line[i];
		if (c != ' ' && c != '\t')
		{
			word += c;
			inWord = true;
		}
		else if (inWord)
		{
			tokens.push_back(word);
			word.clear();
			inWord = false;
		}
	}
	if (!word.empty())
		tokens.push_back(word);

	return tokens;
}
