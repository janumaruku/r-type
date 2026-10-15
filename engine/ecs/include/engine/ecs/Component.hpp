/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Component
*/

#ifndef COMPONENT_HPP_
#define COMPONENT_HPP_

#include "TypeId.hpp"
#include <type_traits>

namespace engine::ecs {
template <class T>
concept Component = detail::PlainType<T> && std::is_object_v<T> &&
    std::move_constructible<T> std::assignable_from<T &, T> &&
        std::destructible<T>;
}

#endif
