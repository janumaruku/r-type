/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** LoopbackHelpers
*/

#ifndef R_TYPE_LOOPBACK_HELPERS_HPP
#define R_TYPE_LOOPBACK_HELPERS_HPP

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iterator>
#include <string>
#include <thread>
#include <vector>

#include "network/Packet.hpp"
#include "network/UdpSocket.hpp"

namespace client_tests {

/**
 * @brief The loopback address of a started socket, the one a peer on this
 * machine sends to.
 */
inline asio::ip::udp::endpoint loopbackOf(const net::UdpSocket &socket)
{
    return {asio::ip::address_v4::loopback(), socket.localEndpoint().port()};
}

/**
 * @brief Waits up to `timeout` for `count` datagrams, and returns those
 * that arrived.
 */
inline std::vector<net::Datagram> waitForDatagrams(net::UdpSocket &receiver,
    std::size_t count,
    std::chrono::milliseconds timeout = std::chrono::milliseconds(1000))
{
    std::vector<net::Datagram> received;
    const auto deadline = std::chrono::steady_clock::now() + timeout;

    while (received.size() < count
        && std::chrono::steady_clock::now() < deadline) {
        auto batch = receiver.receiveAll(count - received.size());
        received.insert(received.end(),
            std::make_move_iterator(batch.begin()),
            std::make_move_iterator(batch.end()));
        if (received.size() < count)
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    return received;
}

/**
 * @brief A packet holding exactly `bytes`, to send what no encoder would.
 */
inline net::Packet rawPacket(const std::vector<std::uint8_t> &bytes)
{
    net::Packet packet;

    packet.writeRaw(bytes.data(), bytes.size());
    return packet;
}

/**
 * @brief How many times `needle` appears in `text`.
 */
inline std::size_t occurrences(const std::string &text,
    const std::string &needle)
{
    std::size_t count = 0;

    for (auto at = text.find(needle); at != std::string::npos;
        at = text.find(needle, at + needle.size()))
        ++count;
    return count;
}

}

#endif //R_TYPE_LOOPBACK_HELPERS_HPP
