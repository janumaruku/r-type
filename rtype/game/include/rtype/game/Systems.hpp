/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Systems
*/

#ifndef R_TYPE_SYSTEMS_HPP
#define R_TYPE_SYSTEMS_HPP

#include "rtype/game/Components.hpp"
#include "engine/ecs/Registry.hpp"

namespace rtype::game {
void playerInputSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<PlayerShip> &ships,
    engine::ecs::SparseSet<Velocity> &velocities,
    engine::ecs::SparseSet<Weapon> &weapons);

void movementSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &transforms,
    const engine::ecs::SparseSet<Velocity> &velocities);

void playfieldSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &transforms,
    const engine::ecs::SparseSet<Collider> &colliders);

void straightMoverSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<StraightMover> &movers,
    engine::ecs::SparseSet<Velocity> &velocities);

void sineMoverSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<SineMover> &movers,
    engine::ecs::SparseSet<Velocity> &velocities);

void aimedShooterSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<AimedShooter> &shooters,
    const engine::ecs::SparseSet<Transform> &transforms,
    engine::ecs::SparseSet<Weapon> &weapons);

void weaponSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Weapon> &weapons,
    const engine::ecs::SparseSet<Transform> &transforms,
    const engine::ecs::SparseSet<Replicated> &replicated);

void collisionSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Collider> &colliders,
    const engine::ecs::SparseSet<Transform> &transforms,
    engine::ecs::SparseSet<Health> &healths);

void healthSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Health> &healths,
    const engine::ecs::SparseSet<Replicated> &replicated);

void lifetimeSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Lifetime> &lifetimes);

void spawnDirectorSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Replicated> &replicated);

void respawnSystem(engine::ecs::Registry &registry);

void gameOverSystem(engine::ecs::Registry &registry,
    const engine::ecs::SparseSet<Replicated> &replicated);

void netIdCleanupSystem(engine::ecs::Registry &registry);
}

#endif //R_TYPE_SYSTEMS_HPP
