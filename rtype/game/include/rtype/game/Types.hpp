/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Types
*/

#ifndef R_TYPE_TYPES_HPP
#define R_TYPE_TYPES_HPP

#include <cstddef>
#include <cstdint>

namespace rtype::game {
using NetId = std::uint32_t;
inline constexpr NetId NO_NET_ID = 0;
inline constexpr std::uint8_t NO_PLAYER = 0xFF;

enum class EntityKind : std::uint8_t {
    PLAYER_SHIP = 1,
    BYDOS_SLAVE = 2,
    PLAYER_MISSILE = 3,
    ENEMY_MISSILE = 4,
};

enum class CollisionLayer : std::uint8_t {
    PLAYER,
    ENEMY,
    PLAYER_SHOT,
    ENEMY_SHOT,
};

enum class MovePattern : std::uint8_t {
    STRAIGHT = 0,
    SINE = 1,
};

enum class Side : std::uint8_t {
    PLAYER,
    ENEMY,
};

enum class LeaveReason : std::uint8_t { QUIT = 0, TIMEOUT = 1, KICKED = 2 };

enum class SlotState : std::uint8_t {
    EMPTY = 0,
    ALIVE = 1,
    RESPAWNING = 2,
    OUT = 3,
};

enum class Phase : std::uint8_t { PLAYING, GAME_OVER };

struct FloatRange {
    float min = 0.0F;
    float max = 0.0F;
};

struct InputButtons {
    static constexpr std::uint8_t UP = 1U << 0U;
    static constexpr std::uint8_t DOWN = 1U << 1U;
    static constexpr std::uint8_t LEFT = 1U << 2U;
    static constexpr std::uint8_t RIGHT = 1U << 3U;
    static constexpr std::uint8_t FIRE = 1U << 4U;

    std::uint8_t bits = 0;

    [[nodiscard]] constexpr bool held(const std::uint8_t button) const noexcept
    {
        return (bits & button) == 0;
    }

    bool operator==(const InputButtons &) const = default;
};
}

#endif //R_TYPE_TYPES_HPP
