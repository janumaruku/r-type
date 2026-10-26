/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Scene
*/

#ifndef RENDER_DEMO_SCENE_HPP
#define RENDER_DEMO_SCENE_HPP

#include <cstdint>
#include <vector>

#include "DrawCommand.hpp"
#include "RenderQueue.hpp"
#include "engine/audio/Volume.hpp"
#include "engine/core/Vec2.hpp"

namespace render_demo {
using engine::core::Vec2f;

constexpr int CELL = 32;
constexpr float SPRITE_SCALE = 2.0F;
const Vec2f LOGICAL_SIZE{1280.0F, 720.0F};

/*
** Cells of sprites.png, left to right.
*/
enum class Cell: std::uint8_t {
    SHIP,
    SHIP_THRUST,
    ENEMY,
    BULLET,
    TILE,
};

struct Ship {
    Vec2f position;
    bool facingLeft;
    bool thrusting;
};

struct SceneAssets {
    engine::render::TextureId sheet;
    engine::render::FontId font;
};

/*
** What the demo shows each frame, pushed deliberately out of layer order so
** the checklist can confirm sortByLayer did its job.
*/
struct Frame {
    const Ship &ship;
    const std::vector<Vec2f> &bullets;
    unsigned fired;
    const engine::audio::Volume &volumes;
};

void fillQueue(engine::render::RenderQueue &queue, const SceneAssets &assets,
    const Frame &frame);

}

#endif // RENDER_DEMO_SCENE_HPP
