/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Dimensions
*/

#ifndef R_TYPE_DIMENSIONS_HPP
#define R_TYPE_DIMENSIONS_HPP

#include <limits>

namespace kollection::core {
inline constexpr double INF = std::numeric_limits<double>::infinity();

struct Size {
    double width = 0;
    double height = 0;

    bool operator==(const Size &) const = default;
};

struct Offset {
    double x = 0;
    double y = 0;

    bool operator==(const Offset &) const = default;

    Offset operator+(Offset o) const;

    Offset operator-(Offset o) const;
};

struct Rect {
    double x = 0;
    double y = 0;
    double width = 0;
    double height = 0;

    [[nodiscard]] bool contains(Offset p) const;
};

struct EdgeInsets {
    double left = 0;
    double top = 0;
    double right = 0;
    double bottom = 0;

    static EdgeInsets all(double v);

    static EdgeInsets symmetric(double h, double v);

    [[nodiscard]] double horizontal() const;

    [[nodiscard]] double vertical() const;

    bool operator==(const EdgeInsets &) const = default;
};
}

#endif //R_TYPE_DIMENSIONS_HPP
