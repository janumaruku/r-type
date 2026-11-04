/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Goodbyes.cpp
*/

#include "Goodbyes.hpp"

#include <utility>

namespace rtype::server {

void Goodbyes::add(const asio::ip::udp::endpoint &to,
    net::Packet packet)
{
    Pending p {.to = to, .packet = std::move(packet), .copiesLeft = BASE_COPIES};
    _pending.push_back(std::move(p));
}

std::vector<std::pair<asio::ip::udp::endpoint, net::Packet>>
Goodbyes::takeDue()
{
    std::vector<std::pair<asio::ip::udp::endpoint, net::Packet>> due;
    for (auto it = _pending.begin(); it != _pending.end();) {
        if (it->copiesLeft == 0) {
            it = _pending.erase(it);
            continue;
        }
        due.emplace_back(it->to, it->packet);
        --it->copiesLeft;
        ++it;
    }
    return due;
}

} // namespace rtype::server
