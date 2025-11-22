#pragma once

#include "boost/asio/read.hpp"
#include "boost/asio/write.hpp"
#include "boost/endian/arithmetic.hpp"
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

        std::deque<std::string> write_queue;
        bool writing = false;

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
                    handle_disconnect(client);
                    return;
                }

                boost::endian::big_uint32_t len_be;
                std::memcpy(&len_be, client->message_header.data(), 4);

                uint32_t len = static_cast<uint32_t>(len_be);
                if (len == 0 || len > s_max_msg) {
                    handle_disconnect(client);
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
                    handle_disconnect(client);
                    return;
                }

                std::string message(client->message_body.begin(), client->message_body.end());
                std::cout << "Client " << client->m_id << ": " << message << std::endl;

                broadcast(client->m_id + ": " + message, client);
                start_read_header(client);
            }
        );
    }

    void enqueue_write(std::shared_ptr<Client> client, const std::string& message)
    {
        uint32_t len = static_cast<uint32_t>(message.size());
        boost::endian::big_uint32_t len_be = len;

        std::string packet;
        packet.resize(4 + message.size());
        std::memcpy(&packet[0], &len_be, 4);
        std::memcpy(&packet[4], message.data(), message.size());

        client->write_queue.push_back(std::move(packet));
        if (!client->writing) start_write(client);
    }

    void start_write(std::shared_ptr<Client> client) {
        client->writing = true;
        boost::asio::async_write(
            client->m_socket,
            boost::asio::buffer(client->write_queue.front()),
            [this, client] (boost::system::error_code ec, size_t) {
                if (ec) {
                    handle_disconnect(client);
                    return;
                }

                client->write_queue.pop_front();
                if (!client->write_queue.empty()) start_write(client);
                else client->writing = false;
            }
        );
    }

    void handle_disconnect(const std::shared_ptr<Client>& client)
    {
        std::cout << "Client " << client->m_id << " disconnected\n";
        client->m_socket.close();
        m_clients.erase(std::remove(m_clients.begin(),
                                    m_clients.end(),
                                    client),
                        m_clients.end());
    }

    void broadcast(const std::string& message,
                   const std::shared_ptr<Client>& sender)
    {
        for (auto& client: m_clients) {
            if (!client || client == sender) continue;
            
            enqueue_write(client, message);
        }
    }
    static constexpr uint32_t s_max_msg = 1024 * 1024;
private:
    std::deque<std::shared_ptr<Client>> m_clients;
    boost::asio::io_context& m_context;
    tcp::acceptor m_acceptor;
};
