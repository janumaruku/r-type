/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** GameWorld
*/

#ifndef R_TYPE_GAME_WORLD_HPP
#define R_TYPE_GAME_WORLD_HPP

#include <cstdint>
#include <optional>
#include <vector>

#include "rtype/game/Commands.hpp"
#include "rtype/game/Events.hpp"
#include "rtype/game/PlayerRoster.hpp"
#include "rtype/game/Rules.hpp"
#include "rtype/game/Types.hpp"
#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"

namespace rtype::game {
enum class Stage : std::uint8_t { SIMULATION };

struct ReplicatedEntity {
    NetId netId = NO_NET_ID;
    EntityKind kind = EntityKind::PLAYER_SHIP;
    std::uint8_t variant = 0;
    engine::core::Vec2f position{};
    engine::core::Vec2f velocity{};

    bool operator==(const ReplicatedEntity &) const = default;
};

struct GameWorldTestAccess;

class GameWorld {
public:
    GameWorld(Rules rules, std::uint32_t seed);

    void apply(const PlayerCommand &command);

    void step(float dt);

    [[nodiscard]] std::vector<GameEvent> drainEvents();

    void collectReplicated(std::vector<ReplicatedEntity> &out) const;

    [[nodiscard]] std::optional<std::uint8_t> reserveSlot();

    [[nodiscard]] const PlayerRoster &roster() const;

    [[nodiscard]] Phase phase() const;

    [[nodiscard]] std::uint32_t tick() const noexcept;

private:
    friend struct GameWorldTestAccess;

    /**
     * @brief `join` with the rules' lives, or `joinWaiting` during a game
     * over, then @ref PlayerJoined. The ship spawns on the next tick.
     */
    void handle(const JoinPlayer &command);

    /**
     * @brief Kills the player's ship and missiles, frees the slot, then
     * @ref PlayerLeft. A free slot does nothing.
     */
    void handle(const LeavePlayer &command);

    /**
     * @brief Copies the buttons onto the player's ship if the sequence is
     * newer than the last one. Without a ship, only the sequence is kept.
     */
    void handle(const SetInput &command);

    engine::ecs::Registry _registry;
    std::uint32_t _tick = 0;
};
}

#endif //R_TYPE_GAME_WORLD_HPP
