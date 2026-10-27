/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Messages
*/

#include "rtype/protocol/Messages.hpp"

#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {
void serialize(net::Packet &packet, const Hello &message)
{
    packet << message.protocolVersion << message.playerName;
}

void deserialize(net::Packet &packet, Hello &message)
{
    packet >> message.protocolVersion >> message.playerName;
    if (message.playerName.size() > 16U)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "HELLO: player name too long");
}
}
