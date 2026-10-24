/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** main
*/

#include <algorithm>

#include <raylib.h>

#include "World.hpp"
#include "engine/core/Rng.hpp"
#include "engine/ecs/Resources.hpp"

namespace {
using ecs_example::Controls;
using ecs_example::Game;
using ecs_example::Stage;
using ecs_example::Vec2f;

float axis(const int negative, const int alternative, const int positive,
    const int alternativePositive)
{
    const bool back = IsKeyDown(negative) || IsKeyDown(alternative);
    const bool forward = IsKeyDown(positive) || IsKeyDown(alternativePositive);

    return static_cast<float>(forward) - static_cast<float>(back);
}

Controls readControls()
{
    return Controls{
        .direction = Vec2f(axis(KEY_LEFT, KEY_A, KEY_RIGHT, KEY_D),
            axis(KEY_UP, KEY_W, KEY_DOWN, KEY_S)),
        .fire = IsKeyDown(KEY_SPACE),
    };
}

/*
** Input and frame time come from raylib once per frame and reach the systems
** as resources, so no gameplay system calls raylib itself.
*/
void runFrame(engine::ecs::Registry &registry)
{
    registry.getResource<engine::ecs::DeltaTime>().seconds =
        std::min(GetFrameTime(), ecs_example::MAX_FRAME_TIME);
    registry.getResource<Controls>() = readControls();

    if (!registry.getResource<Game>().over)
        registry.runStage(Stage::UPDATE);

    BeginDrawing();
    ClearBackground(Color{7, 12, 28, 255});
    registry.runStage(Stage::RENDER);
    EndDrawing();
}
}

int main()
{
    InitWindow(ecs_example::WIDTH, ecs_example::HEIGHT, "R-Type ECS example");
    SetTargetFPS(120);

    engine::ecs::Registry registry;
    ecs_example::buildWorld(registry, engine::core::Rng::randomSeed());

    while (!WindowShouldClose()) {
        if (registry.getResource<Game>().over && IsKeyPressed(KEY_R)) {
            registry = engine::ecs::Registry{};
            ecs_example::buildWorld(registry, engine::core::Rng::randomSeed());
        }
        runFrame(registry);
    }
    CloseWindow();
    return 0;
}
