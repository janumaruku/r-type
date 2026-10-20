/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Component
*/

#ifndef COMPONENT_HPP
#define COMPONENT_HPP

#include "engine/ecs/TypeId.hpp"
#include <concepts>

namespace engine::ecs {
template <class T>
concept ComponentConcept = detail::PlainType<T> && std::is_object_v<T> &&
    std::move_constructible<T> && std::assignable_from<T &, T> &&
        std::destructible<T>;
}

#endif
