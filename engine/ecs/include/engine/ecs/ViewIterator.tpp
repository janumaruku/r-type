/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ViewIterator
*/

#ifndef R_TYPE_VIEWITERATOR_TPP
#define R_TYPE_VIEWITERATOR_TPP

#include <algorithm>
#include <cassert>

#include "engine/ecs/ViewIterator.hpp"

namespace engine::ecs {
template <bool WithEntity, ViewComponentConcept... Components>
ViewIterator<WithEntity, Components...>::ViewIterator(const SetsTuple &sets,
    const SparseSetBase *driver, std::size_t driverIndex,
    std::size_t remaining):
    _sets(sets), _driver(driver), _driverIndex(driverIndex),
    _remaining(std::min(remaining, driver->size()))
{
    skipIncomplete();
}

template <bool WithEntity, ViewComponentConcept... Components>
ViewIterator<WithEntity, Components...>::reference
    ViewIterator<WithEntity, Components...>::operator*() const
{
    assert(_remaining > 0);

    return toReference(_sequence);
}

template <bool WithEntity, ViewComponentConcept... Components>
ViewIterator<WithEntity, Components...> &
ViewIterator<WithEntity, Components...>::operator++()
{
    assert(_remaining > 0);

    _remaining = std::min(_remaining - 1, _driver->size());
    skipIncomplete();
    return *this;
}

template <bool WithEntity, ViewComponentConcept... Components>
ViewIterator<WithEntity, Components...>
    ViewIterator<WithEntity, Components...>::operator++(int)
{
    ViewIterator previous = *this;

    ++*this;
    return previous;
}

template <bool WithEntity, ViewComponentConcept... Components>
bool ViewIterator<WithEntity, Components...>::operator==(
    const ViewIterator &other) const noexcept
{
    return _remaining == other._remaining;
}

template <bool WithEntity, ViewComponentConcept... Components>
void ViewIterator<WithEntity, Components...>::skipIncomplete()
{
    while (_remaining > 0) {
        const std::size_t position = _remaining - 1;

        if (fetchAll(_driver->entities()[position], position, _sequence))
            return;
        _remaining = position;
    }
}

template <bool WithEntity, ViewComponentConcept... Components>
template <std::size_t... Is>
bool ViewIterator<WithEntity, Components...>::fetchAll(const Entity &entity,
    std::size_t position, std::index_sequence<Is...>)
{
    return (((std::get<Is>(_current) =
        Is == _driverIndex
        ? &std::get<Is>(_sets)->components()[position]
        : std::get<Is>(_sets)->tryGet(entity)) != nullptr) && ...);
}

template <bool WithEntity, ViewComponentConcept... Components>
template <std::size_t... Is>
ViewIterator<WithEntity, Components...>::reference
    ViewIterator<WithEntity, Components...>::toReference(
        std::index_sequence<Is...>) const
{
    if constexpr (WithEntity)
        return reference(_driver->entities()[_remaining - 1],
            *std::get<Is>(_current)...);
    else
        return reference(*std::get<Is>(_current)...);
}
}

#endif
