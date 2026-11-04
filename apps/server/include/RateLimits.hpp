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

    /**
     * @brief When the current window started: the first @ref record after
     * the previous one ended.
     */
    [[nodiscard]] TimePoint windowStart() const noexcept;

private:
    std::uint32_t _limit;
    std::chrono::milliseconds _window;
    TimePoint _windowStart;
    std::uint32_t _count = 0;
};

/**
 * @brief Drops a session's datagrams above 240 per second, and tells when
 * 3 seconds in a row went over: the KICK(RATE_LIMITED) threshold.
 */
class FloodTracker {
public:
    static constexpr std::chrono::seconds WINDOW{1};
    static constexpr std::uint32_t LIMIT = 240;
    static constexpr std::uint32_t KICK_AFTER = 3;

    FloodTracker() = default;

    /**
     * @brief Records one datagram.
     * @return `false` for the excess above @ref LIMIT in the current
     * window, which the caller drops.
     */
    [[nodiscard]] bool allow(TimePoint now);

    /**
     * @brief `true` once @ref KICK_AFTER consecutive windows went over,
     * the last one being the current or the previous window.
     */
    [[nodiscard]] bool shouldKick(TimePoint now) const noexcept;

private:
    WindowCounter _datagrams{LIMIT, WINDOW};
    std::uint32_t _floodedSeconds = 0;
    TimePoint _lastFloodedWindow{};
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
