/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** World
*/

#ifndef R_TYPE_ECS_EXAMPLE_WORLD_HPP
#define R_TYPE_ECS_EXAMPLE_WORLD_HPP

#include <cstdint>

#include "Components.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/Registry.hpp"

namespace ecs_example {
using engine::ecs::Entity;
using engine::ecs::Registry;

/*
** Fills an empty registry: resources, the player, the stars and every
** system of both stages. Restarting builds a new registry the same way.
*/
void buildWorld(Registry &registry, std::uint32_t seed);

void makeEnemy(Registry &registry, const Entity &entity, Vec2f at,
    float speed);

void makeBullet(Registry &registry, const Entity &entity, Vec2f at);

void addGameplaySystems(Registry &registry);

void addRenderSystems(Registry &registry);
}

#endif // R_TYPE_ECS_EXAMPLE_WORLD_HPP
