/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** NetIdAllocator
*/

#include "rtype/game/NetIdAllocator.hpp"

#include "engine/ecs/Registry.hpp"

namespace rtype::game {

NetId NetIdAllocator::allocate(engine::ecs::Entity entity)
{
    if (_next == NO_NET_ID)
        throw engine::core::EngineError("NetIdAllocator: ran out of NetIds");

    const NetId netId = _next++;

    _byNetId.emplace(netId, entity);
    return netId;
}

std::optional<engine::ecs::Entity> NetIdAllocator::find(const NetId netId,
    const engine::ecs::Registry &registry) const
{
    const auto it = _byNetId.find(netId);

    if (it == _byNetId.end() || !registry.isAlive(it->second))
        return std::nullopt;

    return it->second;
}

void NetIdAllocator::releaseDead(const engine::ecs::Registry &registry)
{
    std::erase_if(_byNetId, [&registry](const auto &pair) {
        return !registry.isAlive(pair.second);
    });
}

std::size_t NetIdAllocator::size() const noexcept
{
    return _byNetId.size();
}
}
