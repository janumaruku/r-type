/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** FixedTimestep.cpp
*/

#include "engine/runtime/FixedTimestep.hpp"

#include <algorithm>
#include <cmath>

#include "engine/core/Errors.hpp"

namespace engine::runtime {

FixedTimestep::FixedTimestep(double stepSeconds, int maxStepsPerFrame)
    : _step(stepSeconds), _accumulator(0.0), _maxSteps(maxStepsPerFrame)
{
    if (_step <= 0.0)
        throw core::EngineError("FixedTimestep: the step must be > 0");
    if (_maxSteps < 1)
        throw core::EngineError("FixedTimestep: the step cap must be at least 1");
}

int FixedTimestep::advance(double frameSeconds)
{
    int steps = 0;

    frameSeconds = std::clamp(frameSeconds, 0.0, 0.25);
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
