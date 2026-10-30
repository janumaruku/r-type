/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Motion
*/

#include "rtype/game/Motion.hpp"

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
}
