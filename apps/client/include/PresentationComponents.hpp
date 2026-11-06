/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** PresentationComponents.hpp
*/

#ifndef PRESENTATION_COMPONENTS_HPP
#define PRESENTATION_COMPONENTS_HPP

#include <cstdint>

#include "SpriteCatalog.hpp"
#include "engine/render/DrawCommand.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/core/Vec2.hpp"

namespace rtype::client {

struct Sprite {
    engine::render::TextureId texture;
    engine::core::IntRect frame;
    std::int16_t layer = 0;
    engine::core::Color tint = engine::core::Color::WHITE;
    engine::core::Vec2f scale{1.0F, 1.0F};
};

struct Animation {
    SpriteDefId def;
    std::uint8_t state = 0;
    std::size_t index = 0;
    float timer = 0.0F;
};

struct ShipTilt {
    float lastY = 0.0F;
    std::int8_t direction = 0;
    float heldTime = 0.0F;
};

inline constexpr float TILT_SPEED_THRESHOLD = 20.0F;
inline constexpr float TILT_FULL_DELAY = 0.15F;

struct HighContrastOutline {};

} // namespace rtype::client
#endif /* !PRESENTATION_COMPONENTS_HPP_ */