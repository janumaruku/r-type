/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Header
*/

#include "rtype/protocol/Header.hpp"

#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
void serialize(net::Packet &packet, const Header &header)
{
    packet << header.magic << header.version << header.type << header.sequence;
}

void deserialize(net::Packet &packet, Header &header)
{
    packet >> header.magic;
    if (header.magic != Header::MAGIC)
        throw ProtocolError(DecodeError::BAD_MAGIC, "Header: bad magic");

    packet >> header.version;
    packet >> header.type;
    packet >> header.sequence;
}
}
