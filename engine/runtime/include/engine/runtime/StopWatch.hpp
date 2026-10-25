/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StopWatch.hpp
*/

#ifndef STOPWATCH_HPP
#define STOPWATCH_HPP
#include "Clock.hpp"

namespace engine::runtime {
class Stopwatch {
public:
    Stopwatch() noexcept;

    double restart() noexcept;

    [[nodiscard]] double elapsedSeconds() const noexcept;

private:
    TimePoint _start;
};
}

#endif
