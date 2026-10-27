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
}

#endif //R_TYPE_SNAPSHOT_HPP
