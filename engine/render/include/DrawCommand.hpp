/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** DrawCommand.hpp
*/

#ifndef DRAW_COMMAND_HPP
#define DRAW_COMMAND_HPP

#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <string>
#include <variant>

#include "Color.hpp"
#include "Rect2.hpp"
#include "Vec2.hpp"

namespace engine::render {

struct TextureId {
    std::uint16_t value{};

    TextureId() = default;
    constexpr explicit TextureId(std::uint16_t id) : value{id} {}

    bool operator==(const TextureId &) const = default;
};

/*
** A default-constructed FontId names Raylib's built-in font. Cache ids stop
** one below it, so a loaded font can never be mistaken for the default one.
*/
struct FontId {
    static constexpr std::uint16_t DEFAULT =
        std::numeric_limits<std::uint16_t>::max();

    std::uint16_t value = DEFAULT;

    FontId() = default;
    constexpr explicit FontId(std::uint16_t id) : value{id} {}

    [[nodiscard]] constexpr bool isDefault() const noexcept
    {
        return value == DEFAULT;
    }

    bool operator==(const FontId &) const = default;
};

struct SpriteCmd {
    TextureId texture;
    core::IntRect source;
    core::Vec2f position;

    core::Vec2f origin;
    core::Vec2f scale;
    float rotationDeg;

    core::Color tint;

    std::int16_t layer;
};

struct RectCmd {
    core::FloatRect rect;
    core::Color fill;
    core::Color outline;
    float outlineThickness;
    std::int16_t layer;
};

struct TextCmd {
    FontId font;
    std::string text;
    std::uint8_t characterSize;
    core::Vec2f position;
    core::Color color;
    std::int16_t layer;
};

using DrawCommand = std::variant<SpriteCmd, RectCmd, TextCmd>;

}

template <>
struct std::hash<engine::render::TextureId> {
    std::size_t operator()(engine::render::TextureId id) const noexcept
    {
        return std::hash<std::uint16_t>{}(id.value);
    }
};

#endif /* !DRAW_COMMAND_HPP */
