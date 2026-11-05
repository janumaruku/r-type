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

namespace rtype::client
{

    struct RenderClock {
        std::uint32_t latestServerTick = 0;
        float sinceLatestTick = 0.0F;
        [[nodiscard]] float renderTick() const noexcept;
    };

    inline constexpr std::uint32_t INTERPOLATION_DELAY_TICKS = 6;
    inline constexpr float MAX_EXTRAPOLATION_TICKS = 6.0F;

    void interpolationSystem(engine::ecs::Registry &registry,
        engine::ecs::SparseSet<game::Transform> &transforms,
        const engine::ecs::SparseSet<NetSamples> &samples);

    [[nodiscard]] std::uint8_t sampleButtons(const engine::input::ActionMap<Action> &actions,
        const engine::input::InputState &input) noexcept;


}
#endif //R_TYPE_INTERPOLATIONSYSTEM_HPP
