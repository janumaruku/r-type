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
const SparseSet<C> & Registry::getComponents() const
{
    const auto index = detail::typeId<C>();

    if (index >= _storages.size() || _storages[index] == nullptr)
        throw core::EngineError(
            "component not registered: " + std::string{typeid(C).name()});

    assert(dynamic_cast<SparseSet<C> *>(_storages[index].get()) != nullptr);
    return static_cast<SparseSet<C> &>(*_storages[index]);
}
}

#endif
