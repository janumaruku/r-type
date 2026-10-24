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
    [[nodiscard]] static std::uint32_t randomSeed(); // from std::random_device

    [[nodiscard]] int uniformInt(int lo, int hi); // inclusive: lo <= result <= hi
    [[nodiscard]] float uniformFloat(float lo, float hi); // lo <= result < hi
    [[nodiscard]] bool chance(float probability); // clamped to [0, 1]
    [[nodiscard]] std::uint32_t seed() const noexcept;

private:
    std::uint32_t _seed;
    std::mt19937 _engine;
};

}

#endif