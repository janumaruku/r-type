/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** BoxConstraints
*/

#include "kollection/core/BoxConstraints.hpp"

#include <algorithm>

namespace kollection::core {

BoxConstraints BoxConstraints::tight(Size size)
{
    return {
        .minWidth = size.width,
        .maxWidth = size.width,
        .minHeight = size.height,
        .maxHeight = size.height,
    };
}

BoxConstraints BoxConstraints::loose(Size size)
{
    return {
        .minWidth = 0,
        .maxWidth = size.width,
        .minHeight = 0,
        .maxHeight = size.height,
    };
}

BoxConstraints BoxConstraints::expand()
{
    return {
        .minWidth = INF,
        .maxWidth = INF,
        .minHeight = INF,
        .maxHeight = INF,
    };
}

Size BoxConstraints::constrain(Size size) const
{
    return {
        .width = std::clamp(size.width, minWidth, maxWidth),
        .height = std::clamp(size.height, minHeight, maxHeight),
    };
}

Size BoxConstraints::smallest() const
{
    return constrain({
        .width = 0,
        .height = 0,
    });
}

bool BoxConstraints::isTight() const
{
    return minWidth == maxWidth && minHeight == maxHeight;
}

bool BoxConstraints::hasBoundedWidth() const
{
    return maxWidth < INF;
}

bool BoxConstraints::hasBoundedHeight() const
{
    return maxHeight < INF;
}

BoxConstraints BoxConstraints::loosen() const
{
    return {
        .minWidth = 0,
        .maxWidth = maxWidth,
        .minHeight = 0,
        .maxHeight = maxHeight,
    };
}

BoxConstraints BoxConstraints::deflate(const EdgeInsets &insets) const
{
    const double deflatedMaxWidth =
        std::max(0.0, maxWidth - insets.horizontal());
    const double deflatedMaxHeight =
        std::max(0.0, maxHeight - insets.vertical());

    return {
        .minWidth = std::clamp(minWidth - insets.horizontal(), 0.0,
            deflatedMaxWidth),
        .maxWidth = deflatedMaxWidth,
        .minHeight = std::clamp(minHeight - insets.vertical(), 0.0,
            deflatedMaxHeight),
        .maxHeight = deflatedMaxHeight,
    };
}

BoxConstraints BoxConstraints::enforce(const BoxConstraints &parent) const
{
    return {
        .minWidth = std::clamp(minWidth, parent.minWidth, parent.maxWidth),
        .maxWidth = std::clamp(maxWidth, parent.minWidth, parent.maxWidth),
        .minHeight =
            std::clamp(minHeight, parent.minHeight, parent.maxHeight),
        .maxHeight =
            std::clamp(maxHeight, parent.minHeight, parent.maxHeight),
    };
}
}
