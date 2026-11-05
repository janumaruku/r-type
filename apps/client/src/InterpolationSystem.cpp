/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InterpolationSystem.cpp
*/

#include "InterpolationSystem.hpp"

namespace rtype::client
{

    float RenderClock::renderTick() const noexcept
    {
        float time = 0.0F;
        float total = 0.0F;

        if (latestServerTick > INTERPOLATION_DELAY_TICKS)
            time = static_cast<float>(latestServerTick - INTERPOLATION_DELAY_TICKS);
        total = time + sinceLatestTick * 60.0F;
        if (total < 0.0F)
            return 0.0F;
        return total;
    }

    void RenderClock::interpolationSystem(engine::ecs::Registry &registry,
        engine::ecs::SparseSet<game::Transform> &transforms,
        const engine::ecs::SparseSet<NetSamples> &samples)
    {
        
    }

    std::uint8_t RenderClock::sampleButtons(const engine::input::ActionMap<Action> &actions,
        const engine::input::InputState &input) noexcept
    {
        std::uint8_t buttons = 0;

        if (actions.isActive(Action::MOVE_UP, input))
            buttons |= (1 << 0);

        if (actions.isActive(Action::MOVE_DOWN, input))
            buttons |= (1 << 1);
        if (actions.isActive(Action::MOVE_LEFT, input))
            buttons |= (1 << 2);
        if (actions.isActive(Action::MOVE_RIGHT, input))
            buttons |= (1 << 3);
        if (actions.isActive(Action::FIRE, input))
            buttons |= (1 << 4);
        return buttons;
    }

// "MOVE_UP"
// "MOVE_DOWN"
// "MOVE_LEFT"
// "MOVE_RIGHT"
// "FIRE"
// "CONFIRM"
// "QUIT"

} // namespace rtype::client
