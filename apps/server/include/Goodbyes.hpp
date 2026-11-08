/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Goodbyes.hpp
*/

#ifndef GOODBYES_HPP
#define GOODBYES_HPP

#include <cstdint>
#include <vector>

#include "asio.hpp"
#include "network/Packet.hpp"

namespace rtype::server {
class Goodbyes {
public:
    Goodbyes() = default;
    void add(const asio::ip::udp::endpoint &to, net::Packet packet);
    [[nodiscard]] std::vector<std::pair<asio::ip::udp::endpoint, net::Packet>>
        takeDue();

private:
    static constexpr uint8_t BASE_COPIES = 3;

    struct Pending {
        asio::ip::udp::endpoint to;
        net::Packet packet;
        std::uint8_t copiesLeft;
    };
    std::vector<Pending> _pending;
};
} // namespace rtype::server

#endif // GOODBYES_HPP
