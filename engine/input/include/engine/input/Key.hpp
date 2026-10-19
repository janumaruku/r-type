/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Key.hpp
*/

#pragma once

#include <cstdint>
#include <cstddef>
#include <optional>
#include <string_view>

namespace engine::input {

    enum class Key:std::uint8_t {
        UNKNOWN,
        KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
        KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
        KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
        NUM0, NUM1, NUM2, NUM3, NUM4, NUM5, NUM6, NUM7, NUM8, NUM9,
        UP, DOWN, LEFT, RIGHT, SPACE, ENTER, ESCAPE, LSHIFT, LCTRL,
        COUNT
    };

    constexpr std::size_t KEY_COUNT = static_cast<std::size_t>(Key::COUNT);
    constexpr std::size_t KEY_MAX_SIZE = 8;

    std::string_view toString(Key key) noexcept;
    std::optional<Key> keyFromString(std::string_view name) noexcept;
}
