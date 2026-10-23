/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CommandBuffer
*/

#ifndef R_TYPE_COMMANDBUFFER_TPP
#define R_TYPE_COMMANDBUFFER_TPP

#include <type_traits>
#include <utility>

#include "engine/ecs/CommandBuffer.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::ecs {
template <ComponentConcept Component>
void CommandBuffer::add(const Entity &entity, Component value)
{
    static_assert(std::is_copy_constructible_v<Component>,
        "commands are stored in std::function, so a deferred add needs a "
        "copyable component; add a move-only one immediately, outside any "
        "view loop");

    auto func = [entity, value = std::move(value)](
        Registry &registry) -> void {
        if (registry.isAlive(entity))
            registry.addComponent(entity, value);
    };

    _commands.emplace_back(std::move(func));
}

template <ComponentConcept Component>
void CommandBuffer::remove(const Entity &entity)
{
    auto func = [entity](Registry &registry) -> void {
        if (registry.isAlive(entity))
            registry.removeComponent<Component>(entity);
    };

    _commands.emplace_back(std::move(func));
}
}

#endif // R_TYPE_COMMANDBUFFER_TPP
