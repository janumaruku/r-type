/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Clock.hpp
*/

#ifndef CLOCK_HPP
    #define CLOCK_HPP
    #include <chrono>

namespace engine::runtime {
    using Clock = std::chrono::steady_clock;
    using TimePoint = std::chrono::time_point<Clock>;
}

#endif
