#include <boost/asio.hpp>
#include <csignal>
#include <iostream>

#include "server/ChatServer.hpp"
#include "boost/asio/io_context.hpp"

#define PORT 8000

int main()
{
    boost::asio::io_context context;
    ChatServer server(context, PORT);

    std::cout << "Server running on port " << PORT << std::endl;
    context.run();

    return 0;
}
