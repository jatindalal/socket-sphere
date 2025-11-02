#pragma once

#include "boost/system/detail/error_code.hpp"

#include "common/Utils.hpp"

#include <iostream>
#include <boost/asio.hpp>
#include <memory>

class TCPServer
{
public:
    using tcp = boost::asio::ip::tcp;
    inline static constexpr size_t s_client_id_len = 10;
    struct Client
    {
        tcp::socket m_socket;
        std::string m_id;

        Client(tcp::socket&& socket)
        : m_socket(std::move(socket))
        , m_id (random_string(s_client_id_len)) {}
    };
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
                    auto client = std::make_shared<Client>(std::move(socket));
                    m_clients.push_back(client);
                    std::cout << "client connected: "
                              << client->m_socket.remote_endpoint() << " id: "
                              << client->m_id << std::endl;
                    do_read(client);
                }
                listen();
            }
        );
    }

    void do_read(std::shared_ptr<Client> client) 
    {
        auto buf = std::make_shared<std::vector<char>>(m_buffer_size);
        client->m_socket.async_read_some(boost::asio::buffer(*buf),
            [this, client, buf](boost::system::error_code ec, std::size_t length) {
                if (!ec) {
                    std::string msg(buf->data(), length);
                    // std::cout << "Received: " << msg << "\n";
                    std::cout << "Client " << client->m_id << " messaged!\n";
                    do_read(client);
                } else {
                    std::cout << "Client " << client->m_id << " disconnected\n";
                    m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
                }
            }
        );
    }

    inline static constexpr unsigned short m_buffer_size = 4096;
private:
    std::deque<std::shared_ptr<Client>> m_clients;
    boost::asio::io_context& m_context;
    tcp::acceptor m_acceptor;
};
