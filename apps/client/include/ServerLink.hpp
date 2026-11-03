/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ServerLink
*/

#ifndef R_TYPE_SERVER_LINK_HPP
#define R_TYPE_SERVER_LINK_HPP

#include <cstdint>

#include "network/UdpSocket.hpp"
#include "rtype/protocol/MessageType.hpp"

namespace rtype::client {
using Clock = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

struct ServerLink {
    net::UdpSocket socket{0};
    asio::ip::udp::endpoint server;
    std::uint32_t sendSequence = 0;
    TimePoint lastHeard{};

    template <class Message>
    void send(protocol::MessageType type, const Message &message);
};

[[nodiscard]] asio::ip::udp::endpoint resolveServer(const std::string &host,
    std::uint16_t port);
}

#endif //R_TYPE_SERVER_LINK_HPP
