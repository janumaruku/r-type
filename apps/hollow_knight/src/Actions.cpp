/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight input actions
*/

#include "Actions.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <ostream>

#include "engine/input/Key.hpp"

namespace hollow_knight {
namespace {
constexpr std::array ACTIONS{
    Action::MOVE_LEFT,
    Action::MOVE_RIGHT,
    Action::MENU_UP,
    Action::MENU_DOWN,
    Action::JUMP,
    Action::ATTACK,
    Action::DASH,
    Action::FOCUS,
    Action::INTERACT,
    Action::CONFIRM,
    Action::PAUSE,
};

bool sameLetter(const char left, const char right) noexcept
{
    return std::toupper(static_cast<unsigned char>(left)) ==
        std::toupper(static_cast<unsigned char>(right));
}
} // namespace

std::ostream &operator<<(std::ostream &stream, const Action action)
{
    return stream << toString(action);
}

std::string_view toString(const Action action) noexcept
{
    switch (action) {
    case Action::MOVE_LEFT:
        return "MOVE_LEFT";
    case Action::MOVE_RIGHT:
        return "MOVE_RIGHT";
    case Action::MENU_UP:
        return "MENU_UP";
    case Action::MENU_DOWN:
        return "MENU_DOWN";
    case Action::JUMP:
        return "JUMP";
    case Action::ATTACK:
        return "ATTACK";
    case Action::DASH:
        return "DASH";
    case Action::FOCUS:
        return "FOCUS";
    case Action::INTERACT:
        return "INTERACT";
    case Action::CONFIRM:
        return "CONFIRM";
    case Action::PAUSE:
        return "PAUSE";
    }
    return "UNKNOWN";
}

bool fromString(const std::string_view name, Action &action) noexcept
{
    for (const Action candidate: ACTIONS) {
        if (std::ranges::equal(toString(candidate), name, sameLetter)) {
            action = candidate;
            return true;
        }
    }
    return false;
}

void bindDefaults(engine::input::ActionMap<Action> &actions)
{
    using engine::input::Key;

    actions.bind(Action::MOVE_LEFT, Key::LEFT);
    actions.bind(Action::MOVE_LEFT, Key::KEY_A);
    actions.bind(Action::MOVE_RIGHT, Key::RIGHT);
    actions.bind(Action::MOVE_RIGHT, Key::KEY_D);
    actions.bind(Action::MENU_UP, Key::UP);
    actions.bind(Action::MENU_UP, Key::KEY_W);
    actions.bind(Action::MENU_DOWN, Key::DOWN);
    actions.bind(Action::MENU_DOWN, Key::KEY_S);
    actions.bind(Action::JUMP, Key::SPACE);
    actions.bind(Action::ATTACK, Key::KEY_J);
    actions.bind(Action::DASH, Key::LSHIFT);
    actions.bind(Action::FOCUS, Key::KEY_F);
    actions.bind(Action::INTERACT, Key::KEY_E);
    actions.bind(Action::CONFIRM, Key::ENTER);
    actions.bind(Action::PAUSE, Key::ESCAPE);
}
} // namespace hollow_knight
