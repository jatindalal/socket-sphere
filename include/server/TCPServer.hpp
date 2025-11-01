#pragma once

#include "boost/system/detail/error_code.hpp"
#include <iostream>
#include <boost/asio.hpp>
#include <memory>

class TCPServer
{
public:
    using tcp = boost::asio::ip::tcp;
    explicit TCPServer(boost::asio::io_context& context,
                       unsigned short port)
        : m_context(context)
        , m_acceptor(context, tcp::endpoint(tcp::v4(), port))
    { listen(); }

    void listen()
    {
        m_acceptor.async_accept(
            [this] (const boost::system::error_code& ec, tcp::socket&& socket) {
                if (!ec) {
                    std::cout << "client connected: "
                            << socket.remote_endpoint() << std::endl;
                    auto client = std::make_shared<tcp::socket>(std::move(socket));
                    m_clients.push_back(client);
                    do_read(client);
                }
                listen();
            }
        );
    }

    void do_read(std::shared_ptr<tcp::socket> client) 
    {
        auto buf = std::make_shared<std::vector<char>>(m_buffer_size);
        client->async_read_some(boost::asio::buffer(*buf),
            [this, client, buf](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(buf->data(), length);
                    std::cout << "Received: " << msg << "\n";
                    do_read(client);
                } else {
                    std::cout << "Client disconnected\n";
                    m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
                }
            }
        );
    }

    inline static constexpr unsigned short m_buffer_size = 4096;
private:
    std::deque<std::shared_ptr<tcp::socket>> m_clients;
    boost::asio::io_context& m_context;
    tcp::acceptor m_acceptor;
};
