/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** SparseSetBase
*/

#include "engine/ecs/SparseSetBase.hpp"

#include <cassert>
#include <utility>

namespace engine::ecs {
bool SparseSetBase::contains(const Entity &entity) const noexcept
{
    return entity.index() < _sparse.size() &&
        _sparse[entity.index()] != NPOS &&
        _dense[_sparse[entity.index()]] == entity;
}

std::size_t SparseSetBase::size() const noexcept
{
    return _dense.size();
}

std::span<const Entity> SparseSetBase::
entities() const noexcept
{
    return _dense;
}

bool SparseSetBase::empty() const noexcept
{
    return _dense.empty();
}

SparseSetBase::size_type SparseSetBase::positionOf(
    const Entity &entity) const noexcept
{
    assert(contains(entity));

    return _sparse[entity.index()];
}

void SparseSetBase::swapAndPop(const Entity &entity) noexcept
{
    const auto position = positionOf(entity);

    if (const auto last = _dense.back(); position != _dense.size() - 1) {
        _dense[position] = last;
        _sparse[last.index()] = position;
    }

    _dense.pop_back();
    _sparse[entity.index()] = NPOS;
}

void SparseSetBase::append(const Entity &entity)
{
    assert(entity != Entity::NONE);
    assert(entity.index() >= _sparse.size() || _sparse[entity.index()] == NPOS);

    if (entity.index() >= _sparse.size())
        _sparse.resize(entity.index() + 1, NPOS);

    _dense.push_back(entity);
    _sparse[entity.index()] = static_cast<size_type>(_dense.size() - 1);
}
}
