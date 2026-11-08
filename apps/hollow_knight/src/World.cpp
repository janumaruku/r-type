/*
** EPITECH PROJECT, 2026
** r-type
** File description:
** Hollow Knight demo world
*/

#include "World.hpp"

#include <algorithm>
#include <cmath>
#include <span>
#include <utility>

#include "engine/core/Errors.hpp"
#include "engine/ecs/Resources.hpp"
#include "network/Packet.hpp"
#include "rtype/protocol/Messages.hpp"
#include "rtype/protocol/Snapshot.hpp"
#include "Systems.hpp"

namespace hollow_knight {
namespace {
using engine::ecs::Registry;
using rtype::protocol::EntityKind;
using rtype::protocol::EntityState;
using rtype::protocol::PlayerInfo;
using rtype::protocol::PlayerState;
using rtype::protocol::Snapshot;

Entity findByNetId(Registry &registry, const std::uint32_t netId)
{
    for (auto &&[entity, identity]: registry.indexedView<NetIdentity>())
        if (identity.value == netId)
            return entity;
    return Entity::NONE;
}

void fillEmptyRoster(Snapshot &snapshot)
{
    snapshot.roster.clear();
    for (std::uint8_t playerId = 0; playerId < rtype::protocol::MAX_PLAYERS;
        ++playerId) {
        snapshot.roster.push_back(PlayerInfo{
            .playerId = playerId, .state = PlayerState::EMPTY});
    }
}
} // namespace

World::World()
{
    restart();
}

void World::restart()
{
    _registry = engine::ecs::Registry{};
    _registry.setResource(engine::ecs::DeltaTime{0.0F});
    _registry.setResource(PlayerInput{});
    _registry.setResource(RoomGeometry{});
    _registry.setResource(RoomState{});
    _registry.registerComponent<NetIdentity>();
    addSystems(_registry);

    spawnPlayer();
    RoomState &room = _registry.getResource<RoomState>();
    for (const EnemyStart &start: room.enemyStarts)
        spawnEnemy(start);
    saveCheckpoint();
}

void World::update(const PlayerInput &input, const float dt)
{
    RoomState &room = _registry.getResource<RoomState>();
    ++room.tick;
    room.messageTimer = std::max(0.0F, room.messageTimer - dt);
    for (HitEffect &effect: room.hitEffects)
        effect.timeLeft = std::max(0.0F, effect.timeLeft - dt);
    std::erase_if(room.hitEffects,
        [](const HitEffect &effect) { return effect.timeLeft <= 0.0F; });

    if (room.deathTimer > 0.0F) {
        room.deathTimer = std::max(0.0F, room.deathTimer - dt);
        if (room.deathTimer == 0.0F)
            restoreCheckpoint();
        return;
    }

    if (room.waveTransitionTimer > 0.0F) {
        room.waveTransitionTimer =
            std::max(0.0F, room.waveTransitionTimer - dt);
        if (room.waveTransitionTimer == 0.0F)
            startNextWave();
    }

    _registry.setResource(engine::ecs::DeltaTime{dt});
    _registry.setResource(input);
    if (input.interactPressed)
        (void)restAtBench();

    _registry.runStage(Stage::UPDATE);
    _registry.applyCommands();

    if (room.deathTimer <= 0.0F && room.waveTransitionTimer <= 0.0F &&
        _registry.getComponents<Enemy>().size() == 0) {
        room.waveTransitionTimer = 2.0F;
        room.message      = "WAVE " + std::to_string(room.wave) + " CLEARED";
        room.messageTimer = room.waveTransitionTimer;
    }
}

bool World::restAtBench()
{
    RoomState &room      = _registry.getResource<RoomState>();
    Player *playerState  = _registry.getComponent<Player>(room.player);
    Transform *transform = _registry.getComponent<Transform>(room.player);
    Body *body           = _registry.getComponent<Body>(room.player);
    if (playerState == nullptr || transform == nullptr || body == nullptr ||
        std::abs(transform->position.x() - BENCH_X) > 86.0F || !body->grounded)
        return false;

    transform->position          = Vec2f{BENCH_X + 40.0F, FLOOR_LEVEL};
    body->velocity               = Vec2f{0.0F, 0.0F};
    playerState->health          = playerState->maxHealth;
    playerState->invulnerableFor = 0.0F;
    room.waveTransitionTimer     = 0.0F;
    for (const EnemyStart &start:
        _registry.getResource<RoomState>().enemyStarts) {
        Entity entity = findByNetId(_registry, start.netId);
        if (entity == Entity::NONE) {
            spawnEnemy(start);
            continue;
        }
        Transform *enemyTransform = _registry.getComponent<Transform>(entity);
        Body *enemyBody           = _registry.getComponent<Body>(entity);
        Enemy *enemy              = _registry.getComponent<Enemy>(entity);
        if (enemyTransform != nullptr && enemyBody != nullptr &&
            enemy != nullptr) {
            enemyTransform->position = start.position;
            enemyBody->velocity      = Vec2f{0.0F, 0.0F};
            enemy->health            = start.health;
            enemy->direction         = start.direction;
            enemy->hitFlash          = 0.0F;
            enemy->sprite            = start.sprite;
            enemy->lastHitSerial     = 0;
        }
    }
    room.message      = "RESTED AT BENCH";
    room.messageTimer = 2.0F;
    saveCheckpoint();
    return true;
}

const engine::ecs::Registry &World::registry() const noexcept
{
    return _registry;
}

const RoomState &World::state() const
{
    return _registry.getResource<RoomState>();
}

const Player &World::player() const
{
    const Player *value = _registry.getComponent<Player>(state().player);
    if (value == nullptr)
        throw engine::core::
            EngineError("Hollow Knight player entity is missing");
    return *value;
}

void World::spawnPlayer()
{
    const Entity entity = _registry.spawnEntity();
    _registry.addComponent(entity, NetIdentity{1});
    _registry.addComponent(entity,
        Transform{Vec2f{BENCH_X + 40.0F, FLOOR_LEVEL}});
    _registry.addComponent(entity,
        Body{.velocity = Vec2f{0.0F, 0.0F},
            .halfWidth = 12.0F,
            .height    = 44.0F,
            .grounded  = true});
    _registry.addComponent(entity, Player{});
    _registry.getResource<RoomState>().player = entity;
}

void World::spawnEnemy(const EnemyStart &start)
{
    const Entity entity      = _registry.spawnEntity();
    const EnemyHitbox hitbox = enemyHitbox(start.sprite, start.flying);
    _registry.addComponent(entity, NetIdentity{start.netId});
    _registry.addComponent(entity, Transform{start.position});
    _registry.addComponent(entity,
        Body{.velocity = Vec2f{0.0F, 0.0F},
            .halfWidth = hitbox.halfWidth,
            .height    = hitbox.height,
            .grounded  = true});
    _registry.addComponent(entity,
        Enemy{.health    = start.health,
            .direction   = start.direction,
            .hoverAnchor = start.position.y(),
            .sprite      = start.sprite,
            .flying      = start.flying});
}

void World::startNextWave()
{
    static const std::array<Vec2f, 8> SPAWN_POSITIONS{{
        {130.0F, 578.0F},
        {285.0F, 578.0F},
        {440.0F, 578.0F},
        {595.0F, 578.0F},
        {750.0F, 578.0F},
        {905.0F, 578.0F},
        {1060.0F, 578.0F},
        {1180.0F, 578.0F},
    }};
    RoomState &room = _registry.getResource<RoomState>();
    ++room.wave;

    const std::size_t extraEnemies =
        static_cast<std::size_t>(std::min<std::uint64_t>((room.wave - 1U) / 2U,
            5U));
    const std::size_t enemyCount = 3U + extraEnemies;
    const std::uint8_t health    = static_cast<std::uint8_t>(2U +
        std::min<std::uint64_t>((room.wave - 1U) / 3U, 4U));
    room.enemyStarts.clear();
    room.enemyStarts.reserve(enemyCount);

    for (std::size_t index = 0; index < enemyCount; ++index) {
        const bool flying = index == 1U;
        const auto spriteIndex =
            static_cast<std::uint8_t>((room.wave +
                                          static_cast<std::uint64_t>(index) -
                                          1U) %
                3U);
        const EnemySprite spriteType = flying
            ? EnemySprite::CROSSROADS_GUARD
            : static_cast<EnemySprite>(spriteIndex);
        const Vec2f position         = flying
            ? Vec2f{SPAWN_POSITIONS[index].x(), 430.0F}
            : SPAWN_POSITIONS[index];
        const EnemyStart start{.netId = room.nextEnemyNetId++,
            .position                 = position,
            .direction                = index % 2U == 0U ? 1.0F : -1.0F,
            .sprite                   = spriteType,
            .health                   = health,
            .flying                   = flying};
        room.enemyStarts.push_back(start);
        spawnEnemy(start);
    }

    room.message             = "WAVE " + std::to_string(room.wave);
    room.messageTimer        = 1.8F;
    room.waveTransitionTimer = 0.0F;
}

void World::saveCheckpoint()
{
    RoomState &room               = _registry.getResource<RoomState>();
    room.checkpointEnemyStarts    = room.enemyStarts;
    room.checkpointWave           = room.wave;
    room.checkpointNextEnemyNetId = room.nextEnemyNetId;
    const Player *playerState     = _registry.getComponent<Player>(room.player);
    if (playerState == nullptr)
        return;

    Snapshot snapshot{.serverTick = room.tick};
    fillEmptyRoster(snapshot);
    for (auto &&[identity, transform, body]:
        _registry.view<NetIdentity, Transform, Body>()) {
        if (const Player *state =
                _registry.getComponent<Player>(findByNetId(_registry,
                    identity.value))) {
            snapshot.entities.push_back(EntityState{.netId = identity.value,
                .kind    = EntityKind::GENERIC_PLAYER,
                .variant = static_cast<std::uint8_t>(state->health |
                    (state->facingRight ? 0x80U : 0U)),
                .x       = transform.position.x(),
                .y       = transform.position.y(),
                .vx      = body.velocity.x(),
                .vy      = body.velocity.y()});
        } else if (const Enemy *state =
                       _registry.getComponent<Enemy>(findByNetId(_registry,
                           identity.value))) {
            snapshot.entities.push_back(EntityState{.netId = identity.value,
                .kind    = EntityKind::GENERIC_ENEMY,
                .variant = state->health,
                .x       = transform.position.x(),
                .y       = transform.position.y(),
                .vx      = body.velocity.x(),
                .vy      = body.velocity.y()});
        }
    }

    net::Packet packet;
    packet << snapshot;
    room.checkpointBytes.assign(packet.bytes().begin(), packet.bytes().end());
}

void World::restoreCheckpoint()
{
    RoomState &room = _registry.getResource<RoomState>();
    if (room.checkpointBytes.empty()) {
        restart();
        return;
    }

    net::Packet packet{std::span<const std::uint8_t>{room.checkpointBytes}};
    const Snapshot snapshot = packet.read<Snapshot>();
    if (!packet.fullyRead())
        throw engine::core::EngineError("checkpoint has trailing bytes");

    std::vector<Entity> enemies;
    for (auto &&[entity, enemy]: _registry.indexedView<Enemy>()) {
        (void)enemy;
        enemies.push_back(entity);
    }
    for (const Entity entity: enemies)
        _registry.killEntity(entity);

    room.wave                = room.checkpointWave;
    room.nextEnemyNetId      = room.checkpointNextEnemyNetId;
    room.enemyStarts         = room.checkpointEnemyStarts;
    room.waveTransitionTimer = 0.0F;
    for (const EnemyStart &start: room.enemyStarts)
        spawnEnemy(start);

    for (const EntityState &entityState: snapshot.entities) {
        Entity entity = entityState.kind == EntityKind::GENERIC_PLAYER
            ? room.player
            : findByNetId(_registry, entityState.netId);
        if (entity == Entity::NONE)
            continue;

        Transform *transform = _registry.getComponent<Transform>(entity);
        Body *body           = _registry.getComponent<Body>(entity);
        if (transform == nullptr || body == nullptr)
            continue;
        transform->position = Vec2f{entityState.x, entityState.y};
        body->velocity      = Vec2f{entityState.vx, entityState.vy};
        body->grounded      = entityState.y >= FLOOR_LEVEL;

        if (entityState.kind == EntityKind::GENERIC_PLAYER) {
            if (Player *playerState = _registry.getComponent<Player>(entity)) {
                playerState->health      = 1;
                playerState->soul        = 0;
                playerState->facingRight = (entityState.variant & 0x80U) != 0;
                playerState->invulnerableFor = 1.1F;
                playerState->attackFor       = 0.0F;
                playerState->focusFor        = 0.0F;
            }
        } else if (Enemy *enemy = _registry.getComponent<Enemy>(entity)) {
            enemy->health    = entityState.variant;
            enemy->hitFlash  = 0.0F;
            enemy->direction = entityState.vx < 0.0F ? -1.0F : 1.0F;
            const auto start = std::ranges::find(room.enemyStarts,
                entityState.netId, &EnemyStart::netId);
            if (start != room.enemyStarts.end()) {
                enemy->sprite      = start->sprite;
                enemy->flying      = start->flying;
                enemy->hoverAnchor = start->position.y();
            }
        }
    }

    Player *playerState = _registry.getComponent<Player>(room.player);
    if (playerState != nullptr)
        playerState->soul = 0;
    room.deathTimer   = 0.0F;
    room.message      = "RETURNED TO THE BENCH";
    room.messageTimer = 1.7F;
}
} // namespace hollow_knight
