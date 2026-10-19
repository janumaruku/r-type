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

// typeId<T>() gives each type a small integer, assigned the first time the
// type is asked for and unchanged for the rest of the run. The registry uses
// it as a vector index to find a type's storage.
//
// Ids depend on the order in which types are first used, so they differ
// between runs and between the client and the server. Never send one over the
// network or save one to a file.
//
// The registry's vectors are sized by the number of distinct types used in the
// program, not by the value of any one id.
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
