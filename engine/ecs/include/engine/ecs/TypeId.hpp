/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** typeId
*/

#ifndef R_TYPE_TYPEID_HPP
#define R_TYPE_TYPEID_HPP
#include <cstddef>
#include <type_traits>

namespace engine::ecs::detail {
template <class T>
concept PlainType = std::is_same_v<T, std::remove_cv_t<std::remove_reference_t<
    T>>>;

std::size_t nextTypeId() noexcept;

template <class T>
    requires PlainType<T>
std::size_t typeId() noexcept
{
    static const std::size_t id = nextTypeId();

    return id;
}
}

#endif //R_TYPE_TYPEID_HPP
