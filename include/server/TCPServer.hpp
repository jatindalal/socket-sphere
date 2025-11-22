#pragma once

#include "boost/asio/read.hpp"
#include "boost/system/detail/error_code.hpp"

#include "common/Utils.hpp"

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <boost/asio.hpp>
#include <boost/endian.hpp>
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
        std::array<char, 4> message_header;
        std::vector<char> message_body;

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
                    start_read_header(client);
                }
                listen();
            }
        );
    }

    void start_read_header(std::shared_ptr<Client> client) 
    {
        boost::asio::async_read(client->m_socket,
            boost::asio::buffer(client->message_header),
            [this, client] (boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << "Client " << client->m_id << " disconnected\n";
                    m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
                    return;
                }

                boost::endian::big_uint32_t len_be;
                std::memcpy(&len_be, client->message_header.data(), 4);

                uint32_t len = static_cast<uint32_t>(len_be);
                if (len == 0 || len > s_max_msg) {
                    std::cout << "Client " << client->m_id << " disconnected (bad message)\n";
                    client->m_socket.close();
                    m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
                    return;
                }

                client->message_body.resize(len);
                start_read_body(client);
            }
        );
    }

    void start_read_body(std::shared_ptr<Client> client)
    {
        boost::asio::async_read(client->m_socket,
            boost::asio::buffer(client->message_body),
            [this, client] (boost::system::error_code ec, std::size_t) {
                if (ec) {
                    std::cout << "Client " << client->m_id << " disconnected\n";
                    m_clients.erase(std::remove(m_clients.begin(), m_clients.end(), client), m_clients.end());
                    return;
                }

                std::string message(client->message_body.begin(), client->message_body.end());
                std::cout << "Client " << client->m_id << ": " << message << std::endl;

                start_read_header(client);
            }
        );
    }

    static constexpr uint32_t s_max_msg = 1024 * 1024;
private:
    std::deque<std::shared_ptr<Client>> m_clients;
    boost::asio::io_context& m_context;
    tcp::acceptor m_acceptor;
};
