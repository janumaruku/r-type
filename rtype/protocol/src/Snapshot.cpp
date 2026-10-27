/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Snapshot
*/

#include "rtype/protocol/Snapshot.hpp"

#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/ProtocolError.hpp"

namespace rtype::protocol {

void serialize(net::Packet &packet, const PlayerInfo &message)
{
    packet << message.playerId << message.state << message.lives
        << message.score << message.shipNetId << message.name;
}

static bool isKnownPlayerState(const PlayerState state)
{
    switch (state) {
    case PlayerState::EMPTY:
    case PlayerState::ALIVE:
    case PlayerState::RESPAWNING:
    case PlayerState::OUT:
        return true;
    }

    return false;
}

void deserialize(net::Packet &packet, PlayerInfo &message)
{
    packet >> message.playerId >> message.state >> message.lives
        >> message.score >> message.shipNetId >> message.name;

    if (!isKnownPlayerState(message.state))
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: state out of range");

    if (message.lives > MAX_LIVES)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: lives out of range");

    if (message.name.size() > MAX_NAME_LENGTH)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: name too long");

    if (message.state == PlayerState::EMPTY && !message.name.empty())
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: name must be empty when state is EMPTY");

    if (message.state != PlayerState::ALIVE && message.shipNetId != 0)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: shipNetId must be 0 when state is not ALIVE");

    if (message.playerId >= MAX_PLAYERS)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "PLAYER_INFO: playerId must be between 0 and 3");
}

void serialize(net::Packet &packet, const EntityState &message)
{
    packet << message.netId << message.kind << message.variant
        << message.x << message.y << message.vx << message.vy;
}

void deserialize(net::Packet &packet, EntityState &message)
{
    packet >> message.netId >> message.kind >> message.variant
        >> message.x >> message.y >> message.vx >> message.vy;

    if (message.netId == 0)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "ENTITY_STATE: netId must not be 0");

    switch (message.kind) {
    case EntityKind::PLAYER_SHIP:
        if (message.variant >= MAX_PLAYERS)
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "ENTITY_STATE: variant out of range for PLAYER_SHIP");
        return;
    case EntityKind::BYDOS_SLAVE:
        if (message.variant > pattern::SINE)
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "ENTITY_STATE: variant out of range for BYDOS_SLAVE");
        return;
    case EntityKind::PLAYER_MISSILE:
    case EntityKind::ENEMY_MISSILE:
        if (message.variant != 0)
            throw ProtocolError(DecodeError::OUT_OF_RANGE,
                "ENTITY_STATE: variant must be 0 for a missile");
        return;
    }
    throw ProtocolError(DecodeError::OUT_OF_RANGE,
        "ENTITY_STATE: kind out of range");
}
}
