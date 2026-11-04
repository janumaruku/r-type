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

void PaintContext::fillRect(const core::Rect &r, const core::Color &c) const
{
    _list.emplace_back(FillRect{.rect = r, .color = c});
}

void PaintContext::strokeRect(const core::Rect &r, const core::Color &c,
    const double w) const
{
    _list.emplace_back(StrokeRect{.rect = r, .color = c, .width = w});
}

void PaintContext::drawText(const core::Offset &at,
    const core::TextRun &run) const
{
    _list.emplace_back(DrawText{.at = at, .run = run});
}
}
