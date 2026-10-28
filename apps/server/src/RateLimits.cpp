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

bool FloodTracker::shouldKick(TimePoint /*now*/) const noexcept
{
    return _floodedSeconds >= 3;
}

bool FloodTracker::allow(TimePoint now)
{
    const bool allowed = _datagrams.record(now);

    if (_datagrams.countInWindow() == 1) {
        if (_hasTrackedWindow &&
            (!_currentWindowFlooded ||
                now - _trackedWindowStart >= std::chrono::seconds(2))) {
            _floodedSeconds = 0;
        }

        _trackedWindowStart   = now;
        _hasTrackedWindow     = true;
        _currentWindowFlooded = false;
    }

    if (!allowed && !_currentWindowFlooded) {
        ++_floodedSeconds;
        _currentWindowFlooded = true;
    }

    return allowed;
}

} // namespace rtype::server
