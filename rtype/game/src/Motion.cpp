/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Motion
*/

#include "rtype/game/Motion.hpp"

#include <algorithm>

namespace rtype::game {

engine::core::Vec2f inputDirection(const InputButtons buttons) noexcept
{
    const auto axis = [&buttons](const std::uint8_t negative,
        const std::uint8_t positive) {
        return (buttons.held(positive) ? 1.0F : 0.0F)
            - (buttons.held(negative) ? 1.0F : 0.0F);
    };
    const engine::core::Vec2f direction{
        axis(InputButtons::LEFT, InputButtons::RIGHT),
        axis(InputButtons::UP, InputButtons::DOWN)
    };

    return direction.normalized();
}

engine::core::Vec2f confineToPlayfield(const engine::core::Vec2f position,
    const engine::core::Vec2f halfSize, const Rules &rules) noexcept
{
    const auto confine = [](const float value, const float half,
        const float size) {
        return std::max(half, std::min(value, size - half));
    };

    return {
        confine(position.x(), halfSize.x(), rules.playfield.x()),
        confine(position.y(), halfSize.y(), rules.playfield.y())
    };
}

engine::core::Vec2f movePlayer(const engine::core::Vec2f position,
    const InputButtons buttons, const float dt, const Rules &rules)
{
    const engine::core::Vec2f step =
        inputDirection(buttons) * rules.playerSpeed * dt;

    return confineToPlayfield(position + step, rules.playerHalfSize, rules);
}
}
