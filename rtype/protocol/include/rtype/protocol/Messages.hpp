/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Messages
*/

#ifndef R_TYPE_MESSAGES_HPP
#define R_TYPE_MESSAGES_HPP

#include <cstddef>
#include <cstdint>
#include <string>

#include "rtype/protocol/Header.hpp"
#include "network/Packet.hpp"

namespace rtype::protocol {
inline constexpr std::size_t MAX_NAME_LENGTH = 16;
inline constexpr std::uint8_t MAX_PLAYERS = 4;

struct Hello {
    std::uint8_t protocolVersion = Header::VERSION;
    std::string playerName;

    bool operator==(const Hello &) const = default;
};

void serialize(net::Packet &packet, const Hello &message);
void deserialize(net::Packet &packet, Hello &message);
}

#endif //R_TYPE_MESSAGES_HPP
