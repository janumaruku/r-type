//
// Created by janumaruku on 10/19/26.
//

#ifndef R_TYPE_VIEW_HPP
#define R_TYPE_VIEW_HPP

#include "engine/ecs/ViewIterator.hpp"

namespace engine::ecs {
template <class... Containers>
class View {
public:
    using iterator = ViewIterator<Containers...>; // NOLINT
};
} // ecs
// engine

#endif //R_TYPE_VIEW_HPP
