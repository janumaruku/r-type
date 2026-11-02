/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Motion
*/

#ifndef R_TYPE_MOTION_HPP
#define R_TYPE_MOTION_HPP

#include "engine/core/Vec2.hpp"
#include "rtype/game/Rules.hpp"
#include "rtype/game/Types.hpp"

namespace rtype::game {
[[nodiscard]] engine::core::Vec2f inputDirection(InputButtons buttons) noexcept;

[[nodiscard]] engine::core::Vec2f confineToPlayfield(
    engine::core::Vec2f position, engine::core::Vec2f halfSize,
    const Rules &rules) noexcept;

[[nodiscard]] engine::core::Vec2f movePlayer(engine::core::Vec2f position,
    InputButtons buttons, float dt, const Rules &rules);
}

#endif //R_TYPE_MOTION_HPP
