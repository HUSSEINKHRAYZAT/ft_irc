#ifndef HEADER_HPP
#define HEADER_HPP


#include <iostream>     // std::cout, std::cerr
#include <string>       // std::string
#include <vector>       // std::vector
#include <map>          // std::map
#include <set>          // std::set
#include <sstream>      // std::stringstream
#include <cstdlib>      // std::atoi()
#include <cstring>      // std::memset(), std::strerror()
#include <algorithm>    // std::find(), etc.

//networking headers
#include <sys/types.h>      // data types
#include <sys/socket.h>     // socket(), bind(), listen(), accept()
#include <netinet/in.h>     // sockaddr_in, htons(), etc.
#include <arpa/inet.h>      // inet_ntoa(), inet_addr()
#include <netdb.h>          // getaddrinfo(), gethostbyname(), etc.
#include <unistd.h>         // close()
#include <fcntl.h>          // fcntl() for non-blocking
#include <poll.h>           // poll()
#include <errno.h>          // errno
#include <signal.h>         // signal(), sigaction()

//files headers
#include "Server.hpp"
#include "Utils.hpp"
#include "Client.hpp"
#include "Channel.hpp"

#include <sstream>
#include <cctype>
#include "Server.hpp"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#endif