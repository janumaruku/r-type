/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSet
*/

#ifndef SPARSESET_TPP
#define SPARSESET_TPP

#include <assert.h>

#include "engine/ecs/SparseSet.hpp"

namespace engine::ecs {
template <Component C>
const C &SparseSet<C>::get(const Entity &entity) const noexcept
{
    assert(contains(entity));

    return _components[positionOf(entity)];
}

template <Component C>
C *SparseSet<C>::tryGet(const Entity &entity) const noexcept
{
    return contains(entity) ? &_components[positionOf(entity)] : nullptr;
}

template <Component C>
C &SparseSet<C>::insertAt(const Entity &entity, const C &value)
{
    if (contains(entity)) {
        C &component = _components[positionOf(entity)];
        component = value;
        return component;
    }

    _components.push_back(value);
    append(entity);

    return _components.back();
}

template <Component C>
C &SparseSet<C>::insertAt(const Entity &entity, C &&value)
{
    if (contains(entity)) {
        C &component = _components[positionOf(entity)];
        component = std::move(value);
        return component;
    }

    _components.push_back(std::move(value));
    append(entity);

    return _components.back();
}

template <Component C>
template <typename... Args>
C &SparseSet<C>::emplaceAt(const Entity &entity, const Args &... args)
{
    if (contains(entity)) {
        C &component = _components[positionOf(entity)];
        component = C{std::forward<Args>(args)...};
        return component;
    }

    _components.emplace_back(std::forward<Args>(args)...);
    append(entity);

    return _components.back();
}

template <Component C>
void SparseSet<C>::erase(const Entity &entity) noexcept
{
    if (!contains(entity))
        return;

    const auto position = positionOf(entity);
    _components[position] = std::move(_components.back());
    _components.pop_back();
    swapAndPop(entity);
}

template <Component C>
std::span<C> SparseSet<C>::components() const noexcept
{
    return _components;
}
}

#endif
