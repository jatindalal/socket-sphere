#include "boost/asio.hpp"
#include "boost/asio/connect.hpp"
#include "boost/asio/io_context.hpp"

#include <iostream>

using boost::asio::ip::tcp;

int main()
{
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    auto endpoints = resolver.resolve("127.0.0.1", "8000");
    boost::asio::connect(socket, endpoints);

    std::cout << "connected to server\n";

    boost::asio::write(socket, boost::asio::buffer("hello from client\n"));

    socket.close();
    return 0;
}
