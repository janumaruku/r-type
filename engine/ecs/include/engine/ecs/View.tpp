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

namespace engine::ecs::detail {
template <bool WithEntity, ViewComponentConcept... Components>
BasicView<WithEntity, Components...>::BasicView(
    StorageFor<Components> &... containers):
    BasicView(Bases{std::addressof(containers)...}, containers...)
{
}

template <bool WithEntity, ViewComponentConcept... Components>
BasicView<WithEntity, Components...>::iterator
    BasicView<WithEntity, Components...>::begin() const
{
    return iterator(_sets, _driver, _driverIndex, static_cast<std::size_t>(-1));
}

template <bool WithEntity, ViewComponentConcept... Components>
BasicView<WithEntity, Components...>::iterator
    BasicView<WithEntity, Components...>::end() const
{
    return iterator(_sets, _driver, _driverIndex, 0);
}

template <bool WithEntity, ViewComponentConcept... Components>
BasicView<WithEntity, Components...>::BasicView(const Bases &bases,
    StorageFor<Components> &... containers):
    _sets(std::addressof(containers)...), _driverIndex(smallest(bases)),
    _driver(bases[_driverIndex])
{
}

template <bool WithEntity, ViewComponentConcept... Components>
std::size_t BasicView<WithEntity, Components...>::smallest(const Bases &bases)
{
    return static_cast<std::size_t>(std::ranges::min_element(bases, {},
        &SparseSetBase::size) - bases.begin());
}
}

#endif
