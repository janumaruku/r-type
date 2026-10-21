/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSet
*/

#ifndef SPARSESET_TPP
#define SPARSESET_TPP

#include <cassert>

#include "engine/ecs/SparseSet.hpp"

namespace engine::ecs {
template <ComponentConcept Component>
Component &SparseSet<Component>::get(const Entity &entity) noexcept
{
    assert(contains(entity));

    return _components[positionOf(entity)];
}

template <ComponentConcept Component>
const Component &SparseSet<Component>::get(const Entity &entity) const noexcept
{
    assert(contains(entity));

    return _components[positionOf(entity)];
}

template <ComponentConcept Component>
Component *SparseSet<Component>::tryGet(const Entity &entity) noexcept
{
    return contains(entity) ? &_components[positionOf(entity)] : nullptr;
}

template <ComponentConcept Component>
const Component *SparseSet<Component>::tryGet(
    const Entity &entity) const noexcept
{
    return contains(entity) ? &_components[positionOf(entity)] : nullptr;
}

template <ComponentConcept Component>
Component &SparseSet<Component>::insertAt(const Entity &entity,
    const Component &value)
{
    if (contains(entity)) {
        Component &component = _components[positionOf(entity)];
        component = value;
        return component;
    }

    _components.push_back(value);
    exceptionSafeAppend(entity);

    return _components.back();
}

template <ComponentConcept Component>
Component &SparseSet<Component>::insertAt(const Entity &entity,
    Component &&value)
{
    if (contains(entity)) {
        Component &component = _components[positionOf(entity)];
        component = std::move(value);
        return component;
    }

    _components.push_back(std::move(value));
    exceptionSafeAppend(entity);

    return _components.back();
}

template <ComponentConcept Component>
template <typename... Args>
Component &SparseSet<Component>::emplaceAt(const Entity &entity,
    Args &&... args)
{
    if (contains(entity)) {
        Component &component = _components[positionOf(entity)];
        component = Component(std::forward<Args>(args)...);
        return component;
    }

    _components.emplace_back(std::forward<Args>(args)...);
    exceptionSafeAppend(entity);

    return _components.back();
}

template <ComponentConcept Component>
void SparseSet<Component>::erase(const Entity &entity)
{
    if (!contains(entity))
        return;

    const auto position = positionOf(entity);
    if (position != _components.size() - 1)
        _components[position] = std::move(_components.back());

    _components.pop_back();
    swapAndPop(entity);
}

template <ComponentConcept Component>
std::span<Component> SparseSet<Component>::components() noexcept
{
    return _components;
}

template <ComponentConcept Component>
std::span<const Component> SparseSet<Component>::components() const noexcept
{
    return _components;
}

template <ComponentConcept Component>
void SparseSet<Component>::exceptionSafeAppend(const Entity &entity)
{
    try {
        append(entity);
    } catch (...) {
        _components.pop_back();
        throw;
    }
}
}

#endif
