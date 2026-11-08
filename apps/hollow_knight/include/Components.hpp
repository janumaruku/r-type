/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight demo components
*/

#ifndef HOLLOW_KNIGHT_COMPONENTS_HPP
#define HOLLOW_KNIGHT_COMPONENTS_HPP

#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "engine/core/Vec2.hpp"
#include "engine/ecs/Entity.hpp"

namespace hollow_knight {
using engine::core::Vec2f;
using engine::ecs::Entity;

struct Transform {
    Vec2f position;
};

struct Body {
    Vec2f velocity{0.0F, 0.0F};
    float halfWidth = 12.0F;
    float height    = 44.0F;
    bool grounded   = false;
};

enum class NailDirection : std::uint8_t {
    HORIZONTAL,
    UP,
    DOWN,
};

inline constexpr float NAIL_ATTACK_DURATION = 0.19F;
inline constexpr float HIT_EFFECT_LIFETIME  = 0.30F;
// The crop excludes transparent atlas padding so its AABB is the slash art.
inline constexpr int NAIL_SLASH_SOURCE_X      = 3;
inline constexpr int NAIL_SLASH_SOURCE_Y      = 3;
inline constexpr int NAIL_SLASH_SOURCE_WIDTH  = 201;
inline constexpr int NAIL_SLASH_SOURCE_HEIGHT = 86;

struct NailAttackPose {
    float centerX;
    float centerY;
    float scale;
    float rotation;
    float width;
    float height;
    bool flipX;
};

inline NailAttackPose nailAttackPose(const float playerX,
    const float playerBottom, const bool facingRight,
    const NailDirection direction, const float attackFor) noexcept
{
    float progress = 1.0F - attackFor / NAIL_ATTACK_DURATION;
    if (progress < 0.0F)
        progress = 0.0F;
    if (progress > 1.0F)
        progress = 1.0F;
    const float scale = 0.48F + progress * 0.12F;
    const float slashWidth =
        static_cast<float>(NAIL_SLASH_SOURCE_WIDTH) * scale;
    const float slashHeight =
        static_cast<float>(NAIL_SLASH_SOURCE_HEIGHT) * scale;

    switch (direction) {
    case NailDirection::HORIZONTAL: {
        const float facing = facingRight ? 1.0F : -1.0F;
        return NailAttackPose{playerX + facing * (52.0F + progress * 12.0F),
            playerBottom - 48.0F, scale, 180.0F, slashWidth, slashHeight,
            !facingRight};
    }
    case NailDirection::UP:
        return NailAttackPose{playerX, playerBottom - 58.0F, scale, 90.0F,
            slashHeight, slashWidth, false};
    case NailDirection::DOWN:
        return NailAttackPose{playerX, playerBottom - 7.0F, scale, -90.0F,
            slashHeight, slashWidth, false};
    }
    return NailAttackPose{playerX, playerBottom - 48.0F, scale, 0.0F,
        slashWidth, slashHeight, !facingRight};
}

struct Player {
    std::uint8_t health         = 5;
    std::uint8_t maxHealth      = 5;
    std::uint8_t soul           = 0;
    bool facingRight            = true;
    NailDirection nailDirection = NailDirection::HORIZONTAL;
    float invulnerableFor       = 0.0F;
    float attackFor             = 0.0F;
    float attackCooldown        = 0.0F;
    float pogoFor               = 0.0F;
    float dashFor               = 0.0F;
    float dashCooldown          = 0.0F;
    float focusFor              = 0.0F;
    std::uint32_t attackSerial  = 0;
    std::uint32_t hitSerial     = 0;
};

enum class EnemySprite : std::uint8_t {
    CRAWLER,
    INFECTED_HUSK,
    CROSSROADS_GUARD,
};

inline constexpr float enemyRenderScale(const EnemySprite sprite,
    const bool flying) noexcept
{
    if (flying && sprite == EnemySprite::CROSSROADS_GUARD)
        return 0.48F;
    switch (sprite) {
    case EnemySprite::CRAWLER:
        return 0.54F;
    case EnemySprite::INFECTED_HUSK:
        return 0.34F;
    case EnemySprite::CROSSROADS_GUARD:
        return 0.16F;
    }
    return 0.34F;
}

struct EnemyHitbox {
    float halfWidth;
    float height;
};

inline constexpr EnemyHitbox enemyHitbox(const EnemySprite sprite,
    const bool flying) noexcept
{
    const float scale = enemyRenderScale(sprite, flying);
    switch (sprite) {
    case EnemySprite::CRAWLER:
        return EnemyHitbox{110.0F * scale * 0.44F, 83.0F * scale * 0.90F};
    case EnemySprite::INFECTED_HUSK:
        return EnemyHitbox{126.0F * scale * 0.42F, 134.0F * scale * 0.92F};
    case EnemySprite::CROSSROADS_GUARD:
        return EnemyHitbox{415.0F * scale * (flying ? 0.40F : 0.41F),
            326.0F * scale * (flying ? 0.84F : 0.92F)};
    }
    return EnemyHitbox{18.0F, 40.0F};
}

struct Enemy {
    std::uint8_t health         = 2;
    float direction             = 1.0F;
    float hitFlash              = 0.0F;
    float hoverAnchor           = 0.0F;
    float hoverTime             = 0.0F;
    std::uint32_t lastHitSerial = 0;
    EnemySprite sprite          = EnemySprite::INFECTED_HUSK;
    bool flying                 = false;
};

struct NetIdentity {
    std::uint32_t value = 0;
};

struct PlayerInput {
    float horizontal            = 0.0F;
    NailDirection nailDirection = NailDirection::HORIZONTAL;
    bool jumpPressed            = false;
    bool attackPressed          = false;
    bool dashPressed            = false;
    bool focusHeld              = false;
    bool interactPressed        = false;
};

struct Platform {
    float x;
    float width;
    float top;
};

struct RoomGeometry {
    std::array<Platform, 4> platforms{{
        {276.0F, 174.0F, 492.0F},
        {494.0F, 186.0F, 398.0F},
        {708.0F, 190.0F, 304.0F},
        {926.0F, 180.0F, 398.0F},
    }};
};

struct EnemyStart {
    std::uint32_t netId;
    Vec2f position;
    float direction;
    EnemySprite sprite;
    std::uint8_t health = 2;
    bool flying         = false;
};

struct HitEffect {
    Vec2f position;
    float timeLeft = HIT_EFFECT_LIFETIME;
};

struct RoomState {
    Entity player = Entity::NONE;
    std::vector<EnemyStart> enemyStarts{
        {2, {425.0F, 578.0F}, -1.0F, EnemySprite::CRAWLER},
        {3, {746.0F, 578.0F}, 1.0F, EnemySprite::INFECTED_HUSK},
        {4, {850.0F, 390.0F}, -1.0F, EnemySprite::CROSSROADS_GUARD, 2, true},
    };
    std::vector<EnemyStart> checkpointEnemyStarts;
    std::vector<std::uint8_t> checkpointBytes;
    std::vector<HitEffect> hitEffects;
    std::uint64_t wave                     = 1;
    std::uint64_t checkpointWave           = 1;
    std::uint32_t nextEnemyNetId           = 5;
    std::uint32_t checkpointNextEnemyNetId = 5;
    std::uint32_t tick                     = 0;
    std::uint32_t kills                    = 0;
    std::uint32_t deaths                   = 0;
    float deathTimer                       = 0.0F;
    float messageTimer                     = 0.0F;
    float waveTransitionTimer              = 0.0F;
    std::string message;
};

enum class Stage : std::uint8_t {
    UPDATE,
};

inline constexpr float ROOM_LEFT   = 66.0F;
inline constexpr float ROOM_RIGHT  = 1214.0F;
inline constexpr float FLOOR_LEVEL = 584.0F;
inline constexpr float BENCH_X     = 196.0F;
} // namespace hollow_knight

#endif // HOLLOW_KNIGHT_COMPONENTS_HPP
