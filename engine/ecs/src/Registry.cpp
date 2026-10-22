/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#include <cassert>
#include <chrono>
#include <cstddef>

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

Registry::RunGuard::RunGuard(bool &value): running(value)
{
    running = true;
}

Registry::RunGuard::~RunGuard()
{
    running = false;
}

/*
** Systems are re-indexed on each pass: with NDEBUG a system can still add
** another one, and the reallocation would leave an iterator dangling.
** The level is read once per stage, and the clock only at TRACE, so timing
** costs nothing otherwise.
** TODO(RTP-61): apply the commands recorded by each system right after it
** runs, before its duration is logged.
*/
void Registry::runStageAt(const std::uint8_t stage)
{
    assert(!_running);
    if (stage >= _stages.size())
        return;

    RunGuard guard(_running);
    const bool timed = core::log::isEnabled(core::log::Level::EG_TRACE);

    for (std::size_t i = 0; i < _stages[stage].size(); ++i) {
        if (!timed) {
            _stages[stage][i].run(*this);
            continue;
        }
        const auto start = std::chrono::steady_clock::now();
        _stages[stage][i].run(*this);
        const std::chrono::duration<double, std::micro> elapsed =
            std::chrono::steady_clock::now() - start;

        _logger.trace("{} took {:.1f} us", _stages[stage][i].name,
            elapsed.count());
    }
}
}
