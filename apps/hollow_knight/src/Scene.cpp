/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight screens and presentation
*/

#include "Scene.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <string>
#include <string_view>
#include <utility>

#include "Components.hpp"
#include "engine/core/Color.hpp"
#include "engine/core/Rect2.hpp"
#include "engine/render/DrawCommand.hpp"

namespace hollow_knight {
namespace {
using engine::core::Color;
using engine::core::FloatRect;
using engine::core::Vec2f;
using engine::render::RenderQueue;
using engine::render::SpriteCmd;
using engine::render::TextAlign;

constexpr std::int16_t BACKDROP_LAYER = 0;
constexpr std::int16_t CAVE_LAYER     = 10;
constexpr std::int16_t ROOM_LAYER     = 20;
constexpr std::int16_t ACTOR_LAYER    = 100;
constexpr std::int16_t HUD_LAYER      = 1000;
constexpr float SCREEN_WIDTH          = 1280.0F;
constexpr float SCREEN_HEIGHT         = 720.0F;
const Color NO_OUTLINE{0, 0, 0, 0};

void rect(RenderQueue &queue, const float x, const float y, const float width,
    const float height, const Color fill, const std::int16_t layer)
{
    queue.push(engine::render::RectCmd{
        .rect             = FloatRect{Vec2f{x, y}, width, height},
        .fill             = fill,
        .outline          = NO_OUTLINE,
        .outlineThickness = 0.0F,
        .layer            = layer});
}

void text(RenderQueue &queue, std::string value, const float x, const float y,
    const std::uint8_t size, const Color color,
    const TextAlign align    = TextAlign::LEFT,
    const std::int16_t layer = HUD_LAYER)
{
    queue.push(engine::render::TextCmd{.font = {},
        .text                                = std::move(value),
        .characterSize                       = size,
        .position                            = Vec2f{x, y},
        .color                               = color,
        .layer                               = layer,
        .align                               = align});
}

void sprite(RenderQueue &queue, const SpriteAsset &asset, const float centerX,
    const float bottomY, const float scale, const bool flip,
    const std::int16_t layer, const Color tint = Color{255, 255, 255, 255})
{
    const auto width    = static_cast<std::int32_t>(asset.width);
    const auto height   = static_cast<std::int32_t>(asset.height);
    const float widthF  = static_cast<float>(asset.width);
    const float heightF = static_cast<float>(asset.height);
    queue.push(SpriteCmd{.texture = asset.texture,
        .source =
            engine::core::IntRect{engine::core::Vec2i{flip ? width : 0, 0},
                flip ? -width : width, height},
        .position    = Vec2f{centerX, bottomY},
        .origin      = Vec2f{widthF / 2.0F, heightF},
        .scale       = Vec2f{scale, scale},
        .rotationDeg = 0.0F,
        .tint        = tint,
        .layer       = layer});
}

void centeredSprite(RenderQueue &queue, const SpriteAsset &asset,
    const float centerX, const float centerY, const float scaleX,
    const float scaleY, const float rotation, const std::int16_t layer,
    const Color tint = Color{255, 255, 255, 255})
{
    const auto width    = static_cast<std::int32_t>(asset.width);
    const auto height   = static_cast<std::int32_t>(asset.height);
    const float widthF  = static_cast<float>(asset.width);
    const float heightF = static_cast<float>(asset.height);
    queue.push(SpriteCmd{.texture = asset.texture,
        .source =
            engine::core::IntRect{engine::core::Vec2i{0, 0}, width, height},
        .position    = Vec2f{centerX, centerY},
        .origin      = Vec2f{widthF / 2.0F, heightF / 2.0F},
        .scale       = Vec2f{scaleX, scaleY},
        .rotationDeg = rotation,
        .tint        = tint,
        .layer       = layer});
}

void nailSlashSprite(RenderQueue &queue, const SpriteAsset &asset,
    const NailAttackPose &pose, const std::int16_t layer, const Color tint)
{
    const int sourceWidth =
        pose.flipX ? -NAIL_SLASH_SOURCE_WIDTH : NAIL_SLASH_SOURCE_WIDTH;
    const int sourceX =
        NAIL_SLASH_SOURCE_X + (pose.flipX ? NAIL_SLASH_SOURCE_WIDTH : 0);
    const float width  = static_cast<float>(NAIL_SLASH_SOURCE_WIDTH);
    const float height = static_cast<float>(NAIL_SLASH_SOURCE_HEIGHT);
    queue.push(SpriteCmd{.texture = asset.texture,
        .source      = engine::core::IntRect{engine::core::Vec2i{
                                                 sourceX, NAIL_SLASH_SOURCE_Y},
            sourceWidth, NAIL_SLASH_SOURCE_HEIGHT},
        .position    = Vec2f{pose.centerX, pose.centerY},
        .origin      = Vec2f{width / 2.0F, height / 2.0F},
        .scale       = Vec2f{pose.scale, pose.scale},
        .rotationDeg = pose.rotation,
        .tint        = tint,
        .layer       = layer});
}

void particle(RenderQueue &queue, const float x, const float y,
    const float size, const Color color, const std::int16_t layer)
{
    const auto haloAlpha =
        static_cast<std::uint8_t>(static_cast<int>(color.a) * 24 / 100);
    const auto softAlpha =
        static_cast<std::uint8_t>(static_cast<int>(color.a) * 58 / 100);
    rect(queue, x - size * 1.6F, y - size * 1.6F, size * 3.2F, size * 3.2F,
        Color{color.r, color.g, color.b, haloAlpha}, layer);
    rect(queue, x - size * 0.75F, y - size * 0.75F, size * 1.5F, size * 1.5F,
        Color{color.r, color.g, color.b, softAlpha}, layer + 1);
    rect(queue, x - size * 0.35F, y - size * 0.35F, size * 0.7F, size * 0.7F,
        color, layer + 2);
}

void drawKnight(RenderQueue &queue, const SceneAssets &assets, const float x,
    const float bottom, const bool facingRight, const float attackFor,
    const float focusFor, const NailDirection nailDirection,
    const std::int16_t layer, const float time, const Body *body = nullptr)
{
    if (focusFor > 0.0F) {
        const std::uint8_t alpha = static_cast<std::uint8_t>(45 +
            static_cast<int>(std::fmod(focusFor * 190.0F, 55.0F)));
        rect(queue, x - 36.0F, bottom - 76.0F, 72.0F, 76.0F,
            Color{91, 196, 224, alpha}, layer - 1);
    }
    const bool walking = body != nullptr && body->grounded &&
        std::abs(body->velocity.x()) > 12.0F;
    const float framesPerSecond = walking ? 9.0F : 2.3F;
    const std::size_t frame = static_cast<std::size_t>(time * framesPerSecond) %
        assets.knightFrames.size();
    const float step       = walking ? std::sin(time * 18.0F) : 0.0F;
    const float drawBottom = bottom - std::max(0.0F, step) * 1.5F;
    sprite(queue, assets.knightFrames[frame], x, drawBottom, 0.55F, facingRight,
        layer);

    if (attackFor > 0.0F) {
        const float progress = 1.0F - attackFor / NAIL_ATTACK_DURATION;
        const auto alpha     = static_cast<std::uint8_t>(std::max(45,
            static_cast<int>(255.0F * (1.0F - progress * 0.72F))));
        const NailAttackPose pose =
            nailAttackPose(x, bottom, facingRight, nailDirection, attackFor);
        nailSlashSprite(queue, assets.nailSlash, pose, layer + 3,
            Color{255, 255, 255, alpha});
    }
}

void drawEnemy(RenderQueue &queue, const SceneAssets &assets,
    const Enemy &enemy, const Transform &transform, const Body &body,
    const std::int16_t layer, const float time)
{
    const std::size_t spriteIndex = static_cast<std::size_t>(enemy.sprite);
    const SpriteAsset &asset      = assets.enemies[spriteIndex];
    const float scale      = enemyRenderScale(enemy.sprite, enemy.flying);
    const Color tint       = enemy.hitFlash > 0.0F ? Color{255, 219, 190, 255}
                                                   : Color{255, 255, 255, 255};
    const bool moving      = std::abs(body.velocity.x()) > 8.0F;
    const float cycleSpeed = enemy.flying ? 6.2F
        : moving ? (enemy.sprite == EnemySprite::CRAWLER ? 10.5F : 7.2F)
                 : 2.0F;
    const float phase  = time * cycleSpeed + transform.position.x() * 0.019F;
    const float motion = std::sin(phase);
    float bob          = 0.0F;
    float rotation     = 0.0F;
    float squash       = 0.0F;
    if (enemy.flying) {
        // The guard's broad wings read best with a strong flap and hover arc.
        bob      = 6.0F + motion * 7.0F;
        rotation = motion * 5.5F;
        squash   = motion * 0.105F;
    } else if (enemy.sprite == EnemySprite::CRAWLER) {
        // The crawler hugs the floor and undulates as it scuttles.
        bob      = std::abs(motion) * 2.5F;
        rotation = motion * 5.5F;
        squash   = motion * 0.10F;
    } else {
        // Husk steps alternate between a small lift and a planted crouch.
        bob      = std::max(0.0F, motion) * 5.0F;
        rotation = motion * 4.0F;
        squash   = motion * 0.065F;
    }
    const float scaleX  = scale * (1.0F + squash);
    const float scaleY  = scale * (1.0F - squash);
    const auto width    = static_cast<std::int32_t>(asset.width);
    const auto height   = static_cast<std::int32_t>(asset.height);
    const float widthF  = static_cast<float>(asset.width);
    const float heightF = static_cast<float>(asset.height);
    queue.push(SpriteCmd{.texture = asset.texture,
        .source =
            engine::core::IntRect{
                engine::core::Vec2i{enemy.direction < 0.0F ? width : 0, 0},
                enemy.direction < 0.0F ? -width : width, height},
        .position = Vec2f{transform.position.x(), transform.position.y() - bob},
        .origin   = Vec2f{widthF / 2.0F, heightF},
        .scale    = Vec2f{scaleX, scaleY},
        .rotationDeg = rotation,
        .tint        = tint,
        .layer       = layer});
    if (!enemy.flying && moving && motion < -0.78F) {
        const float dustX = transform.position.x() - enemy.direction * 11.0F;
        const auto alpha  = static_cast<std::uint8_t>(110 +
            static_cast<int>((-motion - 0.78F) * 350.0F));
        rect(queue, dustX, transform.position.y() - 3.0F, 4.0F, 3.0F,
            Color{111, 145, 157, alpha}, layer - 1);
        rect(queue, dustX - enemy.direction * 5.0F,
            transform.position.y() + 1.0F, 3.0F, 2.0F,
            Color{72, 107, 124, alpha}, layer - 1);
    }
    const float halfWidth    = static_cast<float>(asset.width) * scale / 2.0F;
    const float visualHeight = static_cast<float>(asset.height) * scale;
    for (std::uint8_t point = 0; point < enemy.health; ++point)
        rect(queue,
            transform.position.x() - halfWidth + 4.0F +
                static_cast<float>(point) * 8.0F,
            transform.position.y() - bob - visualHeight - 9.0F, 5.0F, 2.0F,
            Color{225, 151, 93, 225}, layer + 2);
}

void drawActorParticles(RenderQueue &queue, const float time,
    const World &world)
{
    const auto &registry = world.registry();
    for (auto &&[player, transform, body]:
        registry.view<const Player, const Transform, const Body>()) {
        const float x              = transform.position.x();
        const float bottom         = transform.position.y();
        const float trailDirection = player.facingRight ? -1.0F : 1.0F;
        if (body.grounded && std::abs(body.velocity.x()) > 35.0F) {
            for (int index = 0; index < 4; ++index) {
                const float phase =
                    std::fmod(time * 3.6F + static_cast<float>(index) * 0.25F,
                        1.0F);
                const int alpha = static_cast<int>((1.0F - phase) * 165.0F);
                particle(queue,
                    x + trailDirection * (8.0F + phase * 33.0F) +
                        std::sin(time * 7.0F + static_cast<float>(index)) *
                            3.0F,
                    bottom - 2.0F + phase * 8.0F, 2.0F + phase,
                    Color{130, 191, 202, static_cast<std::uint8_t>(alpha)},
                    ACTOR_LAYER - 3);
            }
        }

        if (player.dashFor > 0.0F) {
            for (int index = 0; index < 7; ++index) {
                const float phase = static_cast<float>(index) / 7.0F;
                const auto alpha =
                    static_cast<std::uint8_t>(static_cast<int>((1.0F - phase) *
                        205.0F));
                const float wobble =
                    std::sin(time * 44.0F + phase * 9.0F) * 5.0F;
                particle(queue, x + trailDirection * (15.0F + phase * 70.0F),
                    bottom - 31.0F + wobble, 2.0F + phase * 2.0F,
                    Color{114, 211, 230, alpha}, ACTOR_LAYER + 5);
            }
        }

        if (player.focusFor > 0.0F) {
            for (int index = 0; index < 7; ++index) {
                const float angle =
                    time * 4.7F + static_cast<float>(index) * 6.2831853F / 7.0F;
                const float pulse = 1.0F +
                    std::sin(time * 9.0F + static_cast<float>(index)) * 2.5F;
                particle(queue, x + std::cos(angle) * (17.0F + pulse),
                    bottom - 37.0F + std::sin(angle) * 24.0F, 2.6F,
                    Color{117, 214, 233, 210}, ACTOR_LAYER + 11);
            }
        }

        if (player.attackFor > 0.0F) {
            const float progress =
                1.0F - player.attackFor / NAIL_ATTACK_DURATION;
            const bool horizontal =
                player.nailDirection == NailDirection::HORIZONTAL;
            const float direction = horizontal
                ? (player.facingRight ? 1.0F : -1.0F)
                : (player.nailDirection == NailDirection::UP ? -1.0F : 1.0F);
            for (int index = 0; index < 6; ++index) {
                const float phase   = static_cast<float>(index) / 5.0F;
                const float offset  = 36.0F + progress * 28.0F + phase * 30.0F;
                const float flicker = std::sin(time * 58.0F + phase * 10.0F);
                const float particleX = horizontal
                    ? x + direction * offset
                    : x + flicker * (7.0F + phase * 7.0F);
                const float particleY = horizontal
                    ? bottom - 38.0F + flicker * (7.0F + phase * 7.0F)
                    : bottom +
                        direction * (24.0F + progress * 26.0F + phase * 32.0F);
                particle(queue, particleX, particleY, 1.5F + phase * 1.2F,
                    Color{207, 241, 247,
                        static_cast<std::uint8_t>(150 +
                            static_cast<int>((1.0F - phase) * 90.0F))},
                    ACTOR_LAYER + 18);
            }
        }

        if (player.pogoFor > 0.0F) {
            const float strength = player.pogoFor / 0.18F;
            for (int index = 0; index < 8; ++index) {
                const float angle =
                    static_cast<float>(index) * 0.7853982F + time * 3.0F;
                const float radius = 11.0F + (1.0F - strength) * 16.0F;
                particle(queue, x + std::cos(angle) * radius,
                    bottom - 3.0F + std::sin(angle) * 7.0F, 1.7F + strength,
                    Color{225, 250, 243,
                        static_cast<std::uint8_t>(strength * 220.0F)},
                    ACTOR_LAYER + 19);
            }
        }
    }

    for (auto &&[enemy, transform]:
        registry.view<const Enemy, const Transform>()) {
        if (enemy.hitFlash <= 0.0F)
            continue;
        const float intensity = std::clamp(enemy.hitFlash / 0.17F, 0.0F, 1.0F);
        for (int index = 0; index < 5; ++index) {
            const float angle =
                time * 18.0F + static_cast<float>(index) * 6.2831853F / 5.0F;
            particle(queue,
                transform.position.x() +
                    std::cos(angle) * (14.0F + index * 2.0F),
                transform.position.y() - 27.0F + std::sin(angle) * 15.0F, 1.8F,
                Color{255, 207, 160,
                    static_cast<std::uint8_t>(intensity * 220.0F)},
                ACTOR_LAYER + 4);
        }
    }

    for (const HitEffect &effect: world.state().hitEffects) {
        const float progress =
            std::clamp(1.0F - effect.timeLeft / HIT_EFFECT_LIFETIME, 0.0F,
                1.0F);
        const float fade     = 1.0F - progress;
        const auto alpha     = static_cast<std::uint8_t>(255.0F * fade);
        const float coreSize = 9.0F * fade;
        const float x        = effect.position.x();
        const float y        = effect.position.y();

        particle(queue, x, y, 5.0F * fade, Color{255, 245, 218, alpha},
            ACTOR_LAYER + 5);
        rect(queue, x - coreSize, y - 1.3F, coreSize * 2.0F, 2.6F,
            Color{255, 239, 204, alpha}, ACTOR_LAYER + 8);
        rect(queue, x - 1.3F, y - coreSize, 2.6F, coreSize * 2.0F,
            Color{255, 239, 204, alpha}, ACTOR_LAYER + 8);

        for (std::size_t index = 0; index < 8; ++index) {
            const float angle =
                static_cast<float>(index) * 0.785398F + progress * 0.35F;
            const float distance = 7.0F + progress * 27.0F;
            const float sparkX   = x + std::cos(angle) * distance;
            const float sparkY   = y + std::sin(angle) * distance;
            const Color spark    = index % 3 == 0 ? Color{255, 177, 119, alpha}
                                                  : Color{139, 225, 231, alpha};
            particle(queue, sparkX, sparkY,
                (2.0F + static_cast<float>(index % 2)) * fade, spark,
                ACTOR_LAYER + 9);
        }
    }
}

void drawRoom(RenderQueue &queue, const float time, const SceneAssets &assets,
    const RoomGeometry &geometry)
{
    rect(queue, 0.0F, 0.0F, SCREEN_WIDTH, SCREEN_HEIGHT, Color{3, 11, 24, 255},
        BACKDROP_LAYER);
    rect(queue, 0.0F, 170.0F, SCREEN_WIDTH, 330.0F, Color{4, 23, 37, 160},
        BACKDROP_LAYER + 1);

    static constexpr std::array<float, 5> LIGHT_SHAFTS{
        169.0F, 423.0F, 701.0F, 927.0F, 1156.0F};
    for (std::size_t index = 0; index < LIGHT_SHAFTS.size(); ++index) {
        const float x = LIGHT_SHAFTS[index] +
            std::sin(time * 0.15F + static_cast<float>(index)) * 12.0F;
        rect(queue, x, 82.0F, 46.0F, 444.0F, Color{15, 100, 121, 20},
            BACKDROP_LAYER + 2);
        particle(queue, x + 23.0F, 366.0F, 55.0F, Color{28, 151, 172, 34},
            BACKDROP_LAYER + 3);
    }

    static constexpr std::array<float, 6> VINE_X{
        50.0F, 286.0F, 493.0F, 756.0F, 1002.0F, 1220.0F};
    for (std::size_t index = 0; index < VINE_X.size(); ++index) {
        const float scale = 0.82F + static_cast<float>(index % 3) * 0.16F;
        centeredSprite(queue, assets.blueVine, VINE_X[index], 170.0F, 0.76F,
            scale, (index % 2U == 0U ? -1.0F : 1.0F) * 2.0F, CAVE_LAYER,
            Color{35, 107, 128, 115});
    }

    centeredSprite(queue, assets.platformStone, 111.0F, 393.0F, 1.56F, 0.98F,
        -2.0F, CAVE_LAYER + 1, Color{44, 84, 104, 92});
    centeredSprite(queue, assets.platformStone, 1184.0F, 320.0F, 1.48F, 0.88F,
        3.0F, CAVE_LAYER + 1, Color{37, 79, 99, 100});
    centeredSprite(queue, assets.rootPlatform, 941.0F, 382.0F, 1.08F, 0.78F,
        0.0F, CAVE_LAYER + 2, Color{52, 121, 124, 90});

    for (int mote = 0; mote < 42; ++mote) {
        const float seed = static_cast<float>(mote);
        const float x    = std::fmod(24.0F + seed * 109.0F +
                std::sin(time * 0.43F + seed * 1.7F) * (9.0F + mote % 7),
            SCREEN_WIDTH);
        const float y =
            76.0F + std::fmod(seed * 53.0F + time * (7.0F + mote % 5), 474.0F);
        const float twinkle =
            0.5F + 0.5F * std::sin(time * (2.3F + mote % 4) + seed * 2.1F);
        const auto alpha = static_cast<std::uint8_t>(35.0F + twinkle * 115.0F);
        particle(queue, x, y, 1.1F + static_cast<float>(mote % 3) * 0.65F,
            Color{122, 208, 211, alpha}, CAVE_LAYER + 3);
    }

    for (std::size_t index = 0; index < geometry.platforms.size(); ++index) {
        const Platform &platform = geometry.platforms[index];
        const float scale =
            platform.width / static_cast<float>(assets.platformLedge.width);
        const float centerX = platform.x + platform.width / 2.0F;
        const float centerY = platform.top + 61.0F * scale;
        if (index % 2U == 0U)
            centeredSprite(queue, assets.rootPlatform, centerX,
                platform.top + 45.0F,
                platform.width / static_cast<float>(assets.rootPlatform.width),
                scale * 0.92F, 0.0F, CAVE_LAYER + 4, Color{87, 162, 162, 188});
        centeredSprite(queue, assets.blueVine, centerX, platform.top + 91.0F,
            0.48F, 0.34F, 0.0F, CAVE_LAYER + 4, Color{53, 119, 137, 170});
        centeredSprite(queue, assets.platformLedge, centerX, centerY, scale,
            scale, 0.0F, ROOM_LAYER);
        sprite(queue, assets.blueGrassTall, platform.x + 24.0F,
            platform.top + 5.0F, 0.46F, false, ROOM_LAYER + 1,
            Color{168, 240, 216, 255});
        sprite(queue, assets.blueGrassSmall,
            platform.x + platform.width - 25.0F, platform.top + 4.0F, 0.52F,
            true, ROOM_LAYER + 1, Color{177, 243, 213, 255});
    }

    rect(queue, 0.0F, FLOOR_LEVEL + 7.0F, SCREEN_WIDTH,
        SCREEN_HEIGHT - FLOOR_LEVEL, Color{5, 14, 24, 255}, ROOM_LAYER - 1);
    constexpr float FLOOR_SCALE = 0.82F;
    for (int tile = 0; tile < 4; ++tile) {
        const float centerX = 163.0F + static_cast<float>(tile) * 318.0F;
        const float centerY = FLOOR_LEVEL + 61.0F * FLOOR_SCALE;
        centeredSprite(queue, assets.platformLedge, centerX, centerY,
            FLOOR_SCALE, FLOOR_SCALE, 0.0F, ROOM_LAYER);
    }

    static constexpr std::array<float, 6> GRASS_X{
        103.0F, 351.0F, 585.0F, 811.0F, 1006.0F, 1174.0F};
    for (std::size_t index = 0; index < GRASS_X.size(); ++index) {
        const SpriteAsset &grass =
            index % 2U == 0U ? assets.blueGrassSmall : assets.blueGrassTall;
        const float scale = index % 2U == 0U ? 0.58F : 0.64F;
        sprite(queue, grass, GRASS_X[index], FLOOR_LEVEL + 4.0F, scale,
            index % 3U == 0U, ROOM_LAYER + 1, Color{143, 211, 190, 245});
    }

    for (int edge = 0; edge < 2; ++edge) {
        const float x        = edge == 0 ? 25.0F : 1255.0F;
        const float rotation = edge == 0 ? -90.0F : 90.0F;
        centeredSprite(queue, assets.platformStone, x, 514.0F, 0.70F, 0.88F,
            rotation, ROOM_LAYER + 2, Color{126, 158, 168, 255});
        centeredSprite(queue, assets.platformStone, x, 443.0F, 0.57F, 0.78F,
            rotation, ROOM_LAYER + 2, Color{90, 130, 145, 255});
    }

    for (int section = 0; section < 4; ++section) {
        const float x = 160.0F + static_cast<float>(section) * 320.0F;
        centeredSprite(queue, assets.ceilingThorns, x, 24.0F, 2.5F, 2.25F, 0.0F,
            ROOM_LAYER + 3);
    }
    for (int section = 0; section < 4; ++section) {
        const float x = 160.0F + static_cast<float>(section) * 320.0F;
        centeredSprite(queue, assets.ceilingThorns, x, 699.0F, 2.7F, -2.5F,
            0.0F, ROOM_LAYER + 3, Color{118, 145, 157, 255});
    }

    rect(queue, BENCH_X - 35.0F, FLOOR_LEVEL - 32.0F, 69.0F, 8.0F,
        Color{90, 97, 101, 255}, ROOM_LAYER + 4);
    rect(queue, BENCH_X - 27.0F, FLOOR_LEVEL - 24.0F, 7.0F, 22.0F,
        Color{54, 62, 70, 255}, ROOM_LAYER + 4);
    rect(queue, BENCH_X + 11.0F, FLOOR_LEVEL - 24.0F, 7.0F, 22.0F,
        Color{54, 62, 70, 255}, ROOM_LAYER + 4);
    rect(queue, BENCH_X - 46.0F, FLOOR_LEVEL - 40.0F, 9.0F, 11.0F,
        Color{113, 169, 185, 150}, ROOM_LAYER + 3);
}

void drawMask(RenderQueue &queue, const SpriteAsset &mask, const float x,
    const float y, const bool filled)
{
    const Color color =
        filled ? Color{225, 232, 235, 255} : Color{79, 94, 105, 255};
    sprite(queue, mask, x + 12.0F, y + 29.0F, 1.0F, false, HUD_LAYER, color);
}

void drawHud(RenderQueue &queue, const SceneAssets &assets, const World &world)
{
    const Player &player   = world.player();
    const RoomState &state = world.state();
    const std::size_t enemyCount =
        world.registry().getComponents<Enemy>().size();
    rect(queue, 24.0F, 22.0F, 230.0F, 72.0F, Color{7, 12, 24, 205},
        HUD_LAYER - 1);
    for (std::uint8_t mask = 0; mask < player.maxHealth; ++mask)
        drawMask(queue, assets.healthMask,
            37.0F + static_cast<float>(mask) * 34.0F, 35.0F,
            mask < player.health);

    rect(queue, 263.0F, 28.0F, 46.0F, 56.0F, Color{7, 12, 24, 205},
        HUD_LAYER - 1);
    rect(queue, 276.0F, 36.0F, 20.0F, 38.0F, Color{78, 111, 125, 255},
        HUD_LAYER);
    rect(queue, 280.0F, 40.0F, 12.0F, 30.0F, Color{11, 22, 35, 255},
        HUD_LAYER + 1);
    for (std::uint8_t shard = 0; shard < player.soul / 33; ++shard)
        rect(queue, 282.0F, 63.0F - static_cast<float>(shard) * 9.0F, 8.0F,
            6.0F, Color{111, 208, 226, 255}, HUD_LAYER + 2);
    if (player.soul % 33 > 0)
        rect(queue, 282.0F, 66.0F, 8.0F,
            4.0F * static_cast<float>(player.soul % 33) / 33.0F,
            Color{111, 208, 226, 255}, HUD_LAYER + 2);

    rect(queue, 570.0F, 22.0F, 140.0F, 68.0F, Color{7, 12, 24, 205},
        HUD_LAYER - 1);
    text(queue, "WAVE " + std::to_string(state.wave), 640.0F, 31.0F, 19,
        Color{222, 233, 235, 255}, TextAlign::CENTER);
    text(queue, std::to_string(enemyCount) + " ENEMIES", 640.0F, 60.0F, 13,
        Color{133, 178, 191, 255}, TextAlign::CENTER);

    text(queue, "GEO", 1080.0F, 29.0F, 17, Color{151, 169, 177, 255},
        TextAlign::CENTER);
    text(queue, std::to_string(37U + state.kills * 5U), 1109.0F, 26.0F, 24,
        Color{226, 233, 233, 255});
    text(queue, "MAP", 1204.0F, 31.0F, 15, Color{126, 150, 160, 255},
        TextAlign::CENTER);
    rect(queue, 1210.0F, 29.0F, 30.0F, 24.0F, Color{37, 55, 67, 255},
        HUD_LAYER - 1);
    rect(queue, 1215.0F, 34.0F, 20.0F, 14.0F, Color{13, 24, 37, 255},
        HUD_LAYER);
    rect(queue, 1220.0F, 38.0F, 4.0F, 6.0F, Color{140, 189, 190, 255},
        HUD_LAYER + 1);

    rect(queue, 24.0F, 642.0F, 1232.0F, 50.0F, Color{5, 10, 20, 192},
        HUD_LAYER - 1);
    text(queue, "FORGOTTEN CROSSROADS", 44.0F, 653.0F, 20,
        Color{196, 208, 211, 255});
    text(queue, "A / D MOVE    SPACE JUMP    J / CLICK NAIL    UP / DOWN AIM",
        438.0F, 654.0F, 14, Color{141, 162, 171, 255});
    text(queue, "F FOCUS    E REST", 1010.0F, 654.0F, 16,
        Color{141, 162, 171, 255});

    if (state.messageTimer > 0.0F)
        text(queue, state.message, SCREEN_WIDTH / 2.0F, 591.0F, 23,
            Color{201, 225, 226, 255}, TextAlign::CENTER);
}

void drawActors(RenderQueue &queue, const SceneAssets &assets,
    const World &world, const float time)
{
    const auto &registry = world.registry();
    for (auto &&[enemy, transform, body]:
        registry.view<const Enemy, const Transform, const Body>()) {
        drawEnemy(queue, assets, enemy, transform, body, ACTOR_LAYER, time);
    }
    for (auto &&[player, transform, body]:
        registry.view<const Player, const Transform, const Body>()) {
        if (player.invulnerableFor > 0.0F &&
            static_cast<int>(player.invulnerableFor * 18.0F) % 2 == 0)
            continue;
        drawKnight(queue, assets, transform.position.x(),
            transform.position.y(), player.facingRight, player.attackFor,
            player.focusFor, player.nailDirection, ACTOR_LAYER + 10, time,
            &body);
    }
}

void drawRoomTitle(RenderQueue &queue)
{
    text(queue, "DIRTMOUTH", 640.0F, 105.0F, 16, Color{114, 143, 158, 255},
        TextAlign::CENTER);
    text(queue, "FORGOTTEN CROSSROADS", 640.0F, 132.0F, 27,
        Color{183, 204, 210, 255}, TextAlign::CENTER);
    rect(queue, 559.0F, 171.0F, 162.0F, 2.0F, Color{86, 125, 140, 175},
        HUD_LAYER - 1);
}

void drawTitleScreen(RenderQueue &queue, const unsigned selection,
    const float time, const SceneAssets &assets)
{
    drawRoom(queue, time, assets, RoomGeometry{});
    rect(queue, 0.0F, 0.0F, SCREEN_WIDTH, SCREEN_HEIGHT, Color{1, 7, 17, 132},
        ROOM_LAYER + 5);

    text(queue, "A KINGDOM BELOW", 640.0F, 91.0F, 17, Color{111, 164, 184, 255},
        TextAlign::CENTER);
    text(queue, "HOLLOW", 642.0F, 131.0F, 64, Color{53, 118, 142, 175},
        TextAlign::CENTER, HUD_LAYER - 1);
    text(queue, "HOLLOW", 640.0F, 127.0F, 64, Color{226, 233, 236, 255},
        TextAlign::CENTER);
    text(queue, "KNIGHT", 642.0F, 190.0F, 57, Color{53, 118, 142, 175},
        TextAlign::CENTER, HUD_LAYER - 1);
    text(queue, "KNIGHT", 640.0F, 186.0F, 57, Color{226, 233, 236, 255},
        TextAlign::CENTER);
    rect(queue, 514.0F, 256.0F, 252.0F, 1.0F, Color{89, 131, 148, 160},
        HUD_LAYER - 1);

    static constexpr std::array<std::string_view, 3> OPTIONS{
        "START GAME", "CONTROLS", "QUIT"};
    for (unsigned index = 0; index < OPTIONS.size(); ++index) {
        const float y       = 292.0F + static_cast<float>(index) * 43.0F;
        const bool selected = selection == index;
        const Color color =
            selected ? Color{235, 242, 241, 255} : Color{132, 151, 160, 255};
        if (selected) {
            rect(queue, 490.0F, y + 12.0F, 15.0F, 3.0F,
                Color{121, 195, 207, 255}, HUD_LAYER - 1);
            rect(queue, 775.0F, y + 12.0F, 15.0F, 3.0F,
                Color{121, 195, 207, 255}, HUD_LAYER - 1);
        }
        text(queue, std::string{OPTIONS[index]}, 640.0F, y, 22, color,
            TextAlign::CENTER);
    }

    drawKnight(queue, assets, 640.0F, FLOOR_LEVEL, true, 0.0F, 0.0F,
        NailDirection::HORIZONTAL, ACTOR_LAYER, time);
    text(queue, "W / S SELECT     ENTER DESCEND", 640.0F, 645.0F, 15,
        Color{116, 146, 159, 255}, TextAlign::CENTER);
    text(queue, "ONE ROOM DEMO  ·  BUILT ON THE R-TYPE ENGINE", 640.0F, 680.0F,
        12, Color{74, 101, 116, 255}, TextAlign::CENTER);
}

void drawOptionsScreen(RenderQueue &queue, const float time,
    const SceneAssets &assets)
{
    drawRoom(queue, time, assets, RoomGeometry{});
    rect(queue, 0.0F, 0.0F, SCREEN_WIDTH, SCREEN_HEIGHT, Color{1, 7, 17, 156},
        ROOM_LAYER + 5);
    text(queue, "CONTROLS", 640.0F, 95.0F, 41, Color{225, 233, 235, 255},
        TextAlign::CENTER);
    text(queue, "A / D     OR     LEFT / RIGHT", 640.0F, 175.0F, 21,
        Color{164, 187, 194, 255}, TextAlign::CENTER);
    text(queue, "MOVE THROUGH THE CAVERN", 640.0F, 205.0F, 15,
        Color{110, 138, 151, 255}, TextAlign::CENTER);
    text(queue, "SPACE", 640.0F, 265.0F, 21, Color{164, 187, 194, 255},
        TextAlign::CENTER);
    text(queue, "JUMP WITH SPACE", 640.0F, 295.0F, 15,
        Color{110, 138, 151, 255}, TextAlign::CENTER);
    text(queue, "J  /  LEFT CLICK", 640.0F, 355.0F, 21,
        Color{164, 187, 194, 255}, TextAlign::CENTER);
    text(queue, "UP / DOWN AIM  ·  DOWN STRIKE POGOS ON HIT", 640.0F, 385.0F,
        15, Color{110, 138, 151, 255}, TextAlign::CENTER);
    text(queue, "SHIFT     HOLD F", 640.0F, 445.0F, 21,
        Color{164, 187, 194, 255}, TextAlign::CENTER);
    text(queue, "DASH     FOCUS TO HEAL WITH SOUL", 640.0F, 475.0F, 15,
        Color{110, 138, 151, 255}, TextAlign::CENTER);
    text(queue, "E", 640.0F, 535.0F, 21, Color{164, 187, 194, 255},
        TextAlign::CENTER);
    text(queue, "REST AT THE BENCH TO RESTORE MASKS AND THE ROOM", 640.0F,
        565.0F, 15, Color{110, 138, 151, 255}, TextAlign::CENTER);
    text(queue, "ENTER / ESC  ·  BACK", 640.0F, 630.0F, 17,
        Color{190, 215, 217, 255}, TextAlign::CENTER);
}

void drawPauseScreen(RenderQueue &queue, const unsigned selection)
{
    rect(queue, 0.0F, 0.0F, SCREEN_WIDTH, SCREEN_HEIGHT, Color{4, 8, 18, 190},
        HUD_LAYER - 2);
    text(queue, "PAUSED", 640.0F, 224.0F, 48, Color{221, 232, 234, 255},
        TextAlign::CENTER);
    static constexpr std::array<std::string_view, 3> OPTIONS{
        "RESUME", "RETURN TO TITLE", "QUIT"};
    for (unsigned index = 0; index < OPTIONS.size(); ++index) {
        const bool selected = index == selection;
        const float y       = 333.0F + static_cast<float>(index) * 49.0F;
        if (selected)
            rect(queue, 527.0F, y + 13.0F, 13.0F, 3.0F,
                Color{120, 198, 210, 255}, HUD_LAYER - 1);
        text(queue, std::string{OPTIONS[index]}, 640.0F, y, 22,
            selected ? Color{231, 240, 240, 255} : Color{125, 147, 158, 255},
            TextAlign::CENTER);
    }
    text(queue, "ENTER SELECT     ESC RESUME", 640.0F, 573.0F, 16,
        Color{126, 155, 167, 255}, TextAlign::CENTER);
}

void drawGameplay(RenderQueue &queue, const float time,
    const SceneAssets &assets, const World &world)
{
    drawRoom(queue, time, assets, world.registry().getResource<RoomGeometry>());
    drawRoomTitle(queue);
    drawActors(queue, assets, world, time);
    drawActorParticles(queue, time, world);
    drawHud(queue, assets, world);
}
} // namespace

void fillScene(RenderQueue &queue, const Screen screen,
    const unsigned selection, const float time, const SceneAssets &assets,
    const World &world)
{
    switch (screen) {
    case Screen::TITLE:
        drawTitleScreen(queue, selection, time, assets);
        return;
    case Screen::OPTIONS:
        drawOptionsScreen(queue, time, assets);
        return;
    case Screen::PLAY:
        drawGameplay(queue, time, assets, world);
        return;
    case Screen::PAUSE:
        drawGameplay(queue, time, assets, world);
        drawPauseScreen(queue, selection);
        return;
    }
}
} // namespace hollow_knight
