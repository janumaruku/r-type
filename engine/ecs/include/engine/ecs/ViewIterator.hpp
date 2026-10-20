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
#include <type_traits>
#include <tuple>
#include <utility>

#include "engine/ecs/Component.hpp"
#include "engine/ecs/Entity.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "engine/ecs/SparseSetBase.hpp"

namespace engine::ecs {
template <class Component>
concept ViewComponentConcept = ComponentConcept<std::remove_const_t<Component>>;

template <ViewComponentConcept... Components>
class View;

namespace detail {
template <class Component>
struct StorageForT {
    using Type = SparseSet<Component>;
};

template <class Component>
struct StorageForT<const Component> {
    using Type = const SparseSet<Component>;
};

template <class Component>
using StorageFor = StorageForT<Component>::Type;
}

template <ViewComponentConcept... Components>
class ViewIterator {
public:
    using value_type = std::tuple<Components &...>; // NOLINT
    using reference = value_type; // NOLINT
    using pointer = void; // NOLINT
    using difference_type = std::ptrdiff_t; // NOLINT
    using iterator_category = std::input_iterator_tag; // NOLINT
    using iterator_concept = std::forward_iterator_tag; // NOLINT

    using SetsTuple = std::tuple<detail::StorageFor<Components> *...>;

    ViewIterator() = default;

    reference operator*() const;

    ViewIterator &operator++();

    ViewIterator operator++(int);

    bool operator==(const ViewIterator &other) const noexcept;

private:
    friend class View<Components...>;

    ViewIterator(const SetsTuple &sets, const SparseSetBase *driver,
        std::size_t remaining);

    void skipIncomplete();

    template <std::size_t... Is>
    bool fetchAll(const Entity &entity, std::size_t position,
        std::index_sequence<Is...>);

    template <std::size_t... Is>
    reference toReference(std::index_sequence<Is...>) const;

    SetsTuple _sets{};
    const SparseSetBase *_driver = nullptr;
    std::size_t _remaining = 0;
    std::tuple<Components *...> _current{};
    static constexpr std::index_sequence_for<Components...> _sequence{}; // NOLINT
};
} // ecs
// engine

#endif //R_TYPE_VIEWITERATOR_HPP

#include "engine/ecs/ViewIterator.tpp"
