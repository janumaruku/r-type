/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StopWatch.cpp
*/

#include "../include/engine/runtime/StopWatch.hpp"

namespace engine::runtime
{

    Stopwatch::Stopwatch() noexcept : _start(MyClock::now()) {}

    double Stopwatch::restart() noexcept
    {
        TimePoint TimeNow = MyClock::now();
        std::chrono::duration<double> TimeElapsed = TimeNow - _start;

        _start = TimeNow;
        return TimeElapsed.count();
    }

    double Stopwatch::elapsedSeconds() const noexcept
    {
        TimePoint TimeNow = MyClock::now();
        std::chrono::duration<double> TimeElapsed = TimeNow - _start;

        return TimeElapsed.count();
    }

}
