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
    double w = 0;
    double h = 0;

    bool operator==(const Size &) const = default;
};

struct Offset {
    double x = 0;
    double y = 0;

    bool operator==(const Offset &) const = default;

    Offset operator+(Offset o) const;

    Offset operator-(Offset o) const;
};

}

#endif //R_TYPE_DIMENSIONS_HPP
