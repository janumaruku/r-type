/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Styles
*/

#include "kollection/core/Styles.hpp"

namespace kollection::core {
const Color Color::BLACK = {.r = 0, .g = 0, .b = 0};
const Color Color::WHITE = {.r = 255, .g = 255, .b = 255};
const Color Color::BLUE = {.r = 0, .g = 0, .b = 255};
const Color Color::TRANSPARENT = {.r = 0, .g = 0, .b = 0, .a = 0};

TextStyle TextStyle::merge(const TextStyle &o) const
{
    auto out = o;

    setUnsetStyle(out.fg, this->fg);
    setUnsetStyle(out.bg, this->bg);
    setUnsetStyle(out.size, this->size);
    setUnsetStyle(out.bold, this->bold);
    setUnsetStyle(out.underline, this->underline);
    setUnsetStyle(out.font, this->font);

    return out;
}
}
