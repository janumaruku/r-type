/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedTimestep.hpp
*/

#ifndef FIXEDTIMESTEP_HPP
#define FIXEDTIMESTEP_HPP

namespace engine::runtime {
class FixedTimestep {
public:
    FixedTimestep(double stepSeconds, int maxStepsPerFrame);

    int advance(double frameSeconds);

    [[nodiscard]] double alpha() const;

    [[nodiscard]] double step() const;

private:
    double _step;
    double _accumulator;
    int _maxSteps;
};
}

#endif
