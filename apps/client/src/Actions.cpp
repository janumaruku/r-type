/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Action
*/

#include "Actions.hpp"

#include <algorithm>
#include <array>
#include <cctype>

namespace rtype::client {
namespace {
constexpr std::array ACTIONS{
    Action::MOVE_UP,
    Action::MOVE_DOWN,
    Action::MOVE_LEFT,
    Action::MOVE_RIGHT,
    Action::FIRE,
    Action::TOGGLE_HUD_HELP,
    Action::QUIT,
    Action::CONFIRM,
};

[[nodiscard]] bool sameLetter(char left, char right) noexcept
{
    return std::toupper(static_cast<unsigned char>(left)) ==
        std::toupper(static_cast<unsigned char>(right));
}
} // namespace

std::ostream &operator<<(std::ostream &stream, const Action &action)
{
    return stream << toString(action);
}

std::string_view toString(Action action) noexcept
{
    switch (action) {
    case Action::MOVE_UP:
        return "MOVE_UP";
    case Action::MOVE_DOWN:
        return "MOVE_DOWN";
    case Action::MOVE_LEFT:
        return "MOVE_LEFT";
    case Action::MOVE_RIGHT:
        return "MOVE_RIGHT";
    case Action::FIRE:
        return "FIRE";
    case Action::TOGGLE_HUD_HELP:
        return "TOGGLE_HUD_HELP";
    case Action::QUIT:
        return "QUIT";
    case Action::CONFIRM:
        return "CONFIRM";
    }

    return "UNKNOWN";
}

bool fromString(std::string_view name, Action &action) noexcept
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
    actions.bind(Action::MOVE_UP, engine::input::Key::UP);
    actions.bind(Action::MOVE_UP, engine::input::Key::KEY_W);
    actions.bind(Action::MOVE_DOWN, engine::input::Key::DOWN);
    actions.bind(Action::MOVE_DOWN, engine::input::Key::KEY_S);
    actions.bind(Action::MOVE_LEFT, engine::input::Key::LEFT);
    actions.bind(Action::MOVE_LEFT, engine::input::Key::KEY_A);
    actions.bind(Action::MOVE_RIGHT, engine::input::Key::RIGHT);
    actions.bind(Action::MOVE_RIGHT, engine::input::Key::KEY_D);
    actions.bind(Action::FIRE, engine::input::Key::SPACE);
    actions.bind(Action::FIRE, engine::input::Key::LCTRL);
    actions.bind(Action::TOGGLE_HUD_HELP, engine::input::Key::F1);
    actions.bind(Action::QUIT, engine::input::Key::ESCAPE);
    actions.bind(Action::CONFIRM, engine::input::Key::ENTER);
}
} // namespace rtype::client
