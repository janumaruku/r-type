/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetBase
*/

#include "engine/ecs/SparseSetBase.hpp"

bool engine::ecs::SparseSetBase::contains(const Entity &entity) const noexcept
{
    return entity.index() <= _sparse.size() - 1 &&
        _sparse[entity.index()] != NPOS &&
        _dense[_sparse[entity.index()]] == entity;
}
