/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PredictionSystem
*/

#ifndef R_TYPE_PREDICTION_SYSTEM_HPP
#define R_TYPE_PREDICTION_SYSTEM_HPP

#include <optional>

#include "engine/core/Vec2.hpp"
#include "engine/ecs/Registry.hpp"

namespace rtype::client {
struct Correction {
    std::optional<engine::core::Vec2f> target;
};

void predictionSystem(engine::ecs::Registry &registry);
}

#endif //R_TYPE_PREDICTION_SYSTEM_HPP
