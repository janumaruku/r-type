/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** CommandBuffer
*/

#include <cstddef>
#include <utility>

#include "engine/ecs/CommandBuffer.hpp"
#include "engine/ecs/Registry.hpp"

namespace engine::ecs {
void CommandBuffer::kill(const Entity &entity)
{
    auto func = [entity](Registry &registry) {
        registry.killEntity(entity);
    };

    _commands.emplace_back(std::move(func));
}

void CommandBuffer::spawn(SpawnInit init)
{
    auto func = [init = std::move(init)](Registry &registry) {
        const Entity entity = registry.spawnEntity();

        if (!init)
            return;
        try {
            init(registry, entity);
        } catch (...) {
            registry.killEntity(entity);
            throw;
        }
    };

    _commands.emplace_back(std::move(func));
}

/*
** The size is read again on each pass, so commands recorded while applying
** run in the same call. Each command is moved out before it runs: one that
** records another may reallocate the vector, which would move the running
** std::function under itself. A half-applied batch is never replayed, so the
** buffer is cleared even when a command throws.
*/
void CommandBuffer::apply(Registry &registry)
{
    try {
        for (std::size_t i = 0; i < _commands.size(); ++i) {
            const Command command = std::move(_commands[i]);
            command(registry);
        }
    } catch (...) {
        clear();
        throw;
    }
    clear();
}

std::size_t CommandBuffer::size() const noexcept
{
    return _commands.size();
}

bool CommandBuffer::empty() const noexcept
{
    return _commands.empty();
}

void CommandBuffer::clear() noexcept
{
    _commands.clear();
}
}
