/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** View
*/

#ifndef R_TYPE_VIEW_HPP
#define R_TYPE_VIEW_HPP

#include <tuple>

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
    static std::size_t smallest(Containers &... containers);

    std::tuple<Containers *...> _sets;
    std::size_t _driver;
};
} // ecs
// engine

#endif //R_TYPE_VIEW_HPP
