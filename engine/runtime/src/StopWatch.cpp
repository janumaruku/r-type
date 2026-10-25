/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StopWatch.cpp
*/

#include "engine/runtime/StopWatch.hpp"

namespace engine::runtime {

Stopwatch::Stopwatch() noexcept: _start(Clock::now())
{
}

double Stopwatch::restart() noexcept
{
    const TimePoint timeNow = Clock::now();
    const std::chrono::duration<double> timeElapsed = timeNow - _start;

    _start = timeNow;
    return timeElapsed.count();
}

double Stopwatch::elapsedSeconds() const noexcept
{
    const TimePoint timeNow = Clock::now();
    const std::chrono::duration<double> timeElapsed = timeNow - _start;

    return timeElapsed.count();
}

}
