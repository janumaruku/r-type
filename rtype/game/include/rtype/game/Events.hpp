/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Events
*/

#ifndef R_TYPE_EVENTS_HPP
#define R_TYPE_EVENTS_HPP

#include <cstddef>
#include <cstdint>
#include <variant>
#include <vector>

#include "engine/core/Vec2.hpp"
#include "rtype/game/Types.hpp"

namespace rtype::game {
struct Fired {
    NetId shooter;
    NetId missile;
    Side side;
    engine::core::Vec2f position;

    bool operator==(const Fired &) const = default;
};

struct EnemyKilled {
    NetId killerShip;
    NetId enemy;
    engine::core::Vec2f position;

    bool operator==(const EnemyKilled &) const = default;
};

struct PlayerKilled {
    NetId killer;
    NetId ship;
    std::uint8_t playerId;
    engine::core::Vec2f position;

    bool operator==(const PlayerKilled &) const = default;
};

struct PlayerJoined {
    std::uint8_t playerId;

    bool operator==(const PlayerJoined &) const = default;
};

struct PlayerLeft {
    std::uint8_t playerId;
    LeaveReason reason;

    bool operator==(const PlayerLeft &) const = default;
};

struct GameOver {
    std::uint32_t teamScore;

    bool operator==(const GameOver &) const = default;
};

struct GameRestarted {
    bool operator==(const GameRestarted &) const = default;
};

using GameEvent = std::variant<Fired, EnemyKilled, PlayerKilled, PlayerJoined,
    PlayerLeft, GameOver, GameRestarted>;

class EventOutbox {
public:
    void push(GameEvent event);

    [[nodiscard]] std::vector<GameEvent> drain();

    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<GameEvent> _events;
};
}

#endif //R_TYPE_EVENTS_HPP
