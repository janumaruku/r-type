/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Letterbox.cpp
*/

#include "Letterbox.hpp"

#include <algorithm>

namespace engine::render {

core::FloatRect letterbox(const core::Vec2u &window,
    const core::Vec2f &logical)
{
    if (window.x() == 0 || window.y() == 0)
        return {core::Vec2f(0.0F, 0.0F), 0.0F, 0.0F};

    const auto windowWidth = static_cast<float>(window.x());
    const auto windowHeight = static_cast<float>(window.y());
    const float scale = std::min(windowWidth / logical.x(),
        windowHeight / logical.y());
    const float width = logical.x() * scale;
    const float height = logical.y() * scale;

    return {core::Vec2f((windowWidth - width) / 2.0F,
            (windowHeight - height) / 2.0F), width, height};
}

}
