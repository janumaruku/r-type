/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Styles
*/

#ifndef R_TYPE_STYLES_HPP
#define R_TYPE_STYLES_HPP

#include <cstdint>

namespace kollection::core {
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    bool operator==(const Color &) const = default;

    static const Color BLACK, WHITE, BLUE, TRANSPARENT;
};
}

#endif //R_TYPE_STYLES_HPP
