/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedRateTicker.cpp
*/

#include "engine/runtime/FixedRateTicker.hpp"

#include <thread>

#include "engine/core/Errors.hpp"

namespace engine::runtime {

namespace {
constexpr std::chrono::milliseconds MAX_LATENESS{250};
}

FixedRateTicker::FixedRateTicker(std::chrono::nanoseconds period)
    : _period(period), _next(Clock::now() + period)
{
    if (_period <= std::chrono::nanoseconds::zero())
        throw core::EngineError("FixedRateTicker: the period must be > 0");
}

/*
** Deadlines are absolute: the next one is the previous one plus the period,
** never "now + period", so oversleeping doesn't add up and lower the rate.
** A small delay is caught up by not sleeping until the server is on time.
** Past MAX_LATENESS the backlog is dropped instead of run as a burst.
*/
void FixedRateTicker::waitNext()
{
    if (const TimePoint now = Clock::now(); now - _next > MAX_LATENESS) {
        const auto late =
            std::chrono::duration_cast<std::chrono::milliseconds>(now - _next);
        _logger.warn("tick {} is {} ms late, skipping the missed ticks",
            _tick, late.count());
        _next = now;
        ++_overruns;
    } else {
        std::this_thread::sleep_until(_next);
    }
    _next += _period;
    ++_tick;
}

std::uint32_t FixedRateTicker::tick() const noexcept
{
    return _tick;
}

std::uint64_t FixedRateTicker::overruns() const noexcept
{
    return _overruns;
}

}
