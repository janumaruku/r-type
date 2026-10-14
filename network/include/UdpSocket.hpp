/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** UdpSocket.hpp
*/

#ifndef UDP_SOCKET_HPP
#define UDP_SOCKET_HPP

#include <array>
#include <asio.hpp>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <mutex>
#include <vector>

#include "Packet.hpp"

namespace net {

struct Datagram {
    std::vector<std::uint8_t> bytes;
    asio::ip::udp::endpoint from;
};

class UdpSocket {
public:
    static constexpr std::size_t MAX_PACKET_SIZE = 1200;

    explicit UdpSocket(std::uint16_t port);
    ~UdpSocket();

    UdpSocket()                  = delete;
    UdpSocket(const UdpSocket &) = delete;
    UdpSocket(UdpSocket &&)      = delete;

    UdpSocket &operator=(const UdpSocket &) = delete;
    UdpSocket &operator=(UdpSocket &&)      = delete;

    void start();
    void stop();

    void sendTo(const asio::ip::udp::endpoint &endpoint, const Packet &packet);
    [[nodiscard]] std::vector<Datagram> receiveAll(std::size_t max = 1024);

    [[nodiscard]] asio::ip::udp::endpoint localEndpoint() const;
    [[nodiscard]] std::uint64_t dropped() const;

private:

    void receiveNext();

    std::uint16_t _port = 0;
    asio::io_context _ioContext;
    asio::executor_work_guard<asio::io_context::executor_type> _workGuard;
    asio::ip::udp::socket _socket;

    std::thread _thread;
    std::array<std::uint8_t, 1500> _receiveBuffer;

    asio::ip::udp::endpoint _sender;
    std::mutex _mutex;
    std::deque<Datagram> _incoming;
    std::atomic<std::uint64_t> _dropped;
};

} // namespace net

#endif
