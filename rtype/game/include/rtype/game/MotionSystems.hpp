/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MotionSystems
*/

#ifndef R_TYPE_MOTION_SYSTEMS_HPP
#define R_TYPE_MOTION_SYSTEMS_HPP

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
}

#endif //R_TYPE_MOTION_SYSTEMS_HPP
