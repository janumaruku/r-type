/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Components
*/

#ifndef R_TYPE_ECS_EXAMPLE_COMPONENTS_HPP
#define R_TYPE_ECS_EXAMPLE_COMPONENTS_HPP

#include <cstdint>

#include <raylib.h>

#include "engine/core/Vec2.hpp"

namespace ecs_example {
using engine::core::Vec2f;

enum class Stage: std::uint8_t {
    UPDATE,
    RENDER,
};

constexpr int WIDTH = 960;
constexpr int HEIGHT = 600;
constexpr float MAX_FRAME_TIME = 0.05F;

struct Position {
    Vec2f value;
};

struct Velocity {
    Vec2f value;
};

struct Radius {
    float value;
};

struct Tint {
    Color color;
};

struct Lifetime {
    float seconds;
};

struct Cooldown {
    float seconds;
};

struct Star {
    unsigned char brightness;
};

struct Player {};

struct Enemy {};

struct Bullet {};

struct Controls {
    Vec2f direction;
    bool fire;
};

struct Game {
    unsigned score;
    bool over;
    float nextEnemy;
};
}

#endif // R_TYPE_ECS_EXAMPLE_COMPONENTS_HPP
