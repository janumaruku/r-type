/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedTimestep.cpp
*/

#include "../include/engine/runtime/FixedTimestep.hpp"
#include <iostream>
#include <cmath>

namespace engine::runtime
{

    FixedTimestep::FixedTimestep(double stepSeconds, int maxStepsPerFrame)
    : _step(stepSeconds), _maxSteps(maxStepsPerFrame), _accumulator(0.0)
    {
        if (_step <= 0.0)
            std::cout << "Steps must be > 0." << std::endl;
        if (_maxSteps < 1)
            std::cout << "Max steps must be atleast 1." << std::endl;
    }

    int FixedTimestep::advance(double frameSeconds)
    {
        int steps = 0;

        if (frameSeconds < 0.0)
            frameSeconds = 0.0;
        if (frameSeconds > 0.25)
            frameSeconds = 0.25;
        _accumulator += frameSeconds;
        steps = static_cast<int>(_accumulator / _step);
        if (steps > _maxSteps) {
            steps = _maxSteps;
            _accumulator = std::fmod(_accumulator, _step);
        } else {
            _accumulator -= static_cast<double>(steps) * _step;
        }
        return steps;
    }

    double FixedTimestep::alpha() const
    {
        return _accumulator / _step;
    }

    double FixedTimestep::step() const
    {
        return _step;
    }

}
