/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight ECS systems
*/

#ifndef HOLLOW_KNIGHT_SYSTEMS_HPP
#define HOLLOW_KNIGHT_SYSTEMS_HPP

#include "Components.hpp"
#include "engine/ecs/Registry.hpp"

namespace hollow_knight {
void addSystems(engine::ecs::Registry &registry);

void playerControllerSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Player> &, engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Body> &);

void enemySystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Enemy> &, engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Body> &);

void physicsSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &, engine::ecs::SparseSet<Body> &);

void combatSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Player> &, engine::ecs::SparseSet<Enemy> &);
} // namespace hollow_knight

#endif // HOLLOW_KNIGHT_SYSTEMS_HPP
