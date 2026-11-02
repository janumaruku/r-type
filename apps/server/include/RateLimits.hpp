/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RateLimits.hpp
*/

#ifndef RATE_LIMITS_HPP
#define RATE_LIMITS_HPP

#include <asio.hpp>
#include <chrono>
#include <map>

namespace rtype::server {

using Clock     = std::chrono::steady_clock;
using TimePoint = Clock::time_point;

class WindowCounter {
public:
    WindowCounter() = default;
    explicit WindowCounter(std::uint32_t limit,
        std::chrono::milliseconds window) noexcept;

    [[nodiscard]] bool record(TimePoint now);
    [[nodiscard]] std::uint32_t countInWindow() const noexcept;

private:
    std::uint32_t _limit;
    std::chrono::milliseconds _window;
    TimePoint _windowStart;
    std::uint32_t _count = 0;
};

class FloodTracker {
public:
    FloodTracker() = default;
    [[nodiscard]] bool allow(TimePoint now);
    [[nodiscard]] bool shouldKick(TimePoint now) const noexcept;

private:
    WindowCounter _datagrams{240, std::chrono::seconds(1)};
    std::uint32_t _floodedSeconds = 0;
    TimePoint _trackedWindowStart;
    bool _hasTrackedWindow = false;
    bool _currentWindowFlooded = false;
};

class RejectLimiter {
public:
    RejectLimiter() = default;
    [[nodiscard]] bool allow(const asio::ip::udp::endpoint &to, TimePoint now);

private:
    std::map<asio::ip::udp::endpoint, TimePoint> _lastByAddress;
    WindowCounter _global{20, std::chrono::seconds(1)};
};

} // namespace rtype::server

#endif
