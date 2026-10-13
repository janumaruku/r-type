/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** typeId
*/

#include "engine/ecs/TypeId.hpp"

#include <atomic>

namespace engine::ecs::detail {
std::size_t nextTypeId() noexcept
{
    static std::atomic<std::size_t> counter{0};

    return counter++;
}
}
