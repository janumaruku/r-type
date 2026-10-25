/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rng.cpp
*/
#include "engine/core/Rng.hpp"

namespace engine::core {

Rng::Rng(std::uint32_t seed) : _seed(seed), _engine(seed) {}

std::uint32_t Rng::randomSeed() {
    return std::random_device{}();
}

int Rng::uniformInt(int lo, int hi) {
    return std::uniform_int_distribution<int>(lo, hi)(_engine);
}

float Rng::uniformFloat(float lo, float hi) {
    return std::uniform_real_distribution<float>(lo, hi)(_engine);
}

bool Rng::chance(float probability) {
    return std::bernoulli_distribution(probability)(_engine);
}

std::uint32_t Rng::seed() const noexcept {
    return _seed;
}

} // namespace engine::core