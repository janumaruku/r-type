/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Rng.hpp
*/

#ifndef RNG_HPP
#define RNG_HPP

#include <cstdint>
#include <random>

namespace engine::core {
class Rng {
public:
    explicit Rng(std::uint32_t seed);
    static std::uint32_t randomSeed();          // from std::random_device, for when no --seed is given

    int uniformInt(int lo, int hi);             // inclusive: lo ≤ result ≤ hi
    float uniformFloat(float lo, float hi);     // lo ≤ result < hi
    bool chance(float probability);             // true with that probability, clamped to [0, 1]
    std::uint32_t seed() const noexcept;        // the seed it was built with, for logging

private:
    std::uint32_t _seed;
    std::mt19937 _engine;
};

}

#endif