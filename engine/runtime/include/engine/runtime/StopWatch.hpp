/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** StopWatch.hpp
*/

#ifndef STOPWATCH_HPP
    #define STOPWATCH_HPP
    #include "Clock.hpp"

    class Stopwatch {
        public:
            Stopwatch() noexcept : _start(MyClock::now()) {}

            double restart() noexcept
            {
                TimePoint TimeNow = MyClock::now();
                std::chrono::duration<double> TimeElapsed = TimeNow - _start;

                _start = now;
                return TimeElapsed.count();
            }

            double elapsedSeconds() const noexcept
            {
                TimePoint TimeNow = MyClock::now();
                std::chrono::duration<double> TimeElapsed = TimeNow - _start;

                return TimeElapsed.count();
            }

        private:
            TimePoint _start;


    };


#endif
