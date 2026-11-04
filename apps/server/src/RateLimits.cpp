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

bool RejectLimiter::allow(const asio::ip::udp::endpoint &to, TimePoint now)
{
    auto it = _lastByAddress.find(to);

    if (it != _lastByAddress.end() &&
        now - it->second < std::chrono::seconds(1))
        return false;

    if (!_global.record(now))
        return false;

    _lastByAddress[to] = now;
    return true;
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
    return _floodedSeconds >= KICK_AFTER
        && now - _lastFloodedWindow < 2 * WINDOW;
}

} // namespace rtype::server
