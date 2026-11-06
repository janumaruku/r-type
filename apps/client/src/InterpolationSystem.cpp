/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InterpolationSystem.cpp
*/

#include "InterpolationSystem.hpp"

#include <algorithm>
#include <optional>

#include "ClientStages.hpp"

namespace rtype::client {
namespace {
constexpr float TICK_RATE = 60.0F;

[[nodiscard]] bool isFilled(const NetSample &sample) noexcept
{
    return sample.tick != 0;
}

[[nodiscard]] engine::core::Vec2f lerp(const NetSample &from,
    const NetSample &to, float renderTick) noexcept
{
    const auto span = static_cast<float>(to.tick - from.tick);
    const float alpha = (renderTick - static_cast<float>(from.tick)) / span;

    return from.position + (to.position - from.position) * alpha;
}

[[nodiscard]] engine::core::Vec2f extrapolate(const NetSample &from,
    float renderTick) noexcept
{
    const float ticks = std::min(renderTick - static_cast<float>(from.tick),
        MAX_EXTRAPOLATION_TICKS);

    return from.position + from.velocity * (ticks / TICK_RATE);
}

[[nodiscard]] std::optional<engine::core::Vec2f> positionAt(
    const NetSamples &net, float renderTick) noexcept
{
    const NetSample *before = nullptr;
    const NetSample *after = nullptr;
    std::size_t filled = 0;

    for (const auto &sample: net.ring) {
        if (!isFilled(sample))
            continue;
        ++filled;
        if (static_cast<float>(sample.tick) <= renderTick) {
            if (before == nullptr || sample.tick > before->tick)
                before = &sample;
        } else if (after == nullptr || sample.tick < after->tick) {
            after = &sample;
        }
    }
    if (before != nullptr && after != nullptr)
        return lerp(*before, *after, renderTick);
    if (before != nullptr && filled == 1)
        return before->position;
    if (before != nullptr)
        return extrapolate(*before, renderTick);
    if (after != nullptr)
        return after->position;
    return std::nullopt;
}
}


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

void interpolationSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<game::Transform> &transforms,
    const engine::ecs::SparseSet<NetSamples> &samples)
{
    const float renderTick = registry.getResource<RenderClock>().renderTick();
    const auto &locals = registry.registerComponent<LocalPlayer>();

    for (const auto &entity: samples.entities()) {
        auto *transform = transforms.tryGet(entity);

        if (transform == nullptr || locals.contains(entity))
            continue;
        if (const auto position = positionAt(samples.get(entity), renderTick))
            transform->position = *position;
    }
}

std::uint8_t sampleButtons(const engine::input::ActionMap<Action> &actions,
    const engine::input::InputState &input) noexcept
{
    std::uint8_t buttons = 0;

    if (actions.isActive(Action::MOVE_UP, input))
        buttons |= (1 << static_cast<std::uint8_t>(Action::MOVE_UP));
    if (actions.isActive(Action::MOVE_DOWN, input))
        buttons |= (1 << static_cast<std::uint8_t>(Action::MOVE_DOWN));
    if (actions.isActive(Action::MOVE_LEFT, input))
        buttons |= (1 << static_cast<std::uint8_t>(Action::MOVE_LEFT));
    if (actions.isActive(Action::MOVE_RIGHT, input))
        buttons |= (1 << static_cast<std::uint8_t>(Action::MOVE_RIGHT));
    if (actions.isActive(Action::FIRE, input))
        buttons |= (1 << static_cast<std::uint8_t>(Action::FIRE));

    return buttons;
}
} // namespace rtype::client
