//
// Created by janumaruku on 10/13/26.
//

#include "../include/entity.hpp"

namespace engine {
namespace ecs {
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
} // ecs
} // engine
