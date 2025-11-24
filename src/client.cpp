#include "boost/asio/io_context.hpp"

#include "common/Utils.hpp"
#include "client/ChatClient.hpp"

#include <iostream>
#include <thread>

using boost::asio::ip::tcp;

int main()
{
    boost::asio::io_context io_context;
    ChatClient client(io_context);
    client.connect("127.0.0.1", "8000");

    std::cout << "connected to server\n";
    std::cout << "Keep on typing stuff\n";

    std::thread context_thread([&io_context] () { io_context.run(); });
    while (true) {
        std::string input_string;
        std::getline(std::cin, input_string);
        client.send(input_string);
    }

    context_thread.join();
    return 0;
}
