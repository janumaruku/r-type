/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** View
*/

#ifndef R_TYPE_VIEW_HPP
#define R_TYPE_VIEW_HPP

#include <tuple>

#include "engine/ecs/SparseSetBase.hpp"
#include "engine/ecs/ViewIterator.hpp"

namespace engine::ecs {
template <class... Containers>
class View {
public:
    using iterator = ViewIterator<Containers...>; // NOLINT

    explicit View(Containers &... containers);

    iterator begin() const;

    iterator end() const;

private:
    static const SparseSetBase *smallest(Containers &... containers);

    std::tuple<Containers *...> _sets;
    const SparseSetBase *_driver;
};
} // ecs
// engine

#endif //R_TYPE_VIEW_HPP

#include "engine/ecs/View.tpp"
