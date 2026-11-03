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
    if (copies == 0)
        return;
    event.eventId = _nextEventId++;
    _pending.push_back(
        Pending{.event = event, .copies = copies, .copiesLeft = copies});
    while (_pending.size() > MAX_PENDING)
        dropOne();
}

/*
** _pending is in the order events were added, so the first match is the
** oldest. A 1-copy event (a shot, a kill) is only a cosmetic hint, while the
** repeated ones (a death, a join, the game over) change what the HUD shows.
*/
void EventRepeater::dropOne()
{
    const auto oneCopy = std::ranges::find_if(_pending,
        [](const Pending &pending) { return pending.copies == 1; });

    _pending.erase(oneCopy != _pending.end() ? oneCopy : _pending.begin());
    ++_dropped;
}

std::vector<protocol::Event> EventRepeater::takeDue()
{
    std::vector<protocol::Event> due;

    if (_dropped != 0) {
        _logger.warn("Event queue full: {} event(s) dropped", _dropped);
        _dropped = 0;
    }
    due.reserve(_pending.size());
    for (auto &pending : _pending) {
        due.push_back(pending.event);
        --pending.copiesLeft;
    }
    std::erase_if(_pending,
        [](const Pending &pending) { return pending.copiesLeft == 0; });
    return due;
}

std::size_t EventRepeater::pending() const noexcept
{
    return _pending.size();
}
}
