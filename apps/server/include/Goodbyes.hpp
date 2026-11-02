/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Goodbyes.hpp
*/

#ifndef GOODBYES_HPP
#define GOODBYES_HPP

#include <vector>


#include "rtype/protocol/Event.hpp"

namespace rtype::server {
class Goodbyes {
public:
    void add(const asio::ip::udp::endpoint &to, protocol::MessageType type, net::Packet packet);
    [[nodiscard]] std::vector<std::pair<asio::ip::udp::endpoint, net::Packet>> takeDue();

private:
    struct Pending { asio::ip::udp::endpoint to; net::Packet packet; std::uint8_t copiesLeft; };
    std::vector<Pending> _pending;
};
} // namespace rtype::server

#endif // GOODBYES_HPP