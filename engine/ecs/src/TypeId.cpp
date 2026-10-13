/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** typeId
*/

#include "engine/ecs/TypeId.hpp"

namespace engine::ecs::detail {
std::size_t nextTypeId()
{
    static std::atomic<std::size_t> counter{0};

    return ++counter;
}
}
