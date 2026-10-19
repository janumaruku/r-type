/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Registry
*/

#ifndef R_TYPE_REGISTRY_TPP
#define R_TYPE_REGISTRY_TPP

#include <cassert>
#include <string>
#include <typeinfo>

#include "Errors.hpp"
#include "engine/ecs/Registry.hpp"
#include "engine/ecs/TypeId.hpp"

namespace engine::ecs {
template <Component C>
SparseSet<C> &Registry::registerComponent()
{
    const auto index = detail::typeId<C>();

    if (index >= _storages.size())
        _storages.resize(index + 1);

    if (_storages[index] == nullptr)
        _storages[index] = std::make_unique<SparseSet<C>>();

    assert(dynamic_cast<SparseSet<C> *>(_storages[index].get()) != nullptr);
    return static_cast<SparseSet<C> &>(*_storages[index]);
}

template <Component C>
SparseSet<C> &Registry::getComponents()
{
    const auto index = detail::typeId<C>();

    if (index >= _storages.size() || _storages[index] == nullptr)
        throw core::EngineError(
            "component not registered: " + std::string{typeid(C).name()});

    assert(dynamic_cast<SparseSet<C> *>(_storages[index].get()) != nullptr);
    return static_cast<SparseSet<C> &>(*_storages[index]);
}

template <Component C>
const SparseSet<C> &Registry::getComponents() const
{
    const auto index = detail::typeId<C>();

    if (index >= _storages.size() || _storages[index] == nullptr)
        throw core::EngineError(
            "component not registered: " + std::string{typeid(C).name()});

    assert(dynamic_cast<SparseSet<C> *>(_storages[index].get()) != nullptr);
    return static_cast<SparseSet<C> &>(*_storages[index]);
}

template <typename C> requires Component<std::remove_cvref_t<C>>
std::remove_cvref_t<C> &Registry::addComponent(const Entity &entity,
    C &&component)
{
    auto &storage = getComponents<std::remove_cvref_t<C>>();

    if (!isAlive(entity))
        throw core::EngineError("can't add a component to a dead entity");

    return storage.insertAt(entity, std::forward<C>(component));
}

template <Component C, typename... Args>
C &Registry::emplaceComponent(const Entity &entity, Args &&... args)
{
    auto &storage = getComponents<C>();

    if (!isAlive(entity))
        throw core::EngineError("can't add a component to a dead entity");

    return storage.emplaceAt(entity, std::forward<Args>(args)...);
}

template <Component C>
void Registry::removeComponent(const Entity &entity)
{
    getComponents<C>().erase(entity);
}

template <Component C>
C *Registry::getComponent(const Entity &entity)
{
    auto &storage = getComponents<C>();

    return isAlive(entity) ? storage.tryGet(entity) : nullptr;
}

template <Component C>
const C *Registry::getComponent(const Entity &entity) const
{
    const auto &storage = getComponents<C>();

    return isAlive(entity) ? storage.tryGet(entity) : nullptr;
}
}

#endif
