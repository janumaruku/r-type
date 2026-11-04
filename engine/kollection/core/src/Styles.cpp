/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Styles
*/

#include "kollection/core/Styles.hpp"

namespace kollection::core {
namespace {
template <typename T>
void setUnsetStyle(std::optional<T> &field,
    const std::optional<T> &fallback)
{
    if (!field)
        field = fallback;
}
}

TextStyle TextStyle::merge(const TextStyle &overrides) const
{
    auto out = overrides;

    setUnsetStyle(out.fg, this->fg);
    setUnsetStyle(out.bg, this->bg);
    setUnsetStyle(out.size, this->size);
    setUnsetStyle(out.bold, this->bold);
    setUnsetStyle(out.underline, this->underline);
    setUnsetStyle(out.font, this->font);

    return out;
}
}
