#pragma once

#include "network/Packet.hpp"

#include <array>
#include <atomic>
#include <cstdint>
#include <deque>
#include <mutex>
#include <thread>
#include <vector>

#include <asio.hpp>

namespace net {

class NetworkError : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

struct Datagram {
    asio::ip::udp::endpoint from;
    std::vector<std::uint8_t> bytes;
};

class UdpSocket {
public:
    explicit UdpSocket(std::uint16_t port = 0);
    ~UdpSocket();

    UdpSocket(const UdpSocket&) = delete;
    UdpSocket& operator=(const UdpSocket&) = delete;

    void start();
    void stop();

    void sendTo(const asio::ip::udp::endpoint& to, const Packet& packet);
    std::vector<Datagram> receiveAll(std::size_t max = 1024);

    asio::ip::udp::endpoint localEndpoint() const;
    [[nodiscard]] std::uint64_t dropped() const noexcept;

private:
    void receiveNext();

    asio::io_context _io;
    asio::executor_work_guard<asio::io_context::executor_type> _work;
    asio::ip::udp::socket _socket;
    std::thread _thread;
    std::array<std::uint8_t, 1500> _receiveBuffer{};
    asio::ip::udp::endpoint _sender;
    mutable std::mutex _mutex;
    std::deque<Datagram> _incoming;
    std::atomic<std::uint64_t> _dropped = 0;
    std::uint16_t _port;
    bool _started = false;
};

}