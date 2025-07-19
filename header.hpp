#ifndef HEADER_HPP
#define HEADER_HPP

// ─────────────────────────────────────────────────────────────
// 🌐 Standard C++ headers: Strings, containers, algorithms, etc.
// ─────────────────────────────────────────────────────────────
#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <vector>       // std::vector
#include <map>          // std::map
#include <set>          // std::set
#include <sstream>      // std::stringstream
#include <cstdlib>      // std::atoi
#include <cstring>      // std::memset, std::strerror
#include <algorithm>    // std::find, std::remove, etc.
#include <cctype>       // std::isspace, std::isalpha, etc.
#include <csignal>

// ─────────────────────────────────────────────────────────────
// 📡 System & Networking headers: sockets, polling, etc.
// ─────────────────────────────────────────────────────────────
#include <sys/types.h>      // Primitive system types
#include <sys/socket.h>     // socket(), bind(), listen(), accept()
#include <netinet/in.h>     // sockaddr_in, htons()
#include <arpa/inet.h>      // inet_ntoa(), inet_addr()
#include <netdb.h>          // getaddrinfo(), gethostbyname()
#include <unistd.h>         // close(), read(), write()
#include <fcntl.h>          // fcntl() for setting non-blocking
#include <poll.h>           // poll()
#include <errno.h>          // errno, error codes
#include <signal.h>         // signal(), sigaction()

// ─────────────────────────────────────────────────────────────
// 📁 Project headers: core classes for IRC server
// ─────────────────────────────────────────────────────────────
#include "Server.hpp"       
#include "Utils.hpp"       
#include "Client.hpp"       
#include "Channel.hpp"   
#include <arpa/inet.h>   // for inet_pton
#include <netinet/in.h>  // for sockaddr_in
#include <fcntl.h>       // for fcntl  
#include <termios.h>

#endif
