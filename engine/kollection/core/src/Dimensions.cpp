/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Dimensions
*/

#include "kollection/core/Dimensions.hpp"

namespace kollection::core {
Offset Offset::operator+(Offset other) const
{
    return {
        .x = x + other.x,
        .y = y + other.y,
    };
}

Offset Offset::operator-(Offset other) const
{
    return {
        .x = x - other.x,
        .y = y - other.y,
    };
}

bool Rect::contains(Offset point) const
{
    return point.x >= x && point.y >= y && point.x < x + width
        && point.y < y + height;
}

EdgeInsets EdgeInsets::all(double inset)
{
    return {
        .left = inset,
        .top = inset,
        .right = inset,
        .bottom = inset,
    };
}

EdgeInsets EdgeInsets::symmetric(double horizontalInset, double verticalInset)
{
    return {
        .left = horizontalInset,
        .top = verticalInset,
        .right = horizontalInset,
        .bottom = verticalInset,
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

Offset Alignment::inscribe(Size child, Size parent) const
{
    return {
        .x = (parent.width - child.width) * (x + 1) / 2,
        .y = (parent.height - child.height) * (y + 1) / 2,
    };
}
}
