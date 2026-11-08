/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight input actions
*/

#ifndef HOLLOW_KNIGHT_ACTIONS_HPP
#define HOLLOW_KNIGHT_ACTIONS_HPP

#include <cstdint>
#include <iosfwd>
#include <string_view>

#include "engine/input/ActionMap.hpp"

namespace hollow_knight {
enum class Action : std::uint8_t {
    MOVE_LEFT,
    MOVE_RIGHT,
    MENU_UP,
    MENU_DOWN,
    JUMP,
    ATTACK,
    DASH,
    FOCUS,
    INTERACT,
    CONFIRM,
    PAUSE,
};

std::ostream &operator<<(std::ostream &stream, Action action);

[[nodiscard]] std::string_view toString(Action action) noexcept;

[[nodiscard]] bool fromString(std::string_view name, Action &action) noexcept;

void bindDefaults(engine::input::ActionMap<Action> &actions);
} // namespace hollow_knight

#endif // HOLLOW_KNIGHT_ACTIONS_HPP
