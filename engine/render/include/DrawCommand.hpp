/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DrawCommand.hpp
*/

#ifndef DRAW_COMMAND_HPP
#define DRAW_COMMAND_HPP

#include <cstdint>
#include <variant>
#include <string>

#include "Vec2.hpp"
#include "Rect2.hpp"
#include "Color.hpp"

namespace engine::render {

struct TextureId {
    std::uint32_t value{};

    explicit TextureId(std::uint32_t id) : value{id} {}
};

struct FontId {
    std::uint32_t value{};

    explicit FontId(std::uint32_t id) : value{id} {}
};

struct SpriteCmd {
    TextureId texture;
    core::IntRect source;
    core::Vec2f position;

    core::Vec2f origin;
    core::Vec2f scale;
    float rotationDeg;

    core::Color tint;

    int16_t layer;
};

struct RectCmd{
    core::FloatRect rect;
    core::Color fill;
    core::Color outline;
    float outlineThickness;
    int16_t layer;
};

struct TextCmd {
    FontId font;
    std::string text;
    std::uint8_t characterSize;
    core::Vec2f position;
    core::Color color;
    int16_t layer;
};

using DrawCommand = std::variant<SpriteCmd, RectCmd, TextCmd>;

}
#endif /* !DRAW_COMMAND_HPP */