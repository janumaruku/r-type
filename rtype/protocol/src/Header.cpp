/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Header
*/

#include "rtype/protocol/Header.hpp"

#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
Header::Header(MessageType myType): type(myType)
{
}

void serialize(net::Packet &packet, const Header &header)
{
    packet << header.magic << header.version << header.type << header.sequence;
}

void deserialize(net::Packet &packet, Header &header)
{
    packet >> header.magic;
    packet >> header.version;

    MessageType temp;
    packet >> temp;
    if (isKnown(temp))
        throw ProtocolError(DecodeError::UNKNOW_TYPE, "unknown message type");

    header.type = temp;
    packet >> header.sequence;
}
}
