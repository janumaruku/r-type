/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Styles
*/

#ifndef R_TYPE_STYLES_HPP
#define R_TYPE_STYLES_HPP

#include <cstdint>
#include <optional>

namespace kollection::core {
struct Color {
    std::uint8_t r = 0;
    std::uint8_t g = 0;
    std::uint8_t b = 0;
    std::uint8_t a = 255;

    bool operator==(const Color &) const = default;

    static const Color BLACK;
    static const Color WHITE;
    static const Color BLUE;
    static const Color TRANSPARENT;
};

struct TextStyle {
    std::optional<Color> fg;
    std::optional<Color> bg;
    std::optional<double> size; // logical px; ignored by the terminal backend
    std::optional<bool> bold;
    std::optional<bool> underline;
    std::optional<int> font; // backend font id; unset = default font

    bool operator==(const TextStyle &) const = default;

    template <typename  T>
    static void setUnsetStyle(std::optional<T> &left, const std::optional<T> &right)
    {
        if (!left.has_value() && right.has_value())
            left.value() = right.value();
    }

    [[nodiscard]] TextStyle merge(const TextStyle &o) const; // fields set in o win
};
}

#endif //R_TYPE_STYLES_HPP
