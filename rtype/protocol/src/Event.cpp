/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Event
*/

#include "rtype/protocol/Event.hpp"

#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {


void serialize(net::Packet &packet, const Event &message)
{
    packet << message.eventId << message.type << message.a << message.b
        << message.x << message.y;
}

/*
** b is a u32 on the wire, so it is range-checked before the cast: 256 would
** otherwise wrap to QUIT.
*/
static bool isKnownLeaveReason(const std::uint32_t reason)
{
    if (reason > UINT8_MAX)
        return false;
    switch (static_cast<LeaveReason>(reason)) {
    case LeaveReason::QUIT:
    case LeaveReason::TIMEOUT:
    case LeaveReason::KICKED:
        return true;
    }

    return false;
}

void deserialize(net::Packet &packet, Event &message)
{
    packet >> message.eventId >> message.type >> message.a >> message.b
        >> message.x >> message.y;

    switch (message.type) {
    case EventType::PLAYER_FIRED:
    case EventType::ENEMY_FIRED:
    case EventType::ENEMY_KILLED:
    case EventType::PLAYER_KILLED:
    case EventType::GAME_OVER:
    case EventType::GAME_RESTARTED:
        return;
    case EventType::PLAYER_JOINED:
        if (message.a >= MAX_PLAYERS)
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "EVENT: playerId out of range for PLAYER_JOINED");
        return;
    case EventType::PLAYER_LEFT:
        if (message.a >= MAX_PLAYERS)
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "EVENT: playerId out of range for PLAYER_LEFT");
        if (!isKnownLeaveReason(message.b))
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "EVENT: reason out of range for PLAYER_LEFT");
        return;
    }
    throw ProtocolError(DecodeError::OUT_OF_RANGE,
        "EVENT: type out of range");
}
}
