/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PaintCommands
*/

#include "kollection/paint/PaintCommands.hpp"

namespace kollection::paint {
PaintContext::PaintContext(PaintList &list): _list(list)
{
}

void PaintContext::fillRect(const core::Rect &rect,
    const core::Color &color)
{
    _list.emplace_back(FillRect{.rect = rect, .color = color});
}

void PaintContext::strokeRect(const core::Rect &rect,
    const core::Color &color, const double width)
{
    _list.emplace_back(
        StrokeRect{.rect = rect, .color = color, .width = width});
}

void PaintContext::drawText(const core::Offset &at,
    const core::TextRun &run)
{
    _list.emplace_back(DrawText{.at = at, .run = run});
}
}
