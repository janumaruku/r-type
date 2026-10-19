/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputEvent.hpp
*/

#pragma once

#include "Vec2.hpp"
#include "engine/input/Key.hpp"

namespace engine::input {

    struct InputEvent {
        enum class Type {
            KEY_PRESSED,
            KEY_RELEASED,
            WINDOW_CLOSED,
            FOCUS_LOST,
            FOCUS_GAINED,
            RESIZED
        };

        Type type{Type::KEY_PRESSED};
        Key key{Key::UNKNOWN};
        core::Vec2u size{0, 0};
    };

}
