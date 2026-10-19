/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#include "engine/ecs/Registry.hpp"

namespace engine::ecs {
Entity Registry::spawnEntity()
{
    if (!_freeIndices.empty()) {
        const auto id = _freeIndices.back();
        const auto generation = _generations[id];
        const Entity entity{id, generation};

        _alive[id] = true;
        ++_aliveCount;
        _freeIndices.pop_back();

        return entity;
    }

    if (_alive.size() >= Entity::NONE.index())
        throw core::EngineError("entity limit reached");

    _alive.push_back(true);

    try {
        _generations.push_back(0);
    } catch (...) {
        _alive.pop_back();
        throw;
    }
    ++_aliveCount;

    return detail::EntityAccess::make(
        static_cast<std::uint32_t>(_alive.size() - 1U), 0);
}

bool Registry::isAlive(const Entity &entity) const noexcept
{
    return entity.index() < _alive.size() && _alive[entity.index()]
        && _generations[entity.index()] == entity.generation();
}

void Registry::killEntity(const Entity &entity)
{
    if (!isAlive(entity))
        return;

    _freeIndices.reserve(_generations.size());

    for (auto &storage: _storages)
        if (storage)
            storage->erase(entity);

    _freeIndices.push_back(entity.index());
    _alive[entity.index()] = false;
    _generations[entity.index()] += 1;
    --_aliveCount;
}

std::size_t Registry::aliveCount() const noexcept
{
    return _aliveCount;
}
}
