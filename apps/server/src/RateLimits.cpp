/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RateLimits.cpp
*/

#include "RateLimits.hpp"

#include <chrono>

namespace rtype::server {

WindowCounter::WindowCounter(std::uint32_t limit,
    std::chrono::milliseconds window) noexcept: _limit{limit}, _window{window}
{}

bool WindowCounter::record(TimePoint now)
{
    if (_count == 0 || now - _windowStart >= _window) {
        _windowStart = now;
        _count       = 0;
    }

    if (_count >= _limit)
        return false;

    ++_count;
    return true;
}

std::uint32_t WindowCounter::countInWindow() const noexcept
{
    return _count;
}

TimePoint WindowCounter::windowStart() const noexcept
{
    return _windowStart;
}

/*
** Every entry left after the pruning is younger than WINDOW, and an entry
** is only added when _global allows it. Its fixed windows let LIMIT through
** at the end of one window and LIMIT more at the start of the next, so the
** map holds at most 2 x LIMIT addresses, whatever the sender spoofs.
*/
bool RejectLimiter::allow(const asio::ip::udp::endpoint &to, TimePoint now)
{
    std::erase_if(_lastByAddress,
        [now](const auto &entry) { return now - entry.second >= WINDOW; });

    if (_lastByAddress.contains(to) || !_global.record(now))
        return false;

    _lastByAddress.emplace(to, now);
    return true;
}

std::size_t RejectLimiter::trackedAddresses() const noexcept
{
    return _lastByAddress.size();
}

/*
** Windows start at the first datagram after the previous one ended, so
** they last at least WINDOW each. A flooded window that starts less than 2
** windows after the previous flooded one directly follows it; a quiet or a
** calm second in between pushes it 2 windows or more away, and the streak
** starts over at 1.
*/
bool FloodTracker::allow(TimePoint now)
{
    if (_datagrams.record(now))
        return true;

    const TimePoint window = _datagrams.windowStart();

    if (_floodedSeconds > 0 && window == _lastFloodedWindow)
        return false;
    if (_floodedSeconds > 0 && window - _lastFloodedWindow < 2 * WINDOW)
        ++_floodedSeconds;
    else
        _floodedSeconds = 1;
    _lastFloodedWindow = window;
    return false;
}

bool FloodTracker::shouldKick(TimePoint now) const noexcept
{
    return _floodedSeconds >= KICK_AFTER &&
        now - _lastFloodedWindow < 2 * WINDOW;
}

} // namespace rtype::server
