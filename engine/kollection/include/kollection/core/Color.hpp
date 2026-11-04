/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Color.hpp
*/

#ifndef KOLLECTION_CORE_COLOR_HPP
#define KOLLECTION_CORE_COLOR_HPP

#include <cstdint>

namespace kollection::core {

/**
 * @brief An 8-bit RGBA colour, opaque black by default.
 *
 * Transparent is a real colour here, never a way to say "not set": a field
 * that can be unset is a std::optional<Color>.
 */
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    bool operator==(const Color &) const = default;

    static const Color Black;
    static const Color White;
    static const Color Blue;
    static const Color Transparent;
};

}

#endif /* !KOLLECTION_CORE_COLOR_HPP */
