/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scene
*/

#include "Scene.hpp"

#include <string>
#include <utility>

#include "engine/core/Color.hpp"
#include "engine/core/Rect2.hpp"

namespace render_demo {
namespace {
using engine::core::Color;
using engine::core::FloatRect;
using engine::render::RectCmd;
using engine::render::RenderQueue;
using engine::render::SpriteCmd;
using engine::render::TextCmd;

constexpr std::int16_t FLOOR_LAYER = 0;
constexpr std::int16_t PANEL_LAYER = 100;
constexpr std::int16_t SAME_LAYER = 150;
constexpr std::int16_t ENEMY_LAYER = 200;
constexpr std::int16_t LABEL_LAYER = 300;
constexpr std::int16_t BULLET_LAYER = 400;
constexpr std::int16_t SHIP_LAYER = 500;
constexpr std::int16_t HUD_LAYER = 1000;

const Color WHITE{255, 255, 255, 255};
const Color NO_OUTLINE{0, 0, 0, 0};

SpriteCmd sprite(const SceneAssets &assets, const Cell cell,
    const Vec2f &position, const bool flipped, const std::int16_t layer)
{
    const int x = static_cast<int>(cell) * CELL;

    return SpriteCmd{
        .texture = assets.sheet,
        .source = engine::core::IntRect{engine::core::Vec2i{x, 0},
            flipped ? -CELL : CELL, CELL},
        .position = position,
        .origin = Vec2f{CELL / 2.0F, CELL / 2.0F},
        .scale = Vec2f{SPRITE_SCALE, SPRITE_SCALE},
        .rotationDeg = 0.0F,
        .tint = WHITE,
        .layer = layer,
    };
}

RectCmd rect(const Vec2f &origin, const float width, const float height,
    const Color &fill, const std::int16_t layer)
{
    return RectCmd{
        .rect = FloatRect{origin, width, height},
        .fill = fill,
        .outline = NO_OUTLINE,
        .outlineThickness = 0.0F,
        .layer = layer,
    };
}

TextCmd text(const engine::render::FontId font, std::string value,
    const Vec2f &position, const std::uint8_t size, const Color &color)
{
    return TextCmd{
        .font = font,
        .text = std::move(value),
        .characterSize = size,
        .position = position,
        .color = color,
        .layer = HUD_LAYER,
    };
}

void pushFloor(RenderQueue &queue, const SceneAssets &assets)
{
    const float side = CELL * SPRITE_SCALE;
    const Vec2f row{side / 2.0F, LOGICAL_SIZE.y() - side / 2.0F};

    for (float x = 0.0F; x < LOGICAL_SIZE.x(); x += side)
        queue.push(sprite(assets, Cell::TILE, row + Vec2f{x, 0.0F}, false,
            FLOOR_LAYER));
}

/*
** Left: a label (300), an enemy (200) and a panel (100) pushed top layer
** first, so they only stack right once sorted. Right: two rectangles on the
** same layer, red pushed before green, so green must stay on top.
*/
void pushLayerChecks(RenderQueue &queue, const SceneAssets &assets)
{
    TextCmd label = text(assets.font, "layer 300", Vec2f{120.0F, 150.0F}, 28,
        WHITE);

    label.layer = LABEL_LAYER;
    queue.push(std::move(label));
    queue.push(sprite(assets, Cell::ENEMY, Vec2f{200.0F, 160.0F}, false,
        ENEMY_LAYER));
    queue.push(rect(Vec2f{100.0F, 120.0F}, 240.0F, 100.0F,
        Color{40, 90, 200, 255}, PANEL_LAYER));

    queue.push(rect(Vec2f{460.0F, 120.0F}, 160.0F, 100.0F,
        Color{220, 50, 60, 255}, SAME_LAYER));
    queue.push(rect(Vec2f{540.0F, 150.0F}, 160.0F, 100.0F,
        Color{50, 200, 90, 255}, SAME_LAYER));
}

void pushHud(RenderQueue &queue, const SceneAssets &assets,
    const Frame &frame)
{
    const Color dim{170, 180, 200, 255};

    queue.push(text(assets.font, "shots fired: " + std::to_string(frame.fired),
        Vec2f{20.0F, 16.0F}, 32, WHITE));
    queue.push(text(assets.font, "loaded font: DejaVu Sans Mono",
        Vec2f{20.0F, 56.0F}, 20, dim));
    queue.push(text(engine::render::FontId{},
        "default font: arrows/WASD move, SPACE/LCTRL fire, ESC quits",
        Vec2f{20.0F, 84.0F}, 20, dim));
    queue.push(text(engine::render::FontId{},
        "left: 300 over 200 over 100   right: same layer, green pushed last",
        Vec2f{100.0F, 270.0F}, 20, dim));
}
}

void fillQueue(RenderQueue &queue, const SceneAssets &assets,
    const Frame &frame)
{
    pushHud(queue, assets, frame);
    queue.push(sprite(assets,
        frame.ship.thrusting ? Cell::SHIP_THRUST : Cell::SHIP,
        frame.ship.position, frame.ship.facingLeft, SHIP_LAYER));
    for (const Vec2f &bullet: frame.bullets)
        queue.push(sprite(assets, Cell::BULLET, bullet, false, BULLET_LAYER));
    pushLayerChecks(queue, assets);
    pushFloor(queue, assets);
}

}
