/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight ECS systems
*/

#include "Systems.hpp"

#include <algorithm>
#include <cmath>

#include "engine/ecs/Resources.hpp"

namespace hollow_knight {
namespace {
constexpr float PLAYER_SPEED         = 235.0F;
constexpr float JUMP_SPEED           = 580.0F;
constexpr float DASH_SPEED           = 610.0F;
constexpr float DASH_DURATION        = 0.15F;
constexpr float POGO_BOUNCE_SPEED    = 520.0F;
constexpr float GRAVITY              = 1480.0F;
constexpr float ENEMY_SPEED          = 78.0F;
constexpr float ENEMY_NOTICE_RANGE   = 295.0F;
constexpr float FOCUS_DURATION       = 0.68F;
constexpr std::uint8_t SOUL_PER_HIT  = 11;
constexpr std::uint8_t SOUL_PER_HEAL = 33;

float countDown(const float timer, const float dt)
{
    return std::max(0.0F, timer - dt);
}

float directionTo(const float from, const float to)
{
    if (to > from)
        return 1.0F;
    if (to < from)
        return -1.0F;
    return 0.0F;
}

void setX(Vec2f &value, const float x)
{
    value = Vec2f{x, value.y()};
}

void setY(Vec2f &value, const float y)
{
    value = Vec2f{value.x(), y};
}
} // namespace

void addSystems(engine::ecs::Registry &registry)
{
    registry.addSystem<Player, Transform, Body>(Stage::UPDATE,
        "playerControllerSystem", playerControllerSystem);
    registry.addSystem<Enemy, Transform, Body>(Stage::UPDATE, "enemySystem",
        enemySystem);
    registry.addSystem<Transform, Body>(Stage::UPDATE, "physicsSystem",
        physicsSystem);
    registry.addSystem<Player, Enemy>(Stage::UPDATE, "combatSystem",
        combatSystem);
}

void playerControllerSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Player> &, engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Body> &)
{
    const float dt = registry.getResource<engine::ecs::DeltaTime>().seconds;
    const PlayerInput &input = registry.getResource<PlayerInput>();
    RoomState &room          = registry.getResource<RoomState>();

    if (room.deathTimer > 0.0F)
        return;

    for (auto &&[player, transform, body]:
        registry.view<Player, Transform, Body>()) {
        player.invulnerableFor = countDown(player.invulnerableFor, dt);
        player.attackCooldown  = countDown(player.attackCooldown, dt);
        player.pogoFor         = countDown(player.pogoFor, dt);
        player.attackFor       = countDown(player.attackFor, dt);
        player.dashCooldown    = countDown(player.dashCooldown, dt);
        player.dashFor         = countDown(player.dashFor, dt);

        if (input.horizontal != 0.0F)
            player.facingRight = input.horizontal > 0.0F;

        if (input.dashPressed && player.dashCooldown <= 0.0F) {
            player.dashFor      = DASH_DURATION;
            player.dashCooldown = 0.64F;
            setY(body.velocity, 0.0F);
        }
        if (input.attackPressed && player.attackCooldown <= 0.0F) {
            player.attackFor      = NAIL_ATTACK_DURATION;
            player.attackCooldown = 0.31F;
            player.nailDirection  = input.nailDirection;
            ++player.attackSerial;
        }

        if (player.dashFor > 0.0F) {
            setX(body.velocity, player.facingRight ? DASH_SPEED : -DASH_SPEED);
        } else if (input.focusHeld && body.grounded &&
            input.horizontal == 0.0F && player.health < player.maxHealth &&
            player.soul >= SOUL_PER_HEAL) {
            setX(body.velocity, 0.0F);
            player.focusFor += dt;
            if (player.focusFor >= FOCUS_DURATION) {
                player.focusFor = 0.0F;
                ++player.health;
                player.health = std::min(player.health, player.maxHealth);
                player.soul =
                    static_cast<std::uint8_t>(player.soul - SOUL_PER_HEAL);
                player.invulnerableFor =
                    std::max(player.invulnerableFor, 0.35F);
                room.message      = "SOUL RESTORED";
                room.messageTimer = 1.25F;
            }
        } else {
            player.focusFor = 0.0F;
            setX(body.velocity, input.horizontal * PLAYER_SPEED);
        }

        if (input.jumpPressed && body.grounded && player.dashFor <= 0.0F) {
            setY(body.velocity, -JUMP_SPEED);
            body.grounded = false;
        }

        if (!input.focusHeld)
            player.focusFor = 0.0F;
        (void)transform;
    }
}

void enemySystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Enemy> &, engine::ecs::SparseSet<Transform> &,
    engine::ecs::SparseSet<Body> &)
{
    const float dt = registry.getResource<engine::ecs::DeltaTime>().seconds;
    const RoomState &room = registry.getResource<RoomState>();
    const std::uint64_t speedIncrease =
        std::min<std::uint64_t>(room.wave - 1U, 14U);
    const float enemySpeed =
        std::min(ENEMY_SPEED + static_cast<float>(speedIncrease) * 4.0F,
            134.0F);
    if (room.deathTimer > 0.0F)
        return;

    const Transform *playerTransform =
        registry.getComponent<Transform>(room.player);
    if (playerTransform == nullptr)
        return;

    for (auto &&[enemy, transform, body]:
        registry.view<Enemy, Transform, Body>()) {
        enemy.hitFlash = countDown(enemy.hitFlash, dt);
        if (enemy.hitFlash > 0.0F)
            continue;
        if (enemy.flying) {
            enemy.hoverTime += dt;
            setY(transform.position,
                enemy.hoverAnchor + std::sin(enemy.hoverTime * 2.2F) * 13.0F);
            setY(body.velocity, 0.0F);
        }
        const float distance =
            playerTransform->position.x() - transform.position.x();

        if (std::abs(distance) <= ENEMY_NOTICE_RANGE &&
            std::abs(playerTransform->position.y() - transform.position.y()) <
                95.0F) {
            enemy.direction = directionTo(transform.position.x(),
                playerTransform->position.x());
        } else if (transform.position.x() < 125.0F ||
            transform.position.x() > 1155.0F) {
            enemy.direction *= -1.0F;
        }
        setX(body.velocity, enemy.direction * enemySpeed);
    }
}

void physicsSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Transform> &, engine::ecs::SparseSet<Body> &)
{
    constexpr float PLATFORM_PADDING = 14.0F;
    const float dt = registry.getResource<engine::ecs::DeltaTime>().seconds;
    const RoomGeometry &geometry = registry.getResource<RoomGeometry>();

    for (auto &&[entity, transform, body]:
        registry.indexedView<Transform, Body>()) {
        const float previousY = transform.position.y();
        Player *player        = registry.getComponent<Player>(entity);
        Enemy *enemy          = registry.getComponent<Enemy>(entity);
        if (enemy != nullptr && enemy->flying) {
            setX(transform.position,
                transform.position.x() + body.velocity.x() * dt);
            setX(transform.position,
                std::clamp(transform.position.x(), ROOM_LEFT + body.halfWidth,
                    ROOM_RIGHT - body.halfWidth));
            if (enemy->hitFlash > 0.0F) {
                setY(body.velocity, body.velocity.y() + GRAVITY * dt);
                setY(transform.position,
                    transform.position.y() + body.velocity.y() * dt);
            } else {
                setY(body.velocity, 0.0F);
            }
            body.grounded = false;
            continue;
        }
        if (player == nullptr || player->dashFor <= 0.0F)
            setY(body.velocity, body.velocity.y() + GRAVITY * dt);
        else
            setY(body.velocity, 0.0F);

        setX(transform.position,
            transform.position.x() + body.velocity.x() * dt);
        setY(transform.position,
            transform.position.y() + body.velocity.y() * dt);
        setX(transform.position,
            std::clamp(transform.position.x(), ROOM_LEFT + body.halfWidth,
                ROOM_RIGHT - body.halfWidth));
        body.grounded = false;

        float landingY    = FLOOR_LEVEL;
        const float left  = transform.position.x() - body.halfWidth;
        const float right = transform.position.x() + body.halfWidth;
        for (const Platform &platform: geometry.platforms) {
            const bool overlaps = right > platform.x - PLATFORM_PADDING &&
                left < platform.x + platform.width + PLATFORM_PADDING;
            if (overlaps && previousY <= platform.top &&
                transform.position.y() >= platform.top)
                landingY = std::min(landingY, platform.top);
        }

        if (transform.position.y() >= landingY) {
            setY(transform.position, landingY);
            setY(body.velocity, 0.0F);
            body.grounded = true;
        }
        if (transform.position.y() < body.height)
            setY(transform.position, body.height);
    }
}

void combatSystem(engine::ecs::Registry &registry,
    engine::ecs::SparseSet<Player> &, engine::ecs::SparseSet<Enemy> &)
{
    RoomState &room = registry.getResource<RoomState>();
    if (room.deathTimer > 0.0F)
        return;

    for (auto &&[player, playerTransform, playerBody]:
        registry.view<Player, Transform, Body>()) {
        const NailAttackPose attackPose =
            nailAttackPose(playerTransform.position.x(),
                playerTransform.position.y(), player.facingRight,
                player.nailDirection, player.attackFor);
        const float attackLeft  = attackPose.centerX - attackPose.width / 2.0F;
        const float attackRight = attackPose.centerX + attackPose.width / 2.0F;
        const float attackTop   = attackPose.centerY - attackPose.height / 2.0F;
        const float attackBottom =
            attackPose.centerY + attackPose.height / 2.0F;
        for (auto &&[entity, identity, enemy, enemyTransform, enemyBody]:
            registry.indexedView<NetIdentity, Enemy, Transform, Body>()) {
            const float enemyLeft =
                enemyTransform.position.x() - enemyBody.halfWidth;
            const float enemyRight =
                enemyTransform.position.x() + enemyBody.halfWidth;
            const float enemyTop =
                enemyTransform.position.y() - enemyBody.height;
            const float enemyBottom = enemyTransform.position.y();
            const float dx =
                enemyTransform.position.x() - playerTransform.position.x();
            const float facing   = player.facingRight ? 1.0F : -1.0F;
            const bool inNailArc = attackRight > enemyLeft &&
                attackLeft < enemyRight && attackBottom > enemyTop &&
                attackTop < enemyBottom;
            bool hitByNail = false;

            if (player.attackFor > 0.0F && inNailArc &&
                enemy.lastHitSerial != player.attackSerial) {
                hitByNail           = true;
                enemy.lastHitSerial = player.attackSerial;
                enemy.hitFlash      = 0.17F;
                ++player.hitSerial;
                room.hitEffects.push_back(HitEffect{
                    .position = Vec2f{enemyTransform.position.x(),
                        enemyTransform.position.y() -
                            enemyBody.height * 0.5F}});
                if (player.nailDirection == NailDirection::UP) {
                    setX(enemyBody.velocity, facing * 75.0F);
                    setY(enemyBody.velocity, -190.0F);
                    enemyBody.grounded = false;
                } else if (player.nailDirection == NailDirection::DOWN) {
                    setX(enemyBody.velocity, 0.0F);
                    setY(enemyBody.velocity, 250.0F);
                    enemyBody.grounded = false;
                    setY(playerBody.velocity, -POGO_BOUNCE_SPEED);
                    playerBody.grounded = false;
                    player.pogoFor      = 0.18F;
                } else {
                    setX(enemyBody.velocity, facing * 275.0F);
                    setY(enemyBody.velocity, -90.0F);
                    enemyBody.grounded = false;
                }
                if (enemy.health > 0)
                    --enemy.health;
                player.soul       = static_cast<std::uint8_t>(std::min<int>(99,
                    static_cast<int>(player.soul) + SOUL_PER_HIT));
                room.message      = player.nailDirection == NailDirection::DOWN
                    ? "POGO!"
                    : "NAIL STRIKE";
                room.messageTimer = 0.48F;
                if (enemy.health == 0) {
                    ++room.kills;
                    registry.commands().kill(entity);
                    continue;
                }
            }

            const float playerLeft =
                playerTransform.position.x() - playerBody.halfWidth;
            const float playerRight =
                playerTransform.position.x() + playerBody.halfWidth;
            const float playerTop =
                playerTransform.position.y() - playerBody.height;
            const float playerBottom = playerTransform.position.y();
            const bool touching      = playerRight > enemyLeft &&
                playerLeft < enemyRight && playerBottom > enemyTop &&
                playerTop < enemyBottom;
            if (!hitByNail && touching && player.invulnerableFor <= 0.0F) {
                if (player.health > 0)
                    --player.health;
                player.invulnerableFor = 1.0F;
                setX(playerBody.velocity, dx < 0.0F ? 215.0F : -215.0F);
                setY(playerBody.velocity, -290.0F);
                playerBody.grounded = false;
                if (player.health == 0) {
                    room.deathTimer = 1.12F;
                    ++room.deaths;
                    room.message      = "SHADE LOST";
                    room.messageTimer = 1.12F;
                }
            }
        }
    }
}
} // namespace hollow_knight
