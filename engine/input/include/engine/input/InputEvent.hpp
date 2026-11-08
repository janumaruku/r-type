/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputEvent.hpp
*/

#pragma once

#include "engine/core/Vec2.hpp"
#include "engine/input/Key.hpp"
#include "engine/input/MouseButton.hpp"

namespace engine::input {
struct InputEvent {
    enum class Type {
        KEY_PRESSED,
        KEY_RELEASED,
        MOUSE_BUTTON_PRESSED,
        MOUSE_BUTTON_RELEASED,
        WINDOW_CLOSED,
        FOCUS_LOST,
        FOCUS_GAINED,
        RESIZED
    };

    Type type{Type::KEY_PRESSED};
    Key key{Key::UNKNOWN};
    MouseButton mouseButton{MouseButton::LEFT};
    core::Vec2u size{0, 0};
};
} // namespace engine::input