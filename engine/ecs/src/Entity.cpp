/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Entity
*/

#include "engine/ecs/Entity.hpp"

namespace engine::ecs {
const Entity Entity::NONE{0xFFFFFFFF, 0xFFFFFFFF};

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

Entity::Entity(std::uint32_t index, std::uint32_t generation) noexcept:
    _index(index), _generation(generation)
{
}
}
