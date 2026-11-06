/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InterpolationSystem.hpp
*/

#ifndef R_TYPE_INTERPOLATIONSYSTEM_HPP
#define R_TYPE_INTERPOLATIONSYSTEM_HPP

#include <cstdint>

#include "Actions.hpp"
#include "ClientComponents.hpp"
#include "engine/ecs/SparseSet.hpp"
#include "rtype/game/Components.hpp"
#include "engine/input/ActionMap.hpp"

namespace rtype::client {
void interpolationSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<game::Transform> &transforms,
    const engine::ecs::SparseSet<NetSamples> &samples);

[[nodiscard]] std::uint8_t sampleButtons(
    const engine::input::ActionMap<Action> &actions,
    const engine::input::InputState &input) noexcept;
}
#endif //R_TYPE_INTERPOLATIONSYSTEM_HPP
