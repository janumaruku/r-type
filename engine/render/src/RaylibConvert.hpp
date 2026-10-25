/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RaylibConvert.hpp
*/

#ifndef RAYLIB_CONVERT_HPP
#define RAYLIB_CONVERT_HPP

#include "engine/core/Color.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"

#include <raylib.h>

/*
** Engine types to Raylib types, field by field. The layouts may look alike,
** but nothing guarantees it, so no reinterpret_cast.
** Engine headers come before raylib.h: Raylib defines WHITE, BLACK, RED,
** GREEN and BLUE as macros, which would rewrite core::Color's constants.
*/
namespace engine::render::detail {

[[nodiscard]] inline ::Color toRaylib(const core::Color &color) noexcept
{
    return ::Color{color.r, color.g, color.b, color.a};
}

[[nodiscard]] inline Vector2 toRaylib(const core::Vec2f &vector) noexcept
{
    return Vector2{vector.x(), vector.y()};
}

[[nodiscard]] inline Rectangle toRaylib(const core::FloatRect &rect) noexcept
{
    return Rectangle{rect.origin().x(), rect.origin().y(), rect.width(),
        rect.height()};
}

[[nodiscard]] inline Rectangle toRaylib(const core::IntRect &rect) noexcept
{
    return Rectangle{static_cast<float>(rect.origin().x()),
        static_cast<float>(rect.origin().y()),
        static_cast<float>(rect.width()), static_cast<float>(rect.height())};
}

}

#endif /* !RAYLIB_CONVERT_HPP */
