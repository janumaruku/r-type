/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight demo world
*/

#ifndef HOLLOW_KNIGHT_WORLD_HPP
#define HOLLOW_KNIGHT_WORLD_HPP

#include "Components.hpp"
#include "engine/ecs/Registry.hpp"

namespace hollow_knight {
class World {
public:
    World();

    void restart();

    void update(const PlayerInput &input, float dt);

    [[nodiscard]] bool restAtBench();

    [[nodiscard]] const engine::ecs::Registry &registry() const noexcept;

    [[nodiscard]] const RoomState &state() const;

    [[nodiscard]] const Player &player() const;

private:
    engine::ecs::Registry _registry;

    void spawnPlayer();

    void spawnEnemy(const EnemyStart &start);

    void startNextWave();

    void saveCheckpoint();

    void restoreCheckpoint();
};
} // namespace hollow_knight

#endif // HOLLOW_KNIGHT_WORLD_HPP
