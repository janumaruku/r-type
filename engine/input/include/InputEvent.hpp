/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** InputEvent.hpp
*/

#pragma once

#include "../../core/include/Vec2.hpp"
#include "Key.hpp"
#include <cstdint>

namespace engine::input {

    struct Vec2u {
        std::uint32_t x{0};
        std::uint32_t y{0};
    };

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
        Vec2u size{0, 0};
    };

}
