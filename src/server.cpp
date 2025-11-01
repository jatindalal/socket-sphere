#include <boost/asio.hpp>
#include <csignal>
#include <iostream>
#include <thread>

#include "TCPServer.hpp"
#include "boost/asio/io_context.hpp"

#define PORT 8000

int main()
{
    boost::asio::io_context context;
    TCPServer server(context, PORT);


    std::vector<std::thread> workers;
    unsigned num_threads = std::thread::hardware_concurrency();
    for (unsigned i = 0; i < num_threads; ++i)
        workers.emplace_back([&context] () { context.run(); });

    for (auto& worker: workers) worker.join();

    return 0;
}
