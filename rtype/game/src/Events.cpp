/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Events
*/

#include "rtype/game/Events.hpp"

#include <utility>

namespace rtype::game {

void EventOutbox::push(GameEvent event)
{
    _events.push_back(std::move(event));
}

std::vector<GameEvent> EventOutbox::drain()
{
    return std::exchange(_events, {});
}

std::size_t EventOutbox::size() const noexcept
{
    return _events.size();
}
}
