/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** IndexedView
*/

#ifndef R_TYPE_INDEXEDVIEW_HPP
#define R_TYPE_INDEXEDVIEW_HPP

#include "engine/ecs/View.hpp"

namespace engine::ecs {
template <ViewComponentConcept... Components>
using IndexedView = detail::BasicView<true, Components...>;
}

#endif // R_TYPE_INDEXEDVIEW_HPP
