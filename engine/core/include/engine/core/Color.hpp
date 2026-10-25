/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Color.hpp
*/

#ifndef COLOR_HPP
#define COLOR_HPP

#include <cstdint>

namespace engine::core {

struct Color {
    Color() noexcept;
    Color(std::uint8_t r,
        std::uint8_t g,
        std::uint8_t b,
        std::uint8_t a) noexcept;


    std::uint8_t r;
    std::uint8_t g;
    std::uint8_t b;
    std::uint8_t a;

    [[nodiscard]] bool operator==(const Color &other) const noexcept = default;
    [[nodiscard]] Color withAlpha(const std::uint8_t &alpha) const noexcept;

    static const Color WHITE;
    static const Color BLACK;
    static const Color RED;
    static const Color GREEN;
    static const Color BLUE;
    static const Color CLEAR;
    
};

inline const Color Color::WHITE = {255, 255, 255, 255};
inline const Color Color::BLACK = {0, 0, 0, 255};
inline const Color Color::RED = {255, 0, 0, 255};
inline const Color Color::GREEN = {0, 255, 0, 255};
inline const Color Color::BLUE = {0, 0, 255, 255};
inline const Color Color::CLEAR = {0, 0, 0, 0};

}

#endif

#include "Color.tpp"
