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
template <ViewComponentConcept... Components>
View<Components...>::View(detail::StorageFor<Components> &... containers):
    View(Bases{std::addressof(containers)...}, containers...)
{
}

template <ViewComponentConcept... Components>
View<Components...>::iterator View<Components...>::begin() const
{
    return iterator(_sets, _driver, _driverIndex, static_cast<std::size_t>(-1));
}

template <ViewComponentConcept... Components>
View<Components...>::iterator View<Components...>::end() const
{
    return iterator(_sets, _driver, _driverIndex, 0);
}

template <ViewComponentConcept ... Components>
View<Components...>::View(const Bases &bases,
    detail::StorageFor<Components> &... containers):
    _sets(std::addressof(containers)...), _driverIndex(smallest(bases)),
    _driver(bases[_driverIndex])
{
}

template <ViewComponentConcept ... Components>
std::size_t View<Components...>::smallest(const Bases &bases)
{
    return static_cast<std::size_t>(std::ranges::min_element(bases, {},
        &SparseSetBase::size) - bases.begin());
}
}

#endif
