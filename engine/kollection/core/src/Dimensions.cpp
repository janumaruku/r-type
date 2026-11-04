/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Dimensions
*/

#include "kollection/core/Dimensions.hpp"

namespace kollection::core {
Offset Offset::operator+(Offset o) const
{
    return {.x = x + o.x, .y = y + o.y};
}

Offset Offset::operator-(Offset o) const
{
    return {.x = x - o.x, .y = y - o.y};
}

bool Rect::contains(Offset p) const
{
    return p.x >= x && p.y >= y && p.x < x + w && p.y < y + h;
}
}
