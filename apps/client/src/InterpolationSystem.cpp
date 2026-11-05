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

    }



} // namespace rtype::client
