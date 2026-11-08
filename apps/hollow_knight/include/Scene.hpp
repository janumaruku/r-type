/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight screens and presentation
*/

#ifndef HOLLOW_KNIGHT_SCENE_HPP
#define HOLLOW_KNIGHT_SCENE_HPP

#include <array>
#include <cstdint>

#include "engine/render/RenderQueue.hpp"
#include "World.hpp"

namespace hollow_knight {
enum class Screen {
    TITLE,
    OPTIONS,
    PLAY,
    PAUSE,
};

struct SpriteAsset {
    engine::render::TextureId texture;
    std::uint32_t width;
    std::uint32_t height;
};

struct SceneAssets {
    std::array<SpriteAsset, 3> knightFrames;
    SpriteAsset nailSlash;
    SpriteAsset healthMask;
    std::array<SpriteAsset, 3> enemies;
    SpriteAsset platformLedge;
    SpriteAsset platformStone;
    SpriteAsset blueGrassSmall;
    SpriteAsset blueGrassTall;
    SpriteAsset rootPlatform;
    SpriteAsset ceilingThorns;
    SpriteAsset blueVine;
};

void fillScene(engine::render::RenderQueue &queue, Screen screen,
    unsigned selection, float time, const SceneAssets &assets,
    const World &world);
} // namespace hollow_knight

#endif // HOLLOW_KNIGHT_SCENE_HPP
