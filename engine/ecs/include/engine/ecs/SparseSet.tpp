/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSet
*/

#ifndef SPARSESET_TPP
#define SPARSESET_TPP

#include "engine/ecs/SparseSet.hpp"

namespace engine::ecs {
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
}

#endif
