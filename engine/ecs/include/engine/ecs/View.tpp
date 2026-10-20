/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** View
*/

#ifndef R_TYPE_VIEW_TPP
#define R_TYPE_VIEW_TPP

#include <algorithm>
#include <array>
#include <memory>

#include "engine/ecs/View.hpp"

namespace engine::ecs {
template <class... Components>
View<Components...>::View(detail::StorageFor<Components> &... containers):
    _sets(std::addressof(containers)...), _driver(smallest(containers...))
{
}

template <class... Components>
View<Components...>::iterator View<Components...>::begin() const
{
    return iterator(_sets, _driver, static_cast<std::size_t>(-1));
}

template <class... Components>
View<Components...>::iterator View<Components...>::end() const
{
    return iterator(_sets, _driver, 0);
}

template <class... Components>
const SparseSetBase *View<Components...>::smallest(
    detail::StorageFor<Components> &... containers)
{
    const std::array<const SparseSetBase *, sizeof...(Components)> sets{
        std::addressof(containers)...};

    return *std::ranges::min_element(sets, {}, &SparseSetBase::size);
}
}

#endif
