/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** RenderSystems
*/

#include <string>

#include <raylib.h>

#include "World.hpp"
#include "engine/ecs/SparseSet.hpp"

namespace ecs_example {
namespace {
using engine::ecs::SparseSet;

constexpr int FONT_SIZE = 20;

Vector2 toScreen(const Vec2f &point)
{
    return Vector2{point.x(), point.y()};
}

void drawCentered(const std::string &text, const int y, const int size)
{
    DrawText(text.c_str(), (WIDTH - MeasureText(text.c_str(), size)) / 2, y,
        size, RAYWHITE);
}

void addStarsSystem(Registry &registry)
{
    registry.addSystem<const Star, const Position>(Stage::RENDER, "stars",
        [](Registry &registry, const SparseSet<Star> &,
        const SparseSet<Position> &) {
            for (auto &&[star, position]:
                 registry.view<const Star, const Position>())
                DrawPixelV(toScreen(position.value), Color{.r = star.brightness,
                    .g = star.brightness, .b = star.brightness, .a = 255,});
        });
}

void addShapesSystems(Registry &registry)
{
    registry.addSystem<const Position, const Radius, const Tint>(
        Stage::RENDER, "circles",
        [](Registry &registry, const SparseSet<Position> &,
        const SparseSet<Radius> &, const SparseSet<Tint> &) {
            for (auto &&[position, radius, tint]: registry.view<
                     const Position, const Radius, const Tint>())
                DrawCircleV(toScreen(position.value), radius.value,
                    tint.color);
        });
    registry.addSystem<const Player, const Position, const Radius>(
        Stage::RENDER, "ship",
        [](Registry &registry, const SparseSet<Player> &,
        const SparseSet<Position> &, const SparseSet<Radius> &) {
            for (auto &&[player, position, radius]: registry.view<
                     const Player, const Position, const Radius>()) {
                const float x = position.value.x();
                const float y = position.value.y();
                const float r = radius.value;
                DrawTriangle(Vector2{.x = x + r, .y = y},
                    Vector2{.x = x - r, .y = y - r},
                    Vector2{.x = x - r, .y = y + r},
                    Color{.r = 60, .g = 210, .b = 255, .a = 255});
            }
        });
}

void addHudSystem(Registry &registry)
{
    registry.addSystem<>(Stage::RENDER, "hud", [](Registry &registry) {
        const Game &game = registry.getResource<Game>();

        DrawText(("Score " + std::to_string(game.score)).c_str(), 20, 16,
            FONT_SIZE, RAYWHITE);
        DrawText(("Entities " + std::to_string(registry.aliveCount())).c_str(),
            20, 40, FONT_SIZE, GRAY);
        DrawFPS(WIDTH - 100, 16);
        if (!game.over)
            return;
        DrawRectangle(0, 0, WIDTH, HEIGHT,
            Color{.r = 0, .g = 0, .b = 0, .a = 150});
        drawCentered("GAME OVER", (HEIGHT / 2) - 40, 2 * FONT_SIZE);
        drawCentered("Press R to restart", (HEIGHT / 2) + 10, FONT_SIZE);
    });
}
}

void addRenderSystems(Registry &registry)
{
    addStarsSystem(registry);
    addShapesSystems(registry);
    addHudSystem(registry);
}
}
