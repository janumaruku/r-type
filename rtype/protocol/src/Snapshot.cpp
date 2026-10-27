/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Snapshot
*/

#include "rtype/protocol/Snapshot.hpp"

#include <algorithm>
#include <cstddef>
#include <format>

#include "engine/core/Errors.hpp"
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

void serialize(net::Packet &packet, const Snapshot &message)
{
    packet << message.serverTick << message.lastProcessedInput
        << message.chunkIndex << message.chunkCount << message.roster
        << message.entities;
}

static bool isRosterInSlotOrder(const std::vector<PlayerInfo> &roster)
{
    for (std::size_t slot = 0; slot < roster.size(); ++slot)
        if (roster[slot].playerId != slot)
            return false;

    return true;
}

void deserialize(net::Packet &packet, Snapshot &message)
{
    packet >> message.serverTick >> message.lastProcessedInput
        >> message.chunkIndex >> message.chunkCount >> message.roster
        >> message.entities;

    if (message.chunkCount == 0 || message.chunkCount > MAX_CHUNKS)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: chunkCount out of range");

    if (message.chunkIndex >= message.chunkCount)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: chunkIndex out of range");

    if (!message.roster.empty() && message.roster.size() != MAX_PLAYERS)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: roster must have 0 or 4 entries");

    if (message.chunkIndex == 0 && message.roster.empty())
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: chunk 0 must carry the roster");

    if (message.chunkIndex != 0 && !message.roster.empty())
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: only chunk 0 may carry the roster");

    if (message.entities.size() > MAX_ENTITIES_PER_CHUNK)
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: too many entities");

    if (!isRosterInSlotOrder(message.roster))
        throw ProtocolError(DecodeError::OUT_OF_RANGE,
            "SNAPSHOT: roster not in slot order");
}

std::vector<Snapshot> splitSnapshot(const Snapshot &full)
{
    const std::size_t total = full.entities.size();
    constexpr std::size_t maxEntities = MAX_CHUNKS * MAX_ENTITIES_PER_CHUNK;

    if (total > maxEntities)
        throw engine::core::EngineError(std::format(
            "splitSnapshot: {} entities, at most {} fit in one tick", total,
            maxEntities));

    const std::size_t count = std::max<std::size_t>(1,
        (total + MAX_ENTITIES_PER_CHUNK - 1) / MAX_ENTITIES_PER_CHUNK);
    std::vector<Snapshot> chunks;

    chunks.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        const std::size_t first = index * MAX_ENTITIES_PER_CHUNK;
        const std::size_t last =
            std::min(total, first + MAX_ENTITIES_PER_CHUNK);

        chunks.push_back(Snapshot{
            .serverTick = full.serverTick,
            .lastProcessedInput = full.lastProcessedInput,
            .chunkIndex = static_cast<std::uint8_t>(index),
            .chunkCount = static_cast<std::uint8_t>(count),
            .roster = index == 0 ? full.roster : std::vector<PlayerInfo>{},
            .entities = {
                full.entities.begin() + static_cast<std::ptrdiff_t>(first),
                full.entities.begin() + static_cast<std::ptrdiff_t>(last),
            },
        });
    }
    return chunks;
}
}
