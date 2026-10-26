/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedRateTicker.hpp
*/

#ifndef FIXEDRATETICKER_HPP
#define FIXEDRATETICKER_HPP

#include <chrono>
#include <cstdint>

#include "engine/core/Logger.hpp"
#include "engine/runtime/Clock.hpp"

namespace engine::runtime {
class FixedRateTicker {
public:
    explicit FixedRateTicker(std::chrono::nanoseconds period);

    void waitNext();

    [[nodiscard]] std::uint32_t tick() const noexcept;

    [[nodiscard]] std::uint64_t overruns() const noexcept;

private:
    std::chrono::nanoseconds _period;
    TimePoint _next;
    std::uint32_t _tick = 0;
    std::uint64_t _overruns = 0;
    core::log::Logger _logger{"runtime"};
};
}

#endif
