#ifndef UTILS_HPP
#define UTILS_HPP

#include <vector>
#include <string>

std::vector<std::string>	split(const std::string& str, char delimiter);
std::string					trim(const std::string& s);
bool						isValidChannelName(const std::string &name);
std::vector<std::string>	splitTrimmed(const std::string &line);

#endif
