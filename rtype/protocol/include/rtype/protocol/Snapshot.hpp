/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Snapshot
*/

#ifndef R_TYPE_SNAPSHOT_HPP
#define R_TYPE_SNAPSHOT_HPP

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "network/Packet.hpp"

namespace rtype::protocol {
inline constexpr std::size_t MAX_ENTITIES_PER_CHUNK = 48;
inline constexpr std::uint8_t MAX_CHUNKS = 16;
inline constexpr std::uint8_t MAX_LIVES = 3;

enum class PlayerState : std::uint8_t {
    EMPTY = 0,
    ALIVE = 1,
    RESPAWNING = 2,
    OUT = 3,
};

struct PlayerInfo {
    std::uint8_t playerId = 0;
    PlayerState state = PlayerState::EMPTY;
    std::uint8_t lives = 0;
    std::uint32_t score = 0;
    std::uint32_t shipNetId = 0;
    std::string name;

    bool operator==(const PlayerInfo &) const = default;
};

void serialize(net::Packet &packet, const PlayerInfo &message);

void deserialize(net::Packet &packet, PlayerInfo &message);

enum class EntityKind : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS_SLAVE = 2,
    PLAYER_MISSILE = 3,
    ENEMY_MISSILE = 4,
};

namespace pattern {
inline constexpr std::uint8_t STRAIGHT = 0;
inline constexpr std::uint8_t SINE = 1;
}

struct EntityState {
    std::uint32_t netId = 0;
    EntityKind kind = EntityKind::PLAYER_SHIP;
    std::uint8_t variant = 0;
    float x = 0.0F;
    float y = 0.0F;
    float vx = 0.0F;
    float vy = 0.0F;

    bool operator==(const EntityState &) const = default;
};

void serialize(net::Packet &packet, const EntityState &message);

void deserialize(net::Packet &packet, EntityState &message);

struct Snapshot {
    std::uint32_t serverTick = 0;
    std::uint32_t lastProcessedInput = 0;
    std::uint8_t chunkIndex = 0;
    std::uint8_t chunkCount = 1;
    std::vector<PlayerInfo> roster;
    std::vector<EntityState> entities;

    bool operator==(const Snapshot &) const = default;
};

void serialize(net::Packet &packet, const Snapshot &message);

void deserialize(net::Packet &packet, Snapshot &message);

/*
** Splits a whole tick into chunks of at most MAX_ENTITIES_PER_CHUNK entities,
** with the roster in chunk 0 only. full's chunkIndex and chunkCount are
** ignored. More than MAX_CHUNKS * MAX_ENTITIES_PER_CHUNK entities is a server
** bug, not network input, so it throws engine::core::EngineError.
*/
[[nodiscard]] std::vector<Snapshot> splitSnapshot(const Snapshot &full);
}

#endif //R_TYPE_SNAPSHOT_HPP
