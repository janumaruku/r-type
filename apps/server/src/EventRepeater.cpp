/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** EventRepeater.cpp
*/

#include "EventRepeater.hpp"

#include <algorithm>

namespace rtype::server {

void EventRepeater::add(protocol::Event event, std::uint8_t copies)
{
    event.eventId = _nextEventId++;
    if (_pending.size() >= MAX_PENDING) {
        const auto oneCopy = std::find_if(_pending.begin(), _pending.end(),
            [](const Pending &pending) { return pending.copiesLeft == 1; });
        if (copies > 1 && oneCopy != _pending.end())
            _pending.erase(oneCopy);
        else {
            if (!_overflowLogged) {
                _logger.warn("Event repeater queue full; dropping events");
                _overflowLogged = true;
            }
            return;
        }
    }
    if (copies == 0)
        return;
    _pending.push_back(Pending{.event = event, .copiesLeft = copies});
}

std::vector<protocol::Event> EventRepeater::takeDue()
{
    std::vector<protocol::Event> due;
    if (_pending.size() > MAX_PENDING)
        _pending.erase(_pending.begin(), _pending.begin() + (_pending.size() - MAX_PENDING));
    for (auto it = _pending.begin(); it != _pending.end();) {
        if (it->copiesLeft == 0) {
            it = _pending.erase(it);
            continue;
        }
        due.push_back(it->event);
        --it->copiesLeft;
        ++it;
    }
    return due;
}
}