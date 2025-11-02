#include "boost/asio.hpp"
#include "boost/asio/connect.hpp"
#include "boost/asio/io_context.hpp"

#include "common/Utils.hpp"

#include <chrono>
#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

int main()
{
    boost::asio::io_context io_context;
    tcp::socket socket(io_context);
    tcp::resolver resolver(io_context);

    auto endpoints = resolver.resolve("127.0.0.1", "8000");
    boost::asio::connect(socket, endpoints);

    std::cout << "connected to server\n";
    std::cout << "Keep on typing stuff\n";
    while (true) {
        // std::string input_string;
        // std::getline(std::cin, input_string);
        auto input_string = random_string(10);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        boost::asio::write(socket, boost::asio::buffer(input_string));
    }

    socket.close();
    return 0;
}
