/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Actions
*/

#ifndef RENDER_DEMO_ACTIONS_HPP
#define RENDER_DEMO_ACTIONS_HPP

#include <cstdint>
#include <string_view>

#include "engine/input/ActionMap.hpp"

namespace render_demo {

enum class Action: std::uint8_t {
    MOVE_UP,
    MOVE_DOWN,
    MOVE_LEFT,
    MOVE_RIGHT,
    FIRE,
    QUIT,
};

/*
** Found by ActionMap through argument-dependent lookup; the names are the
** ones used in keybindings.json.
*/
[[nodiscard]] std::string_view toString(Action action);

[[nodiscard]] bool fromString(std::string_view name, Action &action);

/*
** Arrows and WASD to move, SPACE and LCTRL to fire, ESCAPE to quit:
** keybindings.json can replace any of them.
*/
[[nodiscard]] engine::input::ActionMap<Action> defaultBindings();

}

#endif // RENDER_DEMO_ACTIONS_HPP
