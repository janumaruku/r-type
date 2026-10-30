/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Events
*/

#include "rtype/game/Events.hpp"

namespace rtype::game {

void EventOutbox::push(const GameEvent &event)
{
    _events.push_back(event);
}

std::vector<GameEvent> EventOutbox::drain()
{
    return _events;
}

std::size_t EventOutbox::size() const noexcept
{
    return _events.size();
}
}
