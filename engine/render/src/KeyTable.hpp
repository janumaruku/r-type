/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** KeyTable.hpp
*/

#ifndef KEY_TABLE_HPP
#define KEY_TABLE_HPP

#include <array>
#include <cstddef>

#include <raylib.h>

#include "engine/input/Key.hpp"

namespace engine::render::detail {

constexpr std::size_t keyIndex(input::Key key) noexcept
{
    return static_cast<std::size_t>(key);
}

/*
** Raylib's KEY_* code for each engine Key, indexed by the Key's value.
** UNKNOWN stays KEY_NULL; every other key must have its own code, which
** engine_render_raylib_tests checks.
*/
constexpr std::array<int, input::KEY_COUNT> makeKeyTable() noexcept
{
    using input::Key;
    std::array<int, input::KEY_COUNT> table{};

    table[keyIndex(Key::KEY_A)] = KEY_A;
    table[keyIndex(Key::KEY_B)] = KEY_B;
    table[keyIndex(Key::KEY_C)] = KEY_C;
    table[keyIndex(Key::KEY_D)] = KEY_D;
    table[keyIndex(Key::KEY_E)] = KEY_E;
    table[keyIndex(Key::KEY_F)] = KEY_F;
    table[keyIndex(Key::KEY_G)] = KEY_G;
    table[keyIndex(Key::KEY_H)] = KEY_H;
    table[keyIndex(Key::KEY_I)] = KEY_I;
    table[keyIndex(Key::KEY_J)] = KEY_J;
    table[keyIndex(Key::KEY_K)] = KEY_K;
    table[keyIndex(Key::KEY_L)] = KEY_L;
    table[keyIndex(Key::KEY_M)] = KEY_M;
    table[keyIndex(Key::KEY_N)] = KEY_N;
    table[keyIndex(Key::KEY_O)] = KEY_O;
    table[keyIndex(Key::KEY_P)] = KEY_P;
    table[keyIndex(Key::KEY_Q)] = KEY_Q;
    table[keyIndex(Key::KEY_R)] = KEY_R;
    table[keyIndex(Key::KEY_S)] = KEY_S;
    table[keyIndex(Key::KEY_T)] = KEY_T;
    table[keyIndex(Key::KEY_U)] = KEY_U;
    table[keyIndex(Key::KEY_V)] = KEY_V;
    table[keyIndex(Key::KEY_W)] = KEY_W;
    table[keyIndex(Key::KEY_X)] = KEY_X;
    table[keyIndex(Key::KEY_Y)] = KEY_Y;
    table[keyIndex(Key::KEY_Z)] = KEY_Z;
    table[keyIndex(Key::NUM0)] = KEY_ZERO;
    table[keyIndex(Key::NUM1)] = KEY_ONE;
    table[keyIndex(Key::NUM2)] = KEY_TWO;
    table[keyIndex(Key::NUM3)] = KEY_THREE;
    table[keyIndex(Key::NUM4)] = KEY_FOUR;
    table[keyIndex(Key::NUM5)] = KEY_FIVE;
    table[keyIndex(Key::NUM6)] = KEY_SIX;
    table[keyIndex(Key::NUM7)] = KEY_SEVEN;
    table[keyIndex(Key::NUM8)] = KEY_EIGHT;
    table[keyIndex(Key::NUM9)] = KEY_NINE;
    table[keyIndex(Key::UP)] = KEY_UP;
    table[keyIndex(Key::DOWN)] = KEY_DOWN;
    table[keyIndex(Key::LEFT)] = KEY_LEFT;
    table[keyIndex(Key::RIGHT)] = KEY_RIGHT;
    table[keyIndex(Key::SPACE)] = KEY_SPACE;
    table[keyIndex(Key::ENTER)] = KEY_ENTER;
    table[keyIndex(Key::ESCAPE)] = KEY_ESCAPE;
    table[keyIndex(Key::LSHIFT)] = KEY_LEFT_SHIFT;
    table[keyIndex(Key::LCTRL)] = KEY_LEFT_CONTROL;
    return table;
}

inline constexpr std::array<int, input::KEY_COUNT> KEY_TABLE = makeKeyTable();

}

#endif /* !KEY_TABLE_HPP */
