#pragma once

#include "boost/asio/read.hpp"
#include "boost/endian/arithmetic.hpp"
#include "boost/system/detail/error_code.hpp"
#include <boost/asio.hpp>
#include <boost/endian.hpp>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>

class ChatClient
{
public:
    using tcp = boost::asio::ip::tcp;
    explicit ChatClient(boost::asio::io_context& context)
    : m_context(context)
    , m_socket(m_context)
    , m_resolver(m_context) {}

    void connect(const std::string& host, const std::string& port)
    {
        auto endpoints = m_resolver.resolve(host, port);
        boost::asio::async_connect(m_socket, endpoints,
            [this, host, port] (const boost::system::error_code& ec, tcp::endpoint) {
                if (!ec) {
                    start_read_header();
                } else {
                    std::stringstream error;
                    error << "Couldn't connect to " << host << ":" << port;
                    throw std::runtime_error(error.str());
                }
            }
        );
    }

    void send(const std::string& message)
    {
        boost::endian::big_uint32_t len = static_cast<uint32_t>(message.size());

        std::string packet;
        packet.resize(4 + message.size());

        std::memcpy(&packet[0], &len, 4);
        std::memcpy(&packet[4], message.data(), message.size());

        boost::asio::post(m_context,
            [this, packet] () {
                bool writing = !m_write_queue.empty();
                m_write_queue.push_back(packet);
                if (!writing) start_write();
            }
        );
    }

    void close()
    {
        boost::asio::post(m_context, [this] () { m_socket.close(); });
    }
private:

    void start_read_header() {
        boost::asio::async_read(m_socket,
            boost::asio::buffer(m_message_header),
            [this] (boost::system::error_code ec, std::size_t) {
                if (ec) return;

                boost::endian::big_uint32_t len_be;
                std::memcpy(&len_be, m_message_header.data(), 4);

                uint32_t len = static_cast<uint32_t>(len_be);
                if (len == 0 || len > s_max_msg) {
                    m_socket.close();
                    return;
                }

                m_message_body.resize(len);
                start_read_body();
            }
        );
    }

    void start_read_body()
    {
        boost::asio::async_read(m_socket,
            boost::asio::buffer(m_message_body),
            [this] (boost::system::error_code ec, std::size_t) {
                if (ec) return;

                std::string message(m_message_body.begin(), m_message_body.end());
                std::cout << message << std::endl;

                start_read_header();
            }
        );
    }

    void start_write() {
        boost::asio::async_write(m_socket,
            boost::asio::buffer(m_write_queue.front()),
            [this] (boost::system::error_code ec, std::size_t) {
                if (ec) return;

                m_write_queue.pop_front();

                if (!m_write_queue.empty()) start_write();
            }
        );
    }

    boost::asio::io_context& m_context;
    tcp::socket m_socket;
    tcp::resolver m_resolver;
    std::array<char, 4> m_message_header;
    std::vector<char> m_message_body;
    std::deque<std::string> m_write_queue;
    static constexpr uint32_t s_max_msg = 1024 * 1024;
};