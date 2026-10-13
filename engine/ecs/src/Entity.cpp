/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Entity
*/

#include "ecs/Entity.hpp"

namespace engine::ecs {
std::uint32_t Entity::index() const noexcept
{
    return _index;
}

std::uint32_t Entity::generation() const noexcept
{
    return _generation;
}

bool Entity::operator==(const Entity &rhs) const noexcept
{
    return _generation == rhs._generation && _index == rhs._index;
}

Entity::Entity(std::uint32_t index, std::uint32_t generation): _index(index),
    _generation(generation)
{
}
}
