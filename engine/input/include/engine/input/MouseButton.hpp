/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** MouseButton.hpp
*/

#pragma once

#include <cstddef>
#include <cstdint>

namespace engine::input {

enum class MouseButton : std::uint8_t {
    LEFT,
    RIGHT,
    MIDDLE,
    SIDE,
    EXTRA,
    FORWARD,
    BACK,
    COUNT,
};

constexpr std::size_t MOUSE_BUTTON_COUNT =
    static_cast<std::size_t>(MouseButton::COUNT);

} // namespace engine::input
