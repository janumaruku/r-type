/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ViewIterator
*/

#ifndef R_TYPE_VIEWITERATOR_HPP
#define R_TYPE_VIEWITERATOR_HPP

#include <cstddef>
#include <iterator>
#include <tuple>
#include <type_traits>
#include <utility>

#include "engine/ecs/Entity.hpp"
#include "engine/ecs/SparseSetBase.hpp"

namespace engine::ecs {
template <class... Containers>
class View;

template <class... Containers>
class ViewIterator {
    template <class Container>
    using ComponentReferenceT = decltype(std::declval<Container &>().get(
        std::declval<const Entity &>()));
    template <class Container>
    using ComponentPointerT = std::remove_reference_t<ComponentReferenceT<
        Container>> *;

public:
    using value_type = std::tuple<ComponentReferenceT<Containers>...>; // NOLINT
    using reference = value_type; // NOLINT
    using pointer = void; // NOLINT
    using difference_type = std::ptrdiff_t; // NOLINT
    using iterator_category = std::input_iterator_tag; // NOLINT
    using iterator_concept = std::forward_iterator_tag; // NOLINT

    using SetsTuple = std::tuple<Containers *...>;

    ViewIterator() = default;

private:
    friend class View<Containers...>;

    ViewIterator(const SetsTuple &sets, const SparseSetBase *driver,
        std::size_t remaining);

    SetsTuple _sets;
    const SparseSetBase *_driver;
    std::size_t _remaining;
    std::tuple<ComponentPointerT<Containers>...> _current;
    static constexpr std::index_sequence_for<Containers...> _sequence{}; // NOLINT
};
} // ecs
// engine

#endif //R_TYPE_VIEWITERATOR_HPP
