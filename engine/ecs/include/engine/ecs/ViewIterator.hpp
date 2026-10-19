/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** ViewIterator
*/

#ifndef R_TYPE_VIEWITERATOR_HPP
#define R_TYPE_VIEWITERATOR_HPP
#include <iterator>
#include <utility>

namespace engine::ecs {
template <class... Containers>
class View;

template <class... Containers>
class ViewIterator {
    template <class Container>
    using ComponentReferenceT = decltype(std::declval<Container &>().get(0));
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

private:
    friend class View<Containers...>;

    ViewIterator(const SetsTuple &sets, std::size_t driver,
        std::size_t remaining);
};
} // ecs
// engine

#endif //R_TYPE_VIEWITERATOR_HPP
