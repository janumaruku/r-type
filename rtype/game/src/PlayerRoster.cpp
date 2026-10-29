/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PlayerRoster
*/

#include "rtype/game/PlayerRoster.hpp"

namespace rtype::game {

std::optional<std::uint8_t> PlayerRoster::reserve()
{
    for (std::size_t i = 0; i < _slots.size(); ++i) {
        if (_slots[i].has_value())
            continue;
        const auto playerId = static_cast<std::uint8_t>(i);
        _slots[i].emplace(PlayerSlot{.playerId = playerId});
        return playerId;
    }
    return std::nullopt;
}

void PlayerRoster::release(const std::uint8_t playerId)
{
    if (playerId >= _slots.size())
        return;
    _slots[playerId].reset();
}
}
