/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Types
*/

#ifndef R_TYPE_TYPES_HPP
#define R_TYPE_TYPES_HPP

#include <cstdint>

namespace rtype::game {
using NetId = std::uint32_t;
inline constexpr NetId NO_NET_ID = 0;
inline constexpr std::uint8_t NO_PLAYER = 0xFF;

/**
 * @brief Mirrors EntityState's `kind` (RFC § 6.5). Keep the values equal to
 * the RFC's, so the server adapter converts with a checked `static_cast`.
 */
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

/**
 * @brief Mirrors PLAYER_LEFT's reason (RFC § 6.6). Keep the values equal to
 * the RFC's, so the server adapter converts with a checked `static_cast`.
 */
enum class LeaveReason : std::uint8_t {
    QUIT = 0,
    TIMEOUT = 1,
    KICKED = 2,
};

/**
 * @brief Mirrors PlayerInfo's `state` (RFC § 6.5). Keep the values equal to
 * the RFC's, so the server adapter converts with a checked `static_cast`.
 */
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

/**
 * @brief The bits mirror INPUT's `buttons` (RFC § 6.4). Keep them equal to
 * the RFC's, so the server adapter copies `bits` as is.
 */
struct InputButtons {
    static constexpr std::uint8_t UP = 1U << 0U;
    static constexpr std::uint8_t DOWN = 1U << 1U;
    static constexpr std::uint8_t LEFT = 1U << 2U;
    static constexpr std::uint8_t RIGHT = 1U << 3U;
    static constexpr std::uint8_t FIRE = 1U << 4U;

    std::uint8_t bits = 0;

    [[nodiscard]] constexpr bool held(const std::uint8_t button) const noexcept
    {
        return (bits & button) != 0;
    }

    bool operator==(const InputButtons &) const = default;
};
}

#endif //R_TYPE_TYPES_HPP
