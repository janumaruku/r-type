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
    return {
        .x = x + o.x,
        .y = y + o.y,
    };
}

Offset Offset::operator-(Offset o) const
{
    return {
        .x = x - o.x,
        .y = y - o.y,
    };
}

bool Rect::contains(Offset p) const
{
    return p.x >= x && p.y >= y && p.x < x + width && p.y < y + height;
}

EdgeInsets EdgeInsets::all(double v)
{
    return {
        .left = v,
        .top = v,
        .right = v,
        .bottom = v,
    };
}

EdgeInsets EdgeInsets::symmetric(double h, double v)
{
    return {
        .left = h,
        .top = v,
        .right = h,
        .bottom = v,
    };
}

double EdgeInsets::horizontal() const
{
    return left + right;
}

double EdgeInsets::vertical() const
{
    return top + bottom;
}
}
