/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Actions
*/

#include "Actions.hpp"

#include <algorithm>
#include <array>
#include <cstddef>

namespace render_demo {
namespace {
using engine::input::Key;

constexpr std::array<std::string_view, 10> NAMES{"MOVE_UP", "MOVE_DOWN",
    "MOVE_LEFT", "MOVE_RIGHT", "FIRE", "QUIT", "EXPLODE", "VOLUME_UP",
    "VOLUME_DOWN", "MUTE"};
}

std::string_view toString(const Action action)
{
    return NAMES[static_cast<std::size_t>(action)];
}

bool fromString(const std::string_view name, Action &action)
{
    const auto it = std::ranges::find(NAMES, name);

    if (it == NAMES.end())
        return false;
    action = static_cast<Action>(it - NAMES.begin());
    return true;
}

engine::input::ActionMap<Action> defaultBindings()
{
    engine::input::ActionMap<Action> bindings;

    bindings.bind(Action::MOVE_UP, Key::UP);
    bindings.bind(Action::MOVE_UP, Key::KEY_W);
    bindings.bind(Action::MOVE_DOWN, Key::DOWN);
    bindings.bind(Action::MOVE_DOWN, Key::KEY_S);
    bindings.bind(Action::MOVE_LEFT, Key::LEFT);
    bindings.bind(Action::MOVE_LEFT, Key::KEY_A);
    bindings.bind(Action::MOVE_RIGHT, Key::RIGHT);
    bindings.bind(Action::MOVE_RIGHT, Key::KEY_D);
    bindings.bind(Action::FIRE, Key::SPACE);
    bindings.bind(Action::FIRE, Key::LCTRL);
    bindings.bind(Action::QUIT, Key::ESCAPE);
    bindings.bind(Action::EXPLODE, Key::KEY_X);
    bindings.bind(Action::VOLUME_UP, Key::NUM0);
    bindings.bind(Action::VOLUME_DOWN, Key::NUM9);
    bindings.bind(Action::MUTE, Key::KEY_M);
    return bindings;
}

}
