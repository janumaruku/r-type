/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** View
*/

#ifndef R_TYPE_VIEW_HPP
#define R_TYPE_VIEW_HPP

#include <array>
#include <type_traits>

#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/ViewIterator.hpp"

namespace engine::ecs {
namespace detail {
template <class... Components>
constexpr bool ALL_DIFFERENT = true;

template <class First, class... Rest>
constexpr bool ALL_DIFFERENT<First, Rest...> =
    (!std::is_same_v<First, Rest> && ...) && ALL_DIFFERENT<Rest...>;

template <bool WithEntity, ViewComponentConcept... Components>
class BasicView {
    static_assert(sizeof...(Components) > 0,
        "a view needs at least one component");
    static_assert(detail::ALL_DIFFERENT<std::remove_const_t<Components>...>,
        "a view can't list the same component twice");

public:
    using iterator = ViewIterator<WithEntity, Components...>; // NOLINT

    explicit BasicView(StorageFor<Components> &... containers);

    iterator begin() const;

    iterator end() const;

private:
    using Bases = std::array<const SparseSetBase *, sizeof...(Components)>;

    explicit BasicView(const Bases &bases,
        StorageFor<Components> &...containers);

    static std::size_t smallest(const Bases &bases);

    iterator::SetsTuple _sets;
    std::size_t _driverIndex;
    const SparseSetBase *_driver;
};
}

template <ViewComponentConcept... Components>
using View = detail::BasicView<false, Components...>;
} // ecs
// engine

#endif //R_TYPE_VIEW_HPP

#include "engine/ecs/View.tpp"
