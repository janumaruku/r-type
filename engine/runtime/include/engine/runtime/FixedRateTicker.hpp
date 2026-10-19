/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedRateTicker.hpp
*/

#ifndef FIXEDRATETICKER_HPP
    #define FIXEDRATETICKER_HPP
    #include "Clock.hpp"
    // #include <chrono>

namespace engine::runtime {
    class FixedRateTicker {
        public:
            FixedRateTicker(std::chrono::nanoseconds period);
            void waitNext();
            uint32_t tick();
            uint64_t overruns();

        private:
            std::chrono::nanoseconds _period;
            TimePoint _next;
            uint32_t _tick;
    };
}

#endif
