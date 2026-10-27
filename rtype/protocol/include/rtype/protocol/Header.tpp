/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Header
*/

#ifndef R_TYPE_HEADER_TPP
#define R_TYPE_HEADER_TPP

#include "rtype/protocol/MessageType.hpp"

namespace rtype::protocol {
template <class Message>
net::Packet makePacket(MessageType type, std::uint32_t sequence,
    const Message &message)
{
    Header header(type);
    header.sequence = sequence;

    net::Packet packet;
    packet << header << message;

    return packet;
}
}

#endif // R_TYPE_HEADER_TPP
