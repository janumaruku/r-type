/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Header
*/

#ifndef R_TYPE_HEADER_HPP
#define R_TYPE_HEADER_HPP

#include <cstdint>

#include "MessageType.hpp"
#include "../../../../../network/include/network/Packet.hpp"

namespace rtype::protocol {
struct Header {
    static constexpr std::uint16_t MAGIC = 0x5254;
    static constexpr std::uint8_t VERSION = 1;
    static constexpr std::size_t SIZE = 8;

    explicit Header(MessageType type);

    std::uint16_t magic = MAGIC;
    std::uint8_t version = VERSION;
    MessageType type;
    std::uint32_t sequence = 0;
};

void serialize(net::Packet &packet, const Header &header);

void deserialize(net::Packet &packet, Header &header);

template <class Message>
[[nodiscard]] net::Packet makePacket(MessageType type, std::uint32_t sequence,
    const Message &message);
} // protocol
// rtype

#endif //R_TYPE_HEADER_HPP
