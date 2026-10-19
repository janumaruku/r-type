/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** View
*/

#ifndef R_TYPE_VIEW_TPP
#define R_TYPE_VIEW_TPP

#include <algorithm>

#include "engine/ecs/View.hpp"

namespace engine::ecs {
template <class... Containers>
View<Containers...>::View(Containers &... containers):
    _sets(std::addressof(containers)...), _driver(smallest(containers...))
{
}

template <class... Containers>
View<Containers...>::iterator View<Containers...>::begin() const
{
    return iterator(_sets, _driver, static_cast<std::size_t>(-1));
}

template <class... Containers>
View<Containers...>::iterator View<Containers...>::end() const
{
    return iterator(_sets, _driver, 0);
}

template <class... Containers>
std::size_t View<Containers...>::smallest(Containers &... containers)
{
    std::array<std::size_t, sizeof...(Containers)> sizes(containers.size()...);

    return static_cast<std::size_t>(std::ranges::min_element(sizes) - sizes.
        begin());
}
}

#endif
