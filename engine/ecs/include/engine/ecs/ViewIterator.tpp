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
template <class... Containers>
ViewIterator<Containers...>::ViewIterator(const SetsTuple &sets,
    const SparseSetBase *driver, std::size_t remaining):
    _sets(sets), _driver(driver),
    _remaining(std::min(remaining, driver->size()))
{
    skipIncomplete();
}

template <class... Containers>
typename ViewIterator<Containers...>::reference
ViewIterator<Containers...>::operator*() const
{
    assert(_remaining > 0);

    return toReference(_sequence);
}

template <class... Containers>
ViewIterator<Containers...> &ViewIterator<Containers...>::operator++()
{
    assert(_remaining > 0);

    _remaining = std::min(_remaining - 1, _driver->size());
    skipIncomplete();
    return *this;
}

template <class... Containers>
ViewIterator<Containers...> ViewIterator<Containers...>::operator++(int)
{
    ViewIterator previous = *this;

    ++*this;
    return previous;
}

template <class... Containers>
bool ViewIterator<Containers...>::operator==(
    const ViewIterator &other) const noexcept
{
    return _remaining == other._remaining;
}

template <class... Containers>
void ViewIterator<Containers...>::skipIncomplete()
{
    while (_remaining > 0) {
        const std::size_t position = _remaining - 1;

        if (fetchAll(_driver->entities()[position], position, _sequence))
            return;
        _remaining = position;
    }
}

template <class... Containers>
template <std::size_t... Is>
bool ViewIterator<Containers...>::fetchAll(const Entity &entity,
    std::size_t position, std::index_sequence<Is...>)
{
    return (((std::get<Is>(_current) =
        static_cast<const SparseSetBase *>(std::get<Is>(_sets)) == _driver
            ? &std::get<Is>(_sets)->components()[position]
            : std::get<Is>(_sets)->tryGet(entity)) != nullptr) && ...);
}

template <class... Containers>
template <std::size_t... Is>
ViewIterator<Containers...>::reference
ViewIterator<Containers...>::toReference(std::index_sequence<Is...>) const
{
    return reference(*std::get<Is>(_current)...);
}
}

#endif
