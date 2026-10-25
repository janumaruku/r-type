/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Color.tpp
*/

#ifndef COLOR_TPP
#define COLOR_TPP

#include "engine/core/Color.hpp"

namespace engine::core {
inline Color::Color() noexcept :
r(0), g(0), b(0), a(255)
{}

inline Color::Color(const std::uint8_t r, const std::uint8_t g, const std::uint8_t b, const std::uint8_t a) noexcept :
r(r), g(g), b(b), a(a)
{}

[[nodiscard]] inline Color Color::withAlpha(const std::uint8_t &alpha) const noexcept
{
    return {r, g, b, alpha};

}
}

#endif