/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Starfield.hpp
*/

#ifndef STARFIELD_HPP
#define STARFIELD_HPP

#include <array>
#include <cstdint>
namespace rtype::client {

namespace layers {
inline constexpr std::int16_t FAR_STARS  = 0;
inline constexpr std::int16_t NEAR_STARS = 10;
inline constexpr std::int16_t ENEMIES    = 100;
inline constexpr std::int16_t MISSILES   = 110;
inline constexpr std::int16_t PLAYERS    = 120;
inline constexpr std::int16_t EXPLOSIONS = 200;
inline constexpr std::int16_t HUD        = 1000;
} // namespace layers

struct Star {
    float speed;
};

struct StarLayer {
    std::size_t count;
    float speed;
    std::int16_t layer;
    std::uint8_t brightness;
};
inline constexpr std::array<StarLayer, 3> STAR_LAYERS{{
    {.count = 70, .speed = 20.0F, .layer = layers::FAR_STARS, .brightness = 90},
    {.count         = 40,
        .speed      = 55.0F,
        .layer      = layers::FAR_STARS,
        .brightness = 160},
    {.count         = 20,
        .speed      = 120.0F,
        .layer      = layers::NEAR_STARS,
        .brightness = 255},
}};

} // namespace rtype::client

#endif /* !STARFIELD_HPP_ */
