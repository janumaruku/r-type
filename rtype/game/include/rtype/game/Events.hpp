/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Events
*/

#ifndef R_TYPE_EVENTS_HPP
#define R_TYPE_EVENTS_HPP

#include <variant>
#include <vector>

#include "rtype/game/Types.hpp"
#include "engine/core/Vec2.hpp"

namespace rtype::game {
struct Fired {
    NetId shooter;
    NetId missile;
    Side side;
    engine::core::Vec2f position;
};

struct EnemyKilled {
    NetId killerShip;
    NetId enemy;
    engine::core::Vec2f position;
};

struct PlayerKilled {
    NetId killer;
    NetId ship;
    std::uint8_t playerId;
    engine::core::Vec2f position;
};

struct PlayerJoined {
    std::uint8_t playerId;
};

struct PlayerLeft {
    std::uint8_t playerId;
    LeaveReason reason;
};

struct GameOver {
    std::uint32_t teamScore;
};

struct GameRestarted {
};

using GameEvent = std::variant<Fired, EnemyKilled, PlayerKilled, PlayerJoined,
    PlayerLeft, GameOver, GameRestarted>;

class EventOutbox {
public:
    void push(const GameEvent &event);

    [[nodiscard]] std::vector<GameEvent> drain();

    [[nodiscard]] std::size_t size() const noexcept;

private:
    std::vector<GameEvent> _events;
};
}

#endif //R_TYPE_EVENTS_HPP
