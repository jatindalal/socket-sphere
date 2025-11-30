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
#include <list>

class ChatServer
{
public:
    using tcp = boost::asio::ip::tcp;
    static constexpr uint32_t       s_max_msg_size = 1024 * 1024;
    static constexpr size_t         s_max_write_queue_size = 128;
    inline static constexpr size_t  s_client_id_len = 10;

    struct Client
    {
        tcp::socket m_socket;
        std::string m_id;
        std::array<char, 4> m_message_header;
        std::vector<char> m_message_body;

        std::deque<std::string> m_write_queue;
        bool m_writing = false;

        std::list<std::shared_ptr<Client>>::iterator m_self_iterator;

        Client(tcp::socket&& socket)
        : m_socket(std::move(socket))
        , m_id (random_string(s_client_id_len)) {}
    };
    explicit ChatServer(boost::asio::io_context& context,
                       unsigned short port)
        : m_context(context)
        , m_acceptor(context, tcp::endpoint(tcp::v4(), port))
    { listen(); }

private:
    boost::asio::io_context& m_context;
    tcp::acceptor m_acceptor;
    std::list<std::shared_ptr<Client>> m_clients;

    void listen()
    {
        m_acceptor.async_accept(
            [this] (auto ec, tcp::socket&& socket) {
                if (!ec) {
                    add_client(std::move(socket));
                }
                listen();
            }
        );
    }

    void add_client(tcp::socket&& socket) {
        auto client = std::make_shared<Client>(std::move(socket));
        m_clients.push_back(client);

        std::cout << "client connected: "
                  << client->m_socket.remote_endpoint() << " id: "
                  << client->m_id << std::endl;

        start_read_header(client);
    }

    static uint32_t decode_length(const std::array<char,4>& header) {
            boost::endian::big_uint32_t len_be;
            std::memcpy(&len_be, header.data(), 4);
            return static_cast<uint32_t>(len_be);
    }

    void start_read_header(std::shared_ptr<Client> client)
    {
        boost::asio::async_read(client->m_socket,
            boost::asio::buffer(client->m_message_header),
            [this, client] (auto ec, std::size_t) {
                if (ec) {
                    handle_disconnect(client);
                    return;
                }

                uint32_t len = decode_length(client->m_message_header);

                if (len == 0 || len > s_max_msg_size) {
                    handle_disconnect(client);
                    return;
                }

                client->m_message_body.resize(len);
                start_read_body(client);
            }
        );
    }

    void start_read_body(std::shared_ptr<Client> client)
    {
        boost::asio::async_read(client->m_socket,
            boost::asio::buffer(client->m_message_body),
            [this, client] (auto ec, std::size_t) {
                if (ec) {
                    handle_disconnect(client);
                    return;
                }

                std::string message(client->m_message_body.begin(), client->m_message_body.end());
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
        std::memcpy(packet.data(), &len_be, 4);
        std::memcpy(packet.data() + 4, message.data(), message.size());

        if (client->m_write_queue.size() >= s_max_write_queue_size) {
            handle_disconnect(client);
            return;
        }

        client->m_write_queue.push_back(std::move(packet));
        if (!client->m_writing) start_write(client);
    }

    void start_write(std::shared_ptr<Client> client) {
        client->m_writing = true;
        boost::asio::async_write(
            client->m_socket,
            boost::asio::buffer(client->m_write_queue.front()),
            [this, client] (boost::system::error_code ec, size_t) {
                if (ec) {
                    handle_disconnect(client);
                    return;
                }

                client->m_write_queue.pop_front();
                if (!client->m_write_queue.empty()) start_write(client);
                else client->m_writing = false;
            }
        );
    }

    void handle_disconnect(const std::shared_ptr<Client>& client)
    {
        std::cout << "Client " << client->m_id << " disconnected\n";
        client->m_socket.shutdown(tcp::socket::shutdown_both);
        client->m_socket.close();
        m_clients.erase(client->m_self_iterator);
    }

    void broadcast(const std::string& message,
                   const std::shared_ptr<Client>& sender)
    {
        for (auto& client: m_clients) {
            if (client == sender) continue;
            enqueue_write(client, message);
        }
    }
};
