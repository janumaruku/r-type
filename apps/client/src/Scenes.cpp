/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scenes
*/

#include "Scenes.hpp"

#include <string_view>
#include <unordered_map>

namespace rtype::client {
std::ostream & operator<<(std::ostream &stream, const Action &action)
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
    static const std::unordered_map<std::string_view, Action> conversions = {
        {"MOVE_UP", Action::MOVE_UP},
        {"MOVE_DOWN", Action::MOVE_DOWN},
        {"MOVE_LEFT", Action::MOVE_LEFT},
        {"MOVE_RIGHT", Action::MOVE_RIGHT},
        {"FIRE", Action::FIRE},
        {"TOGGLE_HUD_HELP", Action::TOGGLE_HUD_HELP},
        {"QUIT", Action::QUIT},
        {"CONFIRM", Action::CONFIRM},
    };
    const auto it = conversions.find(name);

    if (it == conversions.end())
        return false;
    action = it->second;
    return true;
}
}
