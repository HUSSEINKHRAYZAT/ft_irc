#include "header.hpp"

static volatile sig_atomic_t g_running = 1;

// SIGINT handler
extern "C" void handle_sigint(int)
{
    g_running = 0;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./ircserv <port> <password>" << std::endl;
        return 1;
    }

    int port = std::atoi(argv[1]);
    std::string password = argv[2];

    try
    {
        std::signal(SIGINT, handle_sigint);
        Server server(port, password);
        server.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
